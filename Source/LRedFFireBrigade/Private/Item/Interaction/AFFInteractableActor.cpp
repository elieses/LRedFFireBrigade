// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Interaction/AFFInteractableActor.h"
#include "Components/SceneComponent.h"

AFFInteractableActor::AFFInteractableActor()
{
}


void AFFInteractableActor::BeginPlay()
{
	Super::BeginPlay();

	// 把当前状态置为初始状态（变化时广播 OnStateChanged）
	SetState(InitialState);
}


void AFFInteractableActor::SetState(const FGameplayTag& NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		OnStateChanged.Broadcast(NewState);
	}
}


void AFFInteractableActor::Interact(AActor* Interactor)
{
}


void AFFInteractableActor::RegisterActionPoint(USceneComponent* Point)
{
	if (Point)
	{
		ActionPoints.Add(Point);
	}
}


USceneComponent* AFFInteractableActor::SelectActionPoint(AActor* User) const
{
	for (const TWeakObjectPtr<USceneComponent>& WeakPoint : ActionPoints)
	{
		if (WeakPoint.IsValid())
		{
			return WeakPoint.Get();
		}
	}
	return nullptr;
}


FRotator AFFInteractableActor::GetActionFacing(USceneComponent* Point, AActor* User) const
{
	if (!Point)
	{
		return GetActorRotation();
	}

	FRotator Rot = Point->GetComponentRotation();
	Rot.Pitch = 0.f;
	Rot.Roll = 0.f;
	return Rot;
}


bool AFFInteractableActor::GetInteractionStand(AActor* User, FVector& OutLocation, FRotator& OutRotation) const
{
	USceneComponent* Point = SelectActionPoint(User);
	if (!Point)
	{
		return false;
	}

	OutLocation = Point->GetComponentLocation();
	OutRotation = GetActionFacing(Point, User);
	return true;
}
