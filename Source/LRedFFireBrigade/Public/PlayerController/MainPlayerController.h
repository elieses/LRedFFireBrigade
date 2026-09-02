// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class ISelectable;
class IMoveable;
class AMoveIndicator;
class AActor;
/**
 * 
 */
UCLASS()
class LREDFFIREBRIGADE_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	void HandleMove(const FInputActionValue& Value);
	void HandleZoom(const FInputActionValue& Value);
	void HandleRotate(const FInputActionValue& Value);
	void HandleSelect(const FInputActionValue& Value);
	void HandleMoveCommand(const FInputActionValue& Value);
private:
	// IMC对应模式输入方案
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* IMC_Default;
	// Input Actions
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Move;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Zoom;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Rotate;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Click;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_RightClick;

	// 右键移动指示器的蓝图类（在编辑器里指定）
	UPROPERTY(EditAnywhere, Category="Movement")
	TSubclassOf<AMoveIndicator> MoveIndicatorClass;

	// 当前选中的对象（弱引用，避免悬垂）
	TWeakObjectPtr<AActor> SelectedActor;
	void SetSelectedActor(AActor* NewActor);

	// 当前活动的移动指示器（新指令时销毁旧的）
	TWeakObjectPtr<AMoveIndicator> ActiveIndicator;
};
