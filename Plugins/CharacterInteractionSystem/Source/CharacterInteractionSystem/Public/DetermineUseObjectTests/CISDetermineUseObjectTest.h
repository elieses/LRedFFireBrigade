// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "Common/CISTypes.h"
#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "UObject/NoExportTypes.h"

#include "CISDetermineUseObjectTest.generated.h"

class ACharacter;
class UCISInteractionObjectComponent;
class UCISCharacterInteractionComponent;

UCLASS(Abstract, editinlinenew, DefaultToInstanced, Blueprintable, meta = (ShowWorldContextPin))
class CHARACTERINTERACTIONSYSTEM_API UCISDetermineUseObjectTest : public UObject
{
	GENERATED_BODY()
public:
	float CalculateObjectScoreInternal(ACharacter* InteractionCharacter, const UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide, float& CurrentScore) const;

protected:
	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "Calculate Object Score"), Category = "CIS|DetermineObjectTest")
	float CalculateObjectScore(ACharacter* InteractionCharacter, const UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, UPARAM(ref) const FCISInteractionObjectSide& InteractionObjectSide, UPARAM(ref) float& CurrentScore) const;

	UFUNCTION(BlueprintPure, Category = "CIS|DetermineObjectTest")
	float GetValueFromCurve(float InTime) const;

protected:
	UPROPERTY(EditAnywhere, Category = "Settings")
	FRuntimeFloatCurve CalculationCurve;

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ClampMin = 0.f, UIMin = 0.f))
	float TestMultiplier = 1.f;

	UPROPERTY(EditAnywhere, Category = "Settings|Debug")
	bool bWithDebug = false;

	UPROPERTY(EditAnywhere, Category = "Settings|Debug", meta = (ClampMin = 0.f, UIMin = 0.f, EditCondition = "bWithDebug"))
	float DebugTime = 5.f;

	UPROPERTY(EditAnywhere, Category = "Settings|Debug", meta = (EditCondition = "bWithDebug"))
	FColor DebugColor = FColor::Yellow;
};
