// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Interaction/Light/AFFLightSwitcher.h"
#include "Components/PointLightComponent.h"
#include "Engine/Engine.h"

AFFLightSwitcher::AFFLightSwitcher()
{
	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	PointLight->SetupAttachment(RootComponent);
	PointLight->SetVisibility(false);

	OnStateTag  = FGameplayTag::RequestGameplayTag(FName(TEXT("Interaction.Switcher.On")));
	OffStateTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Interaction.Switcher.Off")));

	// 灯默认初始为关
	InitialState = OffStateTag;
}


void AFFLightSwitcher::Interact(AActor* Interactor)
{
	// 翻转 On <-> Off
	SetState(GetState() == OnStateTag ? OffStateTag : OnStateTag);
}


void AFFLightSwitcher::BeginPlay()
{
	Super::BeginPlay(); // 基类会把状态置为 InitialState(Off) 并广播一次

	if (PointLight)
	{
		OnStateChanged.AddDynamic(this, &AFFLightSwitcher::HandleStateChanged);
		// 基类 BeginPlay 的广播发生在我们订阅之前，这里手动同步一次初始表现
		HandleStateChanged(GetState());
	}
}


void AFFLightSwitcher::HandleStateChanged(FGameplayTag NewState)
{
	const bool bOn = (NewState == OnStateTag);
	if (PointLight)
	{
		PointLight->SetVisibility(bOn);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, bOn ? FColor::Green : FColor::Red,
			FString::Printf(TEXT("[Light %s] %s"), *GetName(), bOn ? TEXT("开灯 ON") : TEXT("关灯 OFF")));
	}
}
