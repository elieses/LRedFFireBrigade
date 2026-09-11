// Copyright 2023 Dmitriy Vergasov All Rights Reserved
#include "Components/CISInteractionObjectComponent.h"

#include "Components/CISInteractionDataObject.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/ActorChannel.h"
#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "Engine/NetDriver.h"
#include "GameFramework/Actor.h"
#include "InteractionObjectActions/CISInteractionObjectAction.h"
#include "InteractionObjectMoveHandlers/CISInteractionObjectMoveHandler.h"
#include "Kismet/KismetMathLibrary.h"

#if WITH_EDITOR
#include "Editor.h"
#include "LevelEditorViewport.h"
#endif
#include "Net/UnrealNetwork.h"

UCISInteractionObjectComponent::UCISInteractionObjectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	SetIsReplicatedByDefault(true);
}

#if WITH_EDITOR
void UCISInteractionObjectComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UCISInteractionObjectComponent, ObjectState))
	{
		OnObjectStateChangedDelegate.Broadcast(ObjectState);
	}
}
#endif

void UCISInteractionObjectComponent::OnRegister()
{
	Super::OnRegister();
}

void UCISInteractionObjectComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCISInteractionObjectComponent, ObjectState);
	DOREPLIFETIME(UCISInteractionObjectComponent, InteractionSettings);
	DOREPLIFETIME(UCISInteractionObjectComponent, bObjectIsUsing);
}

void UCISInteractionObjectComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		SetObjectState(InitialObjectState);
	}
}

bool UCISInteractionObjectComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = true;

	for (auto& side : InteractionSettings.InteractionSides)
	{
		for (const auto& action : side.InteractionObjectSideActions)
		{
			if (action.InteractionObjectAction != nullptr)
			{
				bWroteSomething |= Channel->ReplicateSubobject(action.InteractionObjectAction, *Bunch, *RepFlags);
			}
		}

		if (side.InteractionObjectSideMoveHandler != nullptr)
		{
			bWroteSomething |= Channel->ReplicateSubobject(side.InteractionObjectSideMoveHandler, *Bunch, *RepFlags);
		}
	}

	return bWroteSomething;
}

void UCISInteractionObjectComponent::SetObjectIsUsing(bool bValue)
{
	bObjectIsUsing = bValue;

	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerSetObjectIsUsing(bValue);
	}
}

void UCISInteractionObjectComponent::ServerSetObjectIsUsing_Implementation(bool bValue)
{
	bObjectIsUsing = bValue;
}

bool UCISInteractionObjectComponent::ServerSetObjectIsUsing_Validate(bool bValue)
{
	return true;
}

bool UCISInteractionObjectComponent::IsObjectUsing() const
{
	return bObjectIsUsing;
}

void UCISInteractionObjectComponent::SetObjectState(const FGameplayTag& NewState)
{
	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerSetObjectState(NewState);
	}

	if (GetOwner()->GetLocalRole() != ROLE_Authority || ObjectState != NewState)
	{
		ObjectState = NewState;
		OnObjectStateChangedDelegate.Broadcast(NewState);
	}
}

void UCISInteractionObjectComponent::ServerSetObjectState_Implementation(const FGameplayTag& NewState)
{
	SetObjectState(NewState);
}

bool UCISInteractionObjectComponent::ServerSetObjectState_Validate(const FGameplayTag& NewState)
{
	return true;
}

UCISInteractionObjectAction* UCISInteractionObjectComponent::GetObjectActionForSide(const FCISInteractionObjectSide& Side) const
{
	if (const auto& sideData = GetSettings().InteractionSides.FindByPredicate([Side](const FCISInteractionObjectSide& OtherSide) { return OtherSide == Side; }))
	{
		const auto actionData = sideData->InteractionObjectSideActions.FindByPredicate([&](auto& it) { return it.RequiredStateTag == GetObjectState(); });

		return actionData != nullptr ? actionData->InteractionObjectAction : nullptr;
	}

	return nullptr;
}

void UCISInteractionObjectComponent::SetUsable(bool bValue)
{
	bUsable = bValue;

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		ServerSetUsable(bValue);
	}
}

const FGameplayTag& UCISInteractionObjectComponent::GetObjectState() const
{
	return ObjectState;
}

void UCISInteractionObjectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
#if WITH_EDITOR
	if ((TickType == ELevelTick::LEVELTICK_ViewportsOnly && (GetOwner()->IsSelectedInEditor() || GetOwner()->GetWorld()->WorldType == EWorldType::EditorPreview) && bWithDebugInEditor) || (TickType != LEVELTICK_ViewportsOnly && IsWithDebugInGame()))
	{
		if (IsInEditorWorld() && (!GCurrentLevelEditingViewportClient || GCurrentLevelEditingViewportClient->IsInGameView()))
		{
			return;
		}

		DrawDebug();
	}
#endif
}

bool UCISInteractionObjectComponent::IsWithDebugInGame() const
{
#if UE_BUILD_SHIPPING
	return false;
#else
	return bWithDebugInGame;
#endif
}

void UCISInteractionObjectComponent::DrawDebug()
{
	for (auto& side : InteractionSettings.InteractionSides)
	{
		if (side.DebugData.bEnableDebug)
		{
			const auto directionFromMin = UKismetMathLibrary::ComposeRotators(GetOwner()->GetActorRotation(), FRotator(0.f, side.InteractionAngleMin, 0.f)).Vector();
			const auto directionFromMax = UKismetMathLibrary::ComposeRotators(GetOwner()->GetActorRotation(), FRotator(0.f, side.InteractionAngleMax, 0.f)).Vector();
			const auto angleOfSegment = UKismetMathLibrary::DegAcos(UKismetMathLibrary::Dot_VectorVector(directionFromMin, directionFromMax));

			const auto direction = side.bAngleSide ? -1.f : 1.f;

			::DrawDebugCone(GetWorld(), GetOwner()->GetActorLocation(), (directionFromMin + directionFromMax) * direction, GetSettings().InteractionDistance, FMath::DegreesToRadians((side.bAngleSide ? (360.f - angleOfSegment) : angleOfSegment) * 0.5f), 0.f, 16, side.DebugData.DebugColor, false, 0.f, DebugLinesDephPriority, side.DebugData.LineThickness);
		}
	}
}

void UCISInteractionObjectComponent::OnRep_ObjectState()
{
	SetObjectState(ObjectState);
}

bool UCISInteractionObjectComponent::IsInEditorWorld() const
{
	if (GetWorld())
	{
		const auto worldType = GetWorld()->WorldType;

		if (worldType == EWorldType::PIE || worldType == EWorldType::Game || worldType == EWorldType::GamePreview)
		{
			return false;
		}

		if (worldType == EWorldType::Editor || worldType == EWorldType::EditorPreview)
		{
			return true;
		}
	}

	return false;
}

void UCISInteractionObjectComponent::ServerSetUsable_Implementation(bool bValue)
{
	bUsable = bValue;
}

bool UCISInteractionObjectComponent::ServerSetUsable_Validate(bool bValue)
{
	return true;
}

const FCISInteractionObjectSettings& UCISInteractionObjectComponent::GetSettings() const
{
	return InteractionSettings;
}
