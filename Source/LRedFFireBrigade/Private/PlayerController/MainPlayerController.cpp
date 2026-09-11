// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/MainPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/IControllable.h"
#include "Interfaces/ISelectable.h"
#include "Interfaces/IMoveable.h"
#include "Interfaces/IInteractor.h"
#include "Interfaces/IFFItemUser.h"
#include "Characters/Firefighters/FirefighterBase.h"
#include "Characters/Firefighters/UFFCharacterStatsComponent.h"
#include "Characters/Firefighters/UFFCharacterData.h"
#include "UI/Backpack/UFFBackpackWidget.h"
#include "Item/MoveIndicator/MoveIndicator.h"
#include "Item/Interaction/AFFInteractableActor.h"
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

	// 创建背包 UI（MVC 的 View），初始隐藏
	if (BackpackWidgetClass)
	{
		if (UFFBackpackWidget* Widget = CreateWidget<UFFBackpackWidget>(this, BackpackWidgetClass))
		{
			BackpackWidget = Widget;
			Widget->AddToViewport();
			Widget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
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
		EnhancedInputComponent->BindAction(IA_Interact, ETriggerEvent::Started, this, &AMainPlayerController::HandleInteract);
		EnhancedInputComponent->BindAction(IA_Pickup, ETriggerEvent::Started, this, &AMainPlayerController::HandlePickup);
		EnhancedInputComponent->BindAction(IA_SwitchHand1, ETriggerEvent::Started, this, &AMainPlayerController::HandleSwitchHand1);
		EnhancedInputComponent->BindAction(IA_SwitchEmptyHand, ETriggerEvent::Started, this, &AMainPlayerController::HandleSwitchEmptyHand);
		EnhancedInputComponent->BindAction(IA_ToggleWalkRun, ETriggerEvent::Started, this, &AMainPlayerController::HandleToggleWalkRun);
		EnhancedInputComponent->BindAction(IA_ToggleCrouch, ETriggerEvent::Started, this, &AMainPlayerController::HandleToggleCrouch);
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

	// 1) 用 CanInteractor 通道沿光标射线多线查询：命中即交互物（overlap/block 都返回）
	float MouseX = 0.f, MouseY = 0.f;
	FVector WorldOrigin = FVector::ZeroVector, WorldDir = FVector::ZeroVector;
	if (GetMousePosition(MouseX, MouseY) && DeprojectScreenPositionToWorld(MouseX, MouseY, WorldOrigin, WorldDir))
	{
		const FVector TraceEnd = WorldOrigin + WorldDir * 100000.f;
		TArray<FHitResult> Hits;
		GetWorld()->LineTraceMultiByChannel(Hits, WorldOrigin, TraceEnd, ECC_GameTraceChannel5 /*CanInteractor*/,
			FCollisionQueryParams());

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
				FString::Printf(TEXT("CanInteractor命中=%d"), Hits.Num()));
		}

		// 最近的可交互物 → 走位交互（跳过拾取物：物品只能由 F 捡，见 IInteractor::TryPickup）
		for (const FHitResult& H : Hits)
		{
			if (AFFInteractableActor* IA = Cast<AFFInteractableActor>(H.GetActor()))
			{
				if (IA->IsPickup())
				{
					continue;
				}
				if (IInteractor* Interactor = Cast<IInteractor>(SelectedActor.Get()))
				{
					Interactor->CommandInteract(IA);
					return;
				}
			}
		}
	}

	// 2) 没有可交互物 → 原有地面移动 + MoveIndicator
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

void AMainPlayerController::HandleInteract(const FInputActionValue& Value)
{
	if (IInteractor* Interactor = Cast<IInteractor>(SelectedActor.Get()))
	{
		Interactor->TryInteract();
	}
}

void AMainPlayerController::HandlePickup(const FInputActionValue& Value)
{
	if (IInteractor* Interactor = Cast<IInteractor>(SelectedActor.Get()))
	{
		Interactor->TryPickup();
	}
}

void AMainPlayerController::HandleSwitchHand1(const FInputActionValue& Value)
{
	if (AFirefighterBase* FF = Cast<AFirefighterBase>(SelectedActor.Get()))
	{
		FF->SwitchToHandSlot(0);
	}
}

void AMainPlayerController::HandleSwitchEmptyHand(const FInputActionValue& Value)
{
	if (AFirefighterBase* FF = Cast<AFirefighterBase>(SelectedActor.Get()))
	{
		FF->SwitchToEmptyHand();
	}
}

void AMainPlayerController::HandleToggleWalkRun(const FInputActionValue& Value)
{
	if (AFirefighterBase* FF = Cast<AFirefighterBase>(SelectedActor.Get()))
	{
		FF->ToggleWalkRun();
	}
}

void AMainPlayerController::HandleToggleCrouch(const FInputActionValue& Value)
{
	if (AFirefighterBase* FF = Cast<AFirefighterBase>(SelectedActor.Get()))
	{
		FF->ToggleCrouchState();
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
		UpdateBackpackForSelection();   // 取消选中 → 隐藏背包
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

	// MVC 联动：选中消防员 → 背包显示并绑定其装备；否则隐藏
	UpdateBackpackForSelection();
}

void AMainPlayerController::UpdateBackpackForSelection()
{
	if (!BackpackWidget)
	{
		return;
	}

	if (IFFItemUser* ItemUser = Cast<IFFItemUser>(SelectedActor.Get()))
	{
		BackpackWidget->BindEquipment(ItemUser->GetEquipment());

		// 状态条：选中消防员时绑定其 Stats
		if (const AFirefighterBase* FF = Cast<AFirefighterBase>(SelectedActor.Get()))
		{
			BackpackWidget->BindStats(FF->Stats);
			BackpackWidget->BindCharacterData(FF->CharacterData);
		}

		BackpackWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		BackpackWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}
