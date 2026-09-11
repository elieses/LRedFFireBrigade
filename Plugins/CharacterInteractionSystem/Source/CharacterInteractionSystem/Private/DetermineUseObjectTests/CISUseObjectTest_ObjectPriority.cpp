// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#include "DetermineUseObjectTests/CISUseObjectTest_ObjectPriority.h"

#include "Components/CISCharacterInteractionComponent.h"
#include "Components/CISInteractionObjectComponent.h"
#include "GameFramework/Character.h"

float UCISUseObjectTest_ObjectPriority::CalculateObjectScore_Implementation(ACharacter* InteractionCharacter, const UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide, float& CurrentScore) const
{
	return (float)InteractionObjectComponent->GetSettings().ObjectPriority;
}
