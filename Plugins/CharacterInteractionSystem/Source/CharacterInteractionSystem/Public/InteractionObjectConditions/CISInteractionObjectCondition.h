// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "Common/CISTypes.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "CISInteractionObjectCondition.generated.h"

class ACharacter;
class UCISInteractionObjectComponent;
class UCISCharacterInteractionComponent;

/**
 * 
 */
UCLASS(Abstract, editinlinenew, DefaultToInstanced, Blueprintable, meta = (ShowWorldContextPin))
class CHARACTERINTERACTIONSYSTEM_API UCISInteractionObjectCondition : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, DisplayName = "Check Condition")
	bool CheckCondition(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, UPARAM(ref) const FCISInteractionObjectSide& InteractionObjectSide) const;
};
