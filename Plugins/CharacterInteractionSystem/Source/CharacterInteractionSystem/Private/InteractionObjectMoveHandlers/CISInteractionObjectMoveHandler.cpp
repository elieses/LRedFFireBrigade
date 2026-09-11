// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#include "InteractionObjectMoveHandlers/CISInteractionObjectMoveHandler.h"

#include "Components/CISCharacterInteractionComponent.h"
#include "Components/CISInteractionObjectComponent.h"
#include "GameFramework/Character.h"

/*Implementable events*/
void UCISInteractionObjectMoveHandler::StartMovement_Implementation(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
}

void UCISInteractionObjectMoveHandler::HandlerTick_Implementation(float DeltaTime, ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
}

void UCISInteractionObjectMoveHandler::MovementAborted_Implementation(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
}
/*Implementable events*/

void UCISInteractionObjectMoveHandler::StartMovementInternal(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
	if (InteractionCharacter->IsLocallyControlled())
	{
		ServerStartMovement(InteractionCharacter, CharacterInteractionComponent, InteractionObjectComponent, InteractionObjectSide);
	}

	CachedCharacter = InteractionCharacter;
	CachedCharacterInteractionComponent = CharacterInteractionComponent;
	CachedInteractionObjectComponent = InteractionObjectComponent;
	CachedInteractionObjectSide = MakeShared<FCISInteractionObjectSide>(InteractionObjectSide);

	bMovementStarted = true;

	StartMovement(InteractionCharacter, CharacterInteractionComponent, InteractionObjectComponent, InteractionObjectSide);
}

void UCISInteractionObjectMoveHandler::MovementAbortedInternal(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
	if (InteractionCharacter->IsLocallyControlled())
	{
		ServerMovementAborted(InteractionCharacter, CharacterInteractionComponent, InteractionObjectComponent, InteractionObjectSide);
	}

	MovementAborted(InteractionCharacter, CharacterInteractionComponent, InteractionObjectComponent, InteractionObjectSide);
}

void UCISInteractionObjectMoveHandler::ServerStartMovement_Implementation(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
	MulticastStartMovement(InteractionCharacter, CharacterInteractionComponent, InteractionObjectComponent, InteractionObjectSide);
}

bool UCISInteractionObjectMoveHandler::ServerStartMovement_Validate(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
	return true;
}

void UCISInteractionObjectMoveHandler::MulticastStartMovement_Implementation(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
	if (!InteractionCharacter->IsLocallyControlled())
	{
		StartMovementInternal(InteractionCharacter, CharacterInteractionComponent, InteractionObjectComponent, InteractionObjectSide);
	}
}

void UCISInteractionObjectMoveHandler::MulticastMovementAborted_Implementation(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
	if (!InteractionCharacter->IsLocallyControlled())
	{
		MovementAbortedInternal(InteractionCharacter, CharacterInteractionComponent, InteractionObjectComponent, InteractionObjectSide);
	}
}

void UCISInteractionObjectMoveHandler::ServerMovementAborted_Implementation(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
	MulticastMovementAborted(InteractionCharacter, CharacterInteractionComponent, InteractionObjectComponent, InteractionObjectSide);
}

bool UCISInteractionObjectMoveHandler::ServerMovementAborted_Validate(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
	return true;
}

void UCISInteractionObjectMoveHandler::MovementFinished_Implementation(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide)
{
}

void UCISInteractionObjectMoveHandler::MulticastInvalidateHandler_Implementation()
{
	if (CachedCharacter.IsValid() && !CachedCharacter->IsLocallyControlled())
	{
		InvalidateHandler();
	}
}

void UCISInteractionObjectMoveHandler::ServerInvalidateHandler_Implementation()
{
	MulticastInvalidateHandler();
}

bool UCISInteractionObjectMoveHandler::ServerInvalidateHandler_Validate()
{
	return true;
}

TStatId UCISInteractionObjectMoveHandler::GetStatId() const
{
	return TStatId();
}

void UCISInteractionObjectMoveHandler::Tick(float DeltaTime)
{
	if (bMovementStarted && (!bMovementFinished || bContinueTickAfterFinishMovement) && CachedInteractionObjectComponent.IsValid() && CachedCharacter.IsValid() && CachedCharacterInteractionComponent.Get()->GetOwner() == CachedCharacter.Get() && CachedInteractionObjectComponent->IsObjectUsing())
	{
		HandlerTick(DeltaTime, CachedCharacter.Get(), CachedCharacterInteractionComponent.Get(), CachedInteractionObjectComponent.Get(), *CachedInteractionObjectSide.Get());
	}
}

bool UCISInteractionObjectMoveHandler::IsMovementFinished() const
{
	return bMovementFinished;
}

void UCISInteractionObjectMoveHandler::FinishMovement()
{
	if (!bMovementFinished)
	{
		if (CachedCharacter.IsValid() && CachedCharacter->IsLocallyControlled())
		{
			ServerFinishMovement();
		}

		bMovementFinished = true;

		MovementFinished(CachedCharacter.Get(), CachedCharacterInteractionComponent.Get(), CachedInteractionObjectComponent.Get(), *CachedInteractionObjectSide.Get());
		OnMovementFinishedDelegate.Broadcast();
	}
}

void UCISInteractionObjectMoveHandler::InvalidateHandler()
{
	if (CachedCharacter.IsValid() && CachedCharacter->IsLocallyControlled())
	{
		ServerInvalidateHandler();
	}

	CachedCharacter.Reset();
	CachedCharacterInteractionComponent.Reset();
	CachedInteractionObjectComponent.Reset();
	CachedInteractionObjectSide.Reset();
	bMovementFinished = false;
	bMovementStarted = false;
}

void UCISInteractionObjectMoveHandler::MulticastFinishMovement_Implementation()
{
	if (CachedCharacter.IsValid() && !CachedCharacter->IsLocallyControlled())
	{
		FinishMovement();
	}
}

void UCISInteractionObjectMoveHandler::ServerFinishMovement_Implementation()
{
	MulticastFinishMovement();
}

bool UCISInteractionObjectMoveHandler::ServerFinishMovement_Validate()
{
	return true;
}
