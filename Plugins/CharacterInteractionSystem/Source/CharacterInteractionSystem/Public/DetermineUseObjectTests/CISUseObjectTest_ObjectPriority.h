// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DetermineUseObjectTests/CISDetermineUseObjectTest.h"

#include "CISUseObjectTest_ObjectPriority.generated.h"

class ACharacter;
class UCISCharacterInteractionComponent;
class UCISInteractionObjectComponent;

/**
 * 
 */
UCLASS()
class CHARACTERINTERACTIONSYSTEM_API UCISUseObjectTest_ObjectPriority : public UCISDetermineUseObjectTest
{
	GENERATED_BODY()

public:
	virtual float CalculateObjectScore_Implementation(ACharacter* InteractionCharacter, const UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide, float& CurrentScore) const override;
};
