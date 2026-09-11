// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#include "InteractionObjectConditions/CISInteractionObjectCondition.h"

#include "Components/CISCharacterInteractionComponent.h"
#include "Components/CISInteractionObjectComponent.h"
#include "GameFramework/Character.h"

bool UCISInteractionObjectCondition::CheckCondition_Implementation(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide) const
{
	return true;
}
