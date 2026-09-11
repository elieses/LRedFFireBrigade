// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#include "InteractionObjectActions/CISInteractionObjectAction.h"

#include "Components/CISCharacterInteractionComponent.h"
#include "Components/CISInteractionObjectComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

void UCISInteractionObjectAction::FinishAction()
{
	if (bActionStarted)
	{
		FinishActionInternal(CachedCharacter, CachedCharacterInteractionComponent, CachedInteractionObject, false);
	}
}

void UCISInteractionObjectAction::FinishActionInternal(TWeakObjectPtr<ACharacter> Character, TWeakObjectPtr<UCISCharacterInteractionComponent> CharacterInteractionComponent, TWeakObjectPtr<UCISInteractionObjectComponent> InteractionObjectComponent, bool bInterrupted)
{
	if (Character.IsValid() && Character->IsLocallyControlled() && CharacterInteractionComponent.IsValid() && InteractionObjectComponent.IsValid())
	{
		ServerFinishAction(Character.Get(), CharacterInteractionComponent.Get(), InteractionObjectComponent.Get(), bInterrupted);
	}

	if (InteractionObjectComponent.IsValid())
	{
		if (!bInterrupted && bSetNewStateAfterFinishAction)
		{
			InteractionObjectComponent->SetObjectState(NewStateAfterFinishAction);
		}
	}

	bActionStarted = false;

	OnUseFinished(Character.Get(), CharacterInteractionComponent.Get(), InteractionObjectComponent.Get(), bInterrupted);

	OnInteractionObjectActionFinishedDelegate.Broadcast(this, bInterrupted);
}

void UCISInteractionObjectAction::StartInteractionObjectActionInternal(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
	if (InteractionCharacter->IsLocallyControlled())
	{
		ServerStartObjectAction(InteractionCharacter, CharacterInteractionComponent, InteractionObjectComponent, InteractionObjectSide);
	}

	bActionStarted = true;

	CachedCharacterInteractionComponent = CharacterInteractionComponent;
	CachedCharacter = InteractionCharacter;
	CachedInteractionObject = InteractionObjectComponent;
	StartInteractionObjectAction(InteractionCharacter, CharacterInteractionComponent, InteractionObjectComponent, InteractionObjectSide);
}

void UCISInteractionObjectAction::OnUseFinished_Implementation(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, bool bWasInterrupted)
{
}

bool UCISInteractionObjectAction::ServerFinishAction_Validate(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, bool bInterrupted)
{
	return true;
}

bool UCISInteractionObjectAction::ServerStartObjectAction_Validate(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
	return true;
}

void UCISInteractionObjectAction::MulticastFinishAction_Implementation(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, bool bInterrupted)
{
	if (!Character || !Character->IsLocallyControlled())
	{
		FinishActionInternal(Character, CharacterInteractionComponent, InteractionObjectComponent, bInterrupted);
	}
}

void UCISInteractionObjectAction::ServerFinishAction_Implementation(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, bool bInterrupted)
{
	MulticastFinishAction(Character, CharacterInteractionComponent, InteractionObjectComponent, bInterrupted);
}

void UCISInteractionObjectAction::MulticastStartObjectAction_Implementation(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
	if (!InteractionCharacter->IsLocallyControlled())
	{
		StartInteractionObjectActionInternal(InteractionCharacter, CharacterInteractionComponent, InteractionObjectComponent, InteractionObjectSide);
	}
}

void UCISInteractionObjectAction::ServerStartObjectAction_Implementation(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
	MulticastStartObjectAction(InteractionCharacter, CharacterInteractionComponent, InteractionObjectComponent, InteractionObjectSide);
}

void UCISInteractionObjectAction::StartInteractionObjectAction_Implementation(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
}
