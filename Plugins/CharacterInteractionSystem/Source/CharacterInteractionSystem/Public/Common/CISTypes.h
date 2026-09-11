// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "CISTypes.generated.h"

class UUserWidget;
class UCISInteractionObjectHandler;
class UCISInteractionObjectComponent;
class ACharacter;
class UCISInteractionObjectCondition;
class UCISInteractionObjectMoveHandler;
class UCISInteractionObjectScript;
class UCISInteractionObjectAction;
class UCISDetermineUseObjectTest;
class UCISInteractionDataObject;

USTRUCT(BlueprintType)
struct CHARACTERINTERACTIONSYSTEM_API FCISPrimitiveComponentsOnActor
{
public:
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FString SelectedPrimitiveComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FName SocketName;
};

USTRUCT(BlueprintType)
struct CHARACTERINTERACTIONSYSTEM_API FCISSceneComponentsOnActor
{
public:
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FString SelectedSceneComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FName SocketName;
};

UENUM()
enum class ECISObjectInterruptionType : uint8
{
	None,
	InterruptAfterReleaseButton,
	InterruptAfterSecondPressButton
};

UENUM()
enum class ECISScriptExecutionType : uint8
{
	UsingStarted,
	UsingEnded,
	MovementToObjectFinished
};

UENUM()
enum class ECISActionExecutionType : uint8
{
	StartAfterFinishMoveHandler,
	StartParallelWithMoveHandler
};

USTRUCT()
struct FCISInteractionObjectSideDebugData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bEnableDebug = true;

	UPROPERTY(EditAnywhere, Category = "Settings")
	FColor DebugColor = FColor::Yellow;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 1.f, UIMin = 1.f), Category = "Settings")
	float LineThickness = 2.f;
};

USTRUCT()
struct FCISInteractionObjectActionData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Settings")
	FGameplayTag RequiredStateTag;

	UPROPERTY(EditAnywhere, Instanced, Category = "Settings")
	UCISInteractionObjectAction* InteractionObjectAction;

	FORCEINLINE bool operator==(const FCISInteractionObjectActionData& OtherData) const
	{
		return OtherData.RequiredStateTag == RequiredStateTag;
	}

	FORCEINLINE bool operator!=(const FCISInteractionObjectActionData& OtherData) const
	{
		return !(*this == OtherData);
	}
};

USTRUCT(BlueprintType)
struct FCISInteractionObjectSide
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (ClampMin = -180.f, UIMin = -180.f, ClampMax = 180.f, UIMax = 180.f), Category = "Settings")
	float InteractionAngleMin = 0.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = -180.f, UIMin = -180.f, ClampMax = 180.f, UIMax = 180.f), Category = "Settings")
	float InteractionAngleMax = 0.f;

	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bAngleSide = false;

	UPROPERTY(EditAnywhere, Instanced, Category = "Settings")
	UCISInteractionObjectMoveHandler* InteractionObjectSideMoveHandler;

	UPROPERTY(EditAnywhere, Category = "Settings")
	TArray<FCISInteractionObjectActionData> InteractionObjectSideActions;

	UPROPERTY(EditAnywhere, Category = "Settings")
	ECISActionExecutionType ActionExecutionType = ECISActionExecutionType::StartAfterFinishMoveHandler;

	UPROPERTY(EditAnywhere, Category = "Settings")
	FCISInteractionObjectSideDebugData DebugData;

	FORCEINLINE bool operator==(const FCISInteractionObjectSide& OtherSide) const
	{
		return FMath::IsNearlyEqual(OtherSide.InteractionAngleMax, InteractionAngleMax) && FMath::IsNearlyEqual(OtherSide.InteractionAngleMin, InteractionAngleMin);
	}

	FORCEINLINE bool operator!=(const FCISInteractionObjectSide& OtherSide) const
	{
		return !(*this == OtherSide);
	}
};
