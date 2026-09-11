// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Item/Interaction/AFFInteractableActor.h"
#include "GameplayTagContainer.h"
#include "AFFLightSwitcher.generated.h"

class UPointLightComponent;

// 灯泡：状态在 On/Off 之间切换，切换时控制 PointLight 亮灭
UCLASS()
class LREDFFIREBRIGADE_API AFFLightSwitcher : public AFFInteractableActor
{
	GENERATED_BODY()

public:
	AFFLightSwitcher();

	// 交互：翻转 On/Off 状态
	virtual void Interact(AActor* Interactor) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light")
	UPointLightComponent* PointLight;

	// 亮/灭对应的状态 Tag（构造函数给默认值，编辑器可改）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (Categories = "Interaction"))
	FGameplayTag OnStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (Categories = "Interaction"))
	FGameplayTag OffStateTag;

	UFUNCTION()
	void HandleStateChanged(FGameplayTag NewState);
};
