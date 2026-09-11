// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#include "Common/CISRootMotionMovementTask.h"

#include "Components/CapsuleComponent.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UCISRootMotionMovementTask* UCISRootMotionMovementTask::RootMotionMoveTo(ACharacter* Character, FVector TargetLocation, ECollisionChannel CollisionCheckTraceType /*= ECC_WorldStatic*/, float MoveToDistance /*= 0.f*/, EMovementMode MovementMode /*= EMovementMode::MOVE_Walking*/, ERootMotionAccumulateMode RootMotionAccumulateMode /*= ERootMotionAccumulateMode::Override*/, bool bWithCustomMoveSpeed /*= false*/, float CustomMoveSpeed /*= 0.f*/, float TimeOut /*= 2.f*/, float VerticalOffset /*= 0.f */)
{
	if (Character != nullptr)
	{
		auto newTask = NewObject<UCISRootMotionMovementTask>(Character);
		newTask->TargetLocation = TargetLocation;
		newTask->Character = Character;
		newTask->CustomMovementMode = MovementMode;
		newTask->bOverrideMoveSpeed = bWithCustomMoveSpeed;
		newTask->CustomMoveSpeed = CustomMoveSpeed;
		newTask->MovementDistance = MoveToDistance;
		newTask->CollisionCheckTraceType = CollisionCheckTraceType;
		newTask->RootMotionAccumulateMode = RootMotionAccumulateMode;
		newTask->TimeOut = TimeOut;
		newTask->VerticalOffset = VerticalOffset;
		return newTask;
	}

	return nullptr;
}

void UCISRootMotionMovementTask::FinishMovement()
{
	if (!bFinished)
	{
		bFinished = true;

		if (Character.IsValid())
		{
			Character->GetCharacterMovement()->RemoveRootMotionSourceByID(RootMotionID);

			if (bOverrideMoveSpeed)
			{
				Character->GetCharacterMovement()->MaxWalkSpeed = OriginMoveSpeed;
			}

			Character->GetCharacterMovement()->SetMovementMode(OriginMovementMode);
		}

		OnRootMotionMovementFinished.Broadcast();

		SetReadyToDestroy();
	}
}

void UCISRootMotionMovementTask::Activate()
{
	Super::Activate();

	if (!Character.IsValid())
	{
		FinishMovement();
		return;
	}

	RegisterWithGameInstance(Character.Get());

	bInitialized = true;

	if (bOverrideMoveSpeed)
	{
		OriginMoveSpeed = Character->GetCharacterMovement()->MaxWalkSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeed = CustomMoveSpeed;
	}

	OriginMovementMode = Character->GetCharacterMovement()->MovementMode;
	Character->GetCharacterMovement()->SetMovementMode(CustomMovementMode);

	auto moveSpeed = Character->GetCharacterMovement()->MaxWalkSpeed;

	FHitResult hitResult;

	UKismetSystemLibrary::LineTraceSingle(this, TargetLocation + FVector(0.f, 0.f, 5.f), TargetLocation - FVector(0.f, 0.f, 500.f), UEngineTypes::ConvertToTraceType(CollisionCheckTraceType), false, { Character.Get() }, EDrawDebugTrace::None, hitResult, true);

	FixedTargetLocation = hitResult.bBlockingHit ? (hitResult.Location + FVector(0.f, 0.f, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + VerticalOffset)) : TargetLocation;

	auto forceName = FName("CISRootMotionMovementTask");
	auto moveToForce = MakeShared<FRootMotionSource_MoveToForce>();
	moveToForce->InstanceName = forceName;
	moveToForce->AccumulateMode = RootMotionAccumulateMode;
	//moveToForce->Settings.SetFlag(ERootMotionSourceSettingsFlags::UseSensitiveLiftoffCheck);
	moveToForce->Priority = 1000;
	moveToForce->TargetLocation = FixedTargetLocation;
	moveToForce->StartLocation = Character->GetActorLocation();
	moveToForce->Duration = FMath::Max((Character->GetActorLocation() - FixedTargetLocation).Size() / moveSpeed, 0.05f);
	moveToForce->bRestrictSpeedToExpected = false;
	moveToForce->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
	moveToForce->FinishVelocityParams.SetVelocity = FVector::ZeroVector;

	RootMotionID = Character->GetCharacterMovement()->ApplyRootMotionSource(moveToForce);
	OnRootMotionMovementCreated.Broadcast(this);
}

TStatId UCISRootMotionMovementTask::GetStatId() const
{
	return TStatId();
}

void UCISRootMotionMovementTask::Tick(float DeltaTime)
{
	if (bInitialized && !bFinished)
	{
		if (!Character.IsValid())
		{
			FinishMovement();
			return;
		}

		CurrentTime += DeltaTime;

		if (CurrentTime >= TimeOut)
		{
			FinishMovement();
			OnTimeOut.Broadcast();
		}

		auto rmSource = Character->GetCharacterMovement()->GetRootMotionSourceByID(RootMotionID);

		if (!rmSource.IsValid() || rmSource->Status.HasFlag(ERootMotionSourceStatusFlags::Finished))
		{
			FinishMovement();
		}
	}
}
