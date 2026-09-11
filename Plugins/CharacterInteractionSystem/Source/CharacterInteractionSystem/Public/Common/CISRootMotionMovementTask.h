// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/RootMotionSource.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Tickable.h"

#include "CISRootMotionMovementTask.generated.h"

class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCISOnRootMotionMovementEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCISOnRootMotionMovementCreated, UCISRootMotionMovementTask*, Task);
UCLASS(Blueprintable)
class CHARACTERINTERACTIONSYSTEM_API UCISRootMotionMovementTask : public UBlueprintAsyncActionBase
    , public FTickableGameObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "bWithCustomMoveSpeed, CustomMoveSpeed, VerticalOffset", BlueprintInternalUseOnly = "true"), Category = "CIS|RootMotionMovement")
	static UCISRootMotionMovementTask* RootMotionMoveTo(
	    ACharacter* Character,
	    FVector TargetLocation,
	    ECollisionChannel CollisionCheckTraceType = ECC_WorldStatic,
	    float MoveToDistance = 0.f,
	    EMovementMode MovementMode = EMovementMode::MOVE_Walking,
	    ERootMotionAccumulateMode RootMotionAccumulateMode = ERootMotionAccumulateMode::Override,
	    bool bWithCustomMoveSpeed = false,
	    float CustomMoveSpeed = 0.f,
	    float TimeOut = 2.f,
	    float VerticalOffset = 0.f);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "StopMovement"), Category = "CIS|RootMotionMovement")
	void FinishMovement();

protected:
	virtual void Activate() override;

	virtual TStatId GetStatId() const override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "CIS|Delegates")
	FCISOnRootMotionMovementCreated OnRootMotionMovementCreated;

	UPROPERTY(BlueprintAssignable, Category = "CIS|Delegates")
	FCISOnRootMotionMovementEvent OnRootMotionMovementFinished;

	UPROPERTY(BlueprintAssignable, Category = "CIS|Settings")
	FCISOnRootMotionMovementEvent OnTimeOut;

protected:
	FVector TargetLocation;
	FVector FixedTargetLocation;
	EMovementMode OriginMovementMode;
	EMovementMode CustomMovementMode;
	TWeakObjectPtr<ACharacter> Character;
	float OriginMoveSpeed = 0.f;
	float CustomMoveSpeed = 0.f;
	bool bOverrideMoveSpeed = false;
	float MovementDistance = 0.f;
	ECollisionChannel CollisionCheckTraceType;
	ERootMotionAccumulateMode RootMotionAccumulateMode;
	uint16 RootMotionID = 0;
	float TimeOut = 0.f;

	float VerticalOffset = 0.f;

	float CurrentTime = 0.f;

	bool bFinished = false;
	bool bInitialized = false;
};
