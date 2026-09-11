// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#include "DetermineUseObjectTests/CISDetermineUseObjectTest.h"

#include "CharacterInteractionSystem.h"
#include "Components/CISCharacterInteractionComponent.h"
#include "Components/CISInteractionObjectComponent.h"
#include "GameFramework/Character.h"

float UCISDetermineUseObjectTest::CalculateObjectScoreInternal(ACharacter* InteractionCharacter, const UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide, float& CurrentScore) const
{
	float score = 0.f;

	score = CalculateObjectScore(InteractionCharacter, CharacterInteractionComponent, InteractionObjectComponent, InteractionObjectSide, CurrentScore);
	score = FMath::Clamp(score, 0.f, score);

	const auto multipliedScore = score * TestMultiplier;

	if (bWithDebug)
	{
		CISMessage(FString::Printf(TEXT("Score: %.2f"), score), DebugTime, DebugColor);
		CISMessage(FString::Printf(TEXT("Multiplied Score: %.2f"), multipliedScore), DebugTime, DebugColor);
		CISMessage(FString::Printf(TEXT("Test: %s"), *GetName()), DebugTime, DebugColor);
		CISMessage(FString::Printf(TEXT("Object: %s"), *InteractionObjectComponent->GetOwner()->GetName()), DebugTime, FColor::Green);
	}

	return multipliedScore;
}

float UCISDetermineUseObjectTest::CalculateObjectScore_Implementation(ACharacter* InteractionCharacter, const UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide, float& CurrentScore) const
{
	return 0.f;
}

float UCISDetermineUseObjectTest::GetValueFromCurve(float InTime) const
{
	return CalculationCurve.ExternalCurve != nullptr ? CalculationCurve.ExternalCurve->GetFloatValue(InTime) : CalculationCurve.EditorCurveData.Eval(InTime);
}
