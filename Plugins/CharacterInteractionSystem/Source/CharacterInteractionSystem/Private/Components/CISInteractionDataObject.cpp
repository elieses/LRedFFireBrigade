// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#include "Components/CISInteractionDataObject.h"

#include "Common/CISFunctionLibrary.h"
#include "Components/CISCharacterInteractionComponent.h"
#include "Components/CISInteractionObjectComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "InteractionObjectActions/CISInteractionObjectAction.h"
#include "InteractionObjectConditions/CISInteractionObjectCondition.h"
#include "InteractionObjectMoveHandlers/CISInteractionObjectMoveHandler.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

void UCISInteractionDataObject::Init(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent)
{
	if (InteractionCharacter == nullptr || CharacterInteractionComponent == nullptr || InteractionObjectComponent == nullptr)
	{
		return;
	}

	bInitialized = true;

	CachedInteractionObjectComponent = InteractionObjectComponent;
	CachedInteractionCharacter = InteractionCharacter;
	CachedCharacterInteractionComponent = CharacterInteractionComponent;

	OnInitialized(InteractionCharacter, CharacterInteractionComponent, InteractionObjectComponent);
}

void UCISInteractionDataObject::Overlapped(bool bOverlapped)
{
	bWasOverlapped = bOverlapped;

	OnOverlapped(GetInteractionCharacter(), GetCharacterInteractionComponent(), GetInteractionObjectComponent(), bOverlapped);
}

bool UCISInteractionDataObject::IsOverlapped() const
{
	return bWasOverlapped;
}

bool UCISInteractionDataObject::IsCurrentUseSideValid() const
{
	return CurrentSide.IsSet();
}

void UCISInteractionDataObject::Tick(float DeltaSeconds)
{
	if (!bInitialized)
	{
		return;
	}

	DetermineUseSide();

	SetVisible(CheckVisibleConditions());

	SetUsable(CheckUsableConditions());
}

bool UCISInteractionDataObject::CheckVisibleConditions() const
{
	if (!IsInitialized() || !IsOverlapped() || !IsCurrentUseSideValid() || GetInteractionObjectComponent()->IsObjectUsing() || !CheckAngleAndDistance())
	{
		return false;
	}

	if (!GetInteractionObjectComponent()->IsObjectUsable())
	{
		return false;
	}

	auto returnValue = true;

	for (const auto condition : GetInteractionObjectComponent()->GetSettings().VisibleConditions)
	{
		if (condition == nullptr)
		{
			continue;
		}

		if (!condition->CheckCondition(GetInteractionCharacter(), GetCharacterInteractionComponent(), GetInteractionObjectComponent(), GetCurrentSide()))
		{
			returnValue = false;
			break;
		}
	}

	return returnValue;
}

bool UCISInteractionDataObject::CheckUsableConditions() const
{
	if (!IsObjectVisible())
	{
		return false;
	}

	bool bReturnValue = true;

	for (const auto condition : GetInteractionObjectComponent()->GetSettings().UsableConditions)
	{
		if (condition == nullptr)
		{
			continue;
		}

		if (!condition->CheckCondition(GetInteractionCharacter(), GetCharacterInteractionComponent(), GetInteractionObjectComponent(), GetCurrentSide()))
		{
			bReturnValue = false;
			break;
		}
	}

	return bReturnValue;
}

bool UCISInteractionDataObject::CheckAngleAndDistance() const
{
	if (!IsInitialized() || !IsOverlapped() || !IsCurrentUseSideValid() || GetInteractionObjectComponent()->IsObjectUsing() || GetCharacterInteractionComponent()->IsCharacterUseAnyObject())
	{
		return false;
	}

	bool bReturnValue = false;

	const auto distanceBetweenCharacterAndObject = FVector(GetInteractionObjectComponent()->GetOwner()->GetActorLocation() - (GetInteractionCharacter()->GetActorLocation() - FVector(0.f, 0.f, GetInteractionCharacter()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()))).Size2D();
	if (distanceBetweenCharacterAndObject <= GetInteractionObjectComponent()->GetSettings().InteractionDistance)
	{
		FVector socketLocation = FVector::ZeroVector;

		const USceneComponent* angleDetectionComponent = UCISFunctionLibrary::GetSceneComponentWithName(GetInteractionObjectComponent()->GetOwner(), GetInteractionObjectComponent()->GetSettings().AngleDetectionComponent, socketLocation);

		if (angleDetectionComponent == nullptr)
		{
			angleDetectionComponent = GetInteractionObjectComponent()->GetOwner()->GetRootComponent();
		}

		if (angleDetectionComponent != nullptr)
		{
			const auto angle = UKismetMathLibrary::DegAcos(UKismetMathLibrary::Dot_VectorVector(GetInteractionCharacter()->GetActorForwardVector().GetUnsafeNormal2D(), (socketLocation - GetInteractionCharacter()->GetActorLocation()).GetUnsafeNormal2D()));

			const auto interactionAngle = GetInteractionObjectComponent()->GetSettings().bOverrideCharacterInteractionAngle ? GetInteractionObjectComponent()->GetSettings().OverrideCharacterInteractionAngle : GetCharacterInteractionComponent()->GetCharacterInteractionSettings().InteractionAngle;

			bReturnValue = angle <= interactionAngle * 0.5f;
		}
		else
		{
			bReturnValue = true;
		}
	}

	return bReturnValue;
}

void UCISInteractionDataObject::DetermineUseSide()
{
	if (!bWasOverlapped)
	{
		return;
	}

	const auto objectComponentOwner = GetInteractionObjectComponent()->GetOwner();

	const FCISInteractionObjectSide* newSide = nullptr;

	for (auto& side : GetInteractionObjectComponent()->GetSettings().InteractionSides)
	{
		const auto ownerRotation = objectComponentOwner->GetActorRotation();
		auto directionFromMin = UKismetMathLibrary::ComposeRotators(ownerRotation, FRotator(0.f, side.InteractionAngleMin, 0.f)).Vector();
		auto directionFromMax = UKismetMathLibrary::ComposeRotators(ownerRotation, FRotator(0.f, side.InteractionAngleMax, 0.f)).Vector();

		const FVector direction = FVector(directionFromMin + directionFromMax).GetSafeNormal2D() * (side.bAngleSide ? -1.f : 1.f);

		auto angleOfSegment = UKismetMathLibrary::DegAcos(UKismetMathLibrary::Dot_VectorVector(directionFromMin, directionFromMax));

		angleOfSegment = (side.bAngleSide ? (360.f - angleOfSegment) : angleOfSegment);

		const auto directionToCharacter = FVector(GetInteractionCharacter()->GetActorLocation() - objectComponentOwner->GetActorLocation()).GetSafeNormal2D();

		const auto angleToCharacter = UKismetMathLibrary::DegAcos(UKismetMathLibrary::Dot_VectorVector(direction, directionToCharacter));

		if (angleToCharacter <= angleOfSegment * 0.5f && GetInteractionObjectComponent()->GetObjectActionForSide(side) != nullptr)
		{
			newSide = &side;
			break;
		}
	}

	SetCurrentUseSide(newSide);
}

bool UCISInteractionDataObject::IsCharacterCanUseObject() const
{
	return ObjectUsable.Get(false);
}

bool UCISInteractionDataObject::IsObjectVisible() const
{
	return ObjectVisible.Get(false);
}

void UCISInteractionDataObject::UseObject()
{
	if (IsCharacterCanUseObject())
	{
		bUsingObject = true;

		GetCharacterInteractionComponent()->SetOwnerOfActor(CastChecked<AActor>(GetInteractionObjectComponent()->GetOwner()));

		GetInteractionObjectComponent()->SetObjectIsUsing(true);

		OnObjectUseStarted(GetInteractionCharacter(), GetCharacterInteractionComponent(), GetInteractionObjectComponent());

		if (const auto action = GetInteractionObjectComponent()->GetObjectActionForSide(GetCurrentSide()))
		{
			CurrentAction = action;
			CurrentAction->OnInteractionObjectActionFinishedDelegate.AddUObject(this, &UCISInteractionDataObject::OnObjectActionUseFinished);
		}

		CurrentMoveHandler = GetInteractionObjectComponent()->GetSettings().InteractionSides.FindByPredicate([&](const FCISInteractionObjectSide& OtherSide) { return OtherSide == CurrentSide.GetValue(); })->InteractionObjectSideMoveHandler;

		if (CurrentMoveHandler.IsValid())
		{
			CurrentMoveHandler.Get()->OnMovementFinishedDelegate.AddUObject(this, &UCISInteractionDataObject::OnMovementToObjectFinished);
			OnMovementStarted(GetInteractionCharacter(), GetCharacterInteractionComponent(), GetInteractionObjectComponent());
			CurrentMoveHandler.Get()->StartMovementInternal(GetInteractionCharacter(), GetCharacterInteractionComponent(), GetInteractionObjectComponent(), GetCurrentSide());
		}
		else
		{
			OnMovementToObjectFinished();
		}

		if (GetCurrentSide().ActionExecutionType == ECISActionExecutionType::StartParallelWithMoveHandler)
		{
			StartObjectAction();
		}
	}
}

void UCISInteractionDataObject::InterruptUsing()
{
	if (CurrentMoveHandler.IsValid())
	{
		CurrentMoveHandler.Get()->OnMovementFinishedDelegate.RemoveAll(this);
		CurrentMoveHandler.Get()->MovementAbortedInternal(GetInteractionCharacter(), GetCharacterInteractionComponent(), GetInteractionObjectComponent(), GetCurrentSide());
		ResetMovementHandler();
	}

	if (IsUsingObject())
	{
		if (CurrentAction.IsValid())
		{
			CurrentAction->FinishActionInternal(CachedInteractionCharacter, CachedCharacterInteractionComponent, CachedInteractionObjectComponent, true);
		}
		else
		{
			FinishUseObject(true);
		}
	}
}

void UCISInteractionDataObject::ResetMovementHandler()
{
	CurrentMoveHandler->InvalidateHandler();
	CurrentMoveHandler.Reset();
}

void UCISInteractionDataObject::SetCurrentUseSide(const FCISInteractionObjectSide* NewSide)
{
	if (NewSide == nullptr)
	{
		if (IsCurrentUseSideValid())
		{
			SetVisible(false);
			SetUsable(false);
			CurrentSide.Reset();
		}
	}
	else
	{
		if (!IsCurrentUseSideValid() || GetCurrentSide() != *NewSide)
		{
			SetVisible(false);
			SetUsable(false);
			CurrentSide = *NewSide;
		}
	}
}

void UCISInteractionDataObject::StartObjectAction()
{
	if (CurrentAction.IsValid())
	{
		OnObjectActionStarted(CurrentAction.Get(), GetInteractionCharacter(), GetCharacterInteractionComponent(), GetInteractionObjectComponent());

		if (CurrentAction != nullptr)
		{
			CurrentAction->StartInteractionObjectActionInternal(GetInteractionCharacter(), GetCharacterInteractionComponent(), GetInteractionObjectComponent(), GetCurrentSide());
		}
	}
	else
	{
		FinishUseObject(false);
	}
}

void UCISInteractionDataObject::FinishUseObject(bool bInterrupted)
{
	check(IsUsingObject());

	OnObjectUseFinished(GetInteractionCharacter(), GetCharacterInteractionComponent(), GetInteractionObjectComponent(), bInterrupted);

	if (bUsingObject)
	{
		bUsingObject = false;

		if (CurrentMoveHandler.IsValid())
		{
			CurrentMoveHandler->FinishMovement();
			ResetMovementHandler();
		}

		const auto cachedObjectComponent = GetInteractionObjectComponent();

		if (CurrentAction.IsValid())
		{
			CurrentAction->OnInteractionObjectActionFinishedDelegate.RemoveAll(this);
		}

		CurrentAction.Reset();

		cachedObjectComponent->SetObjectIsUsing(false);

		GetCharacterInteractionComponent()->SetOwnerOfActor(cachedObjectComponent->GetOwner(), true);

		OnEndUseObjectDelegate.Broadcast(cachedObjectComponent, this);
	}
}

void UCISInteractionDataObject::OnMovementToObjectFinished()
{
	if (CurrentMoveHandler.IsValid())
	{
		CurrentMoveHandler.Get()->OnMovementFinishedDelegate.RemoveAll(this);
	}

	OnMovementEnded(GetInteractionCharacter(), GetCharacterInteractionComponent(), GetInteractionObjectComponent());

	if (IsUsingObject() && GetCurrentSide().ActionExecutionType == ECISActionExecutionType::StartAfterFinishMoveHandler)
	{
		StartObjectAction();
	}
}

void UCISInteractionDataObject::OnObjectActionUseFinished(UCISInteractionObjectAction* Action, bool bInterrupted)
{
	FinishUseObject(bInterrupted);
}

void UCISInteractionDataObject::SetUsable(bool bValue)
{
	if (!ObjectUsable.IsSet() || ObjectUsable.GetValue() != bValue)
	{
		ObjectUsable = bValue;

		OnUsableChanged(bValue);
	}
}

void UCISInteractionDataObject::SetVisible(bool bEnter)
{
	if (!ObjectVisible.IsSet() || ObjectVisible.GetValue() != bEnter)
	{
		ObjectVisible = bEnter;

		OnVisibleChanged(bEnter);
	}
}

bool UCISInteractionDataObject::IsInitialized() const
{
	return bInitialized;
}

bool UCISInteractionDataObject::IsUsingObject() const
{
	return bUsingObject;
}
void UCISInteractionDataObject::OnOverlapped_Implementation(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, bool bOverlap)
{
}

void UCISInteractionDataObject::OnInitialized_Implementation(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent)
{
}

void UCISInteractionDataObject::OnMovementEnded_Implementation(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent)
{
}
void UCISInteractionDataObject::OnMovementStarted_Implementation(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent)
{
}
void UCISInteractionDataObject::OnUsableChanged_Implementation(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, bool bUsable)
{
}
void UCISInteractionDataObject::OnVisibleChanged_Implementation(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, bool bEnter)
{
}
void UCISInteractionDataObject::OnObjectUseStarted_Implementation(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent)
{
}
void UCISInteractionDataObject::OnObjectActionStarted_Implementation(UCISInteractionObjectAction* Action, ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent)
{
}

void UCISInteractionDataObject::OnObjectUseFinished_Implementation(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, bool bInterrupted)
{
}

UCISInteractionObjectComponent* UCISInteractionDataObject::GetInteractionObjectComponent() const
{
	if (CachedInteractionObjectComponent.IsValid())
	{
		return CachedInteractionObjectComponent.Get();
	}

	return nullptr;
}

UCISCharacterInteractionComponent* UCISInteractionDataObject::GetCharacterInteractionComponent() const
{
	if (CachedCharacterInteractionComponent.IsValid())
	{
		return CachedCharacterInteractionComponent.Get();
	}

	return nullptr;
}

ACharacter* UCISInteractionDataObject::GetInteractionCharacter() const
{
	if (CachedInteractionCharacter.IsValid())
	{
		return CachedInteractionCharacter.Get();
	}

	return nullptr;
}

void UCISInteractionDataObject::OnDataRemovedFromCharacterComponent()
{
	InterruptUsing();
	SetVisible(false);
	SetUsable(false);
}

const FCISInteractionObjectSide& UCISInteractionDataObject::GetCurrentSide() const
{
	static FCISInteractionObjectSide DefaultSide;

	return IsCurrentUseSideValid() ? CurrentSide.GetValue() : DefaultSide;
}

void UCISInteractionDataObject::OnUsableChanged(bool bCanUse)
{
	if (bCanUse)
	{
		GetCharacterInteractionComponent()->CountOfUsableObjects++;
	}
	else
	{
		GetCharacterInteractionComponent()->CountOfUsableObjects--;
	}

	OnUsableChanged(GetInteractionCharacter(), GetCharacterInteractionComponent(), GetInteractionObjectComponent(), bCanUse);
}

void UCISInteractionDataObject::OnVisibleChanged(bool bEnter)
{
	OnVisibleChanged(GetInteractionCharacter(), GetCharacterInteractionComponent(), GetInteractionObjectComponent(), bEnter);
}
