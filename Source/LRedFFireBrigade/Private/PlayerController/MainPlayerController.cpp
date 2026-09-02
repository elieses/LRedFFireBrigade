// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/MainPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Interfaces/IControllable.h"
#include "Interfaces/ISelectable.h"
#include "Interfaces/IMoveable.h"
#include "Item/MoveIndicator/MoveIndicator.h"
void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();
	//游戏开始绑定 MappingContext：IMC_Default
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (IMC_Default)
			{
				Subsystem->AddMappingContext(IMC_Default, 0);
			}
		}
	}
	// 显示鼠标光标
	bShowMouseCursor = true;
	bEnableClickEvents = true;      // 需要点击事件才开
	bEnableMouseOverEvents = true;  // 需要悬停检测才开
}

void AMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	// 将 InputComponent 转为 EnhancedInputComponent，用于绑定增强输入
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// 绑定 IA_Move 的 Triggered 事件到 HandleMove
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AMainPlayerController::HandleMove);
		EnhancedInputComponent->BindAction(IA_Zoom, ETriggerEvent::Triggered, this, &AMainPlayerController::HandleZoom);
		EnhancedInputComponent->BindAction(IA_Rotate, ETriggerEvent::Started, this, &AMainPlayerController::HandleRotate);
		EnhancedInputComponent->BindAction(IA_Click, ETriggerEvent::Started, this, &AMainPlayerController::HandleSelect);
		EnhancedInputComponent->BindAction(IA_RightClick, ETriggerEvent::Started, this, &AMainPlayerController::HandleMoveCommand);
	}
}

void AMainPlayerController::HandleMove(const FInputActionValue& Value)
{
	if (IControllable* MoveInterface = Cast<IControllable>(GetPawn()))
	{
		MoveInterface->Move(Value);
	}
}

void AMainPlayerController::HandleZoom(const FInputActionValue& Value)
{
	if (IControllable* MoveInterface = Cast<IControllable>(GetPawn()))
	{
		MoveInterface->Zoom(Value);
	}
}

void AMainPlayerController::HandleRotate(const FInputActionValue& Value)
{
	if (IControllable* MoveInterface = Cast<IControllable>(GetPawn()))
	{
		MoveInterface->Rotate(Value);
	}
}

void AMainPlayerController::HandleSelect(const FInputActionValue& Value)
{
	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_GameTraceChannel1, false, Hit))
	{
		SetSelectedActor(Hit.GetActor());
	}
}

void AMainPlayerController::HandleMoveCommand(const FInputActionValue& Value)
{
	if (!SelectedActor.IsValid()) return;

	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		if (IMoveable* Movable = Cast<IMoveable>(SelectedActor.Get()))
		{
			Movable->MoveToLocation(Hit.ImpactPoint);
		}

		// 新指令：销毁旧指示器
		if (ActiveIndicator.IsValid())
		{
			ActiveIndicator->Destroy();
		}

		// 在落点生成移动指示器（蓝图子类，含绿色圆环），等待角色到达后消失
		if (MoveIndicatorClass)
		{
			const FVector IndicatorSpawnLocation = Hit.ImpactPoint + FVector(0, 0, 3.f);
			AMoveIndicator* Indicator = GetWorld()->SpawnActor<AMoveIndicator>(
				MoveIndicatorClass, IndicatorSpawnLocation, FRotator::ZeroRotator);
			if (Indicator)
			{
				Indicator->Initialize(SelectedActor.Get(), Hit.ImpactPoint);
				ActiveIndicator = Indicator;
			}
		}
	}
}

void AMainPlayerController::SetSelectedActor(AActor* NewActor)
{
	// 再次点击同一角色 → 取消选中
	if (SelectedActor.Get() == NewActor)
	{
		if (ISelectable* Sel = Cast<ISelectable>(NewActor))
		{
			Sel->SetSelected(false);
		}
		SelectedActor = nullptr;
		return;
	}

	// 取消旧选中
	if (ISelectable* OldSelected = Cast<ISelectable>(SelectedActor.Get()))
	{
		OldSelected->SetSelected(false);
	}

	SelectedActor = NewActor;

	// 显示新选中（非可选中对象则无选中效果）
	if (ISelectable* NewSelectable = Cast<ISelectable>(NewActor))
	{
		NewSelectable->SetSelected(true);
	}
}
