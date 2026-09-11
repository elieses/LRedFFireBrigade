// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "Common/CISReplicatedObject.h"
#include "Common/CISTypes.h"
#include "CoreMinimal.h"
#include "Tickable.h"
#include "UObject/NoExportTypes.h"

#include "CISInteractionObjectMoveHandler.generated.h"

class ACharacter;
class UCISInteractionObjectComponent;
class UCISCharacterInteractionComponent;

DECLARE_MULTICAST_DELEGATE(FCISOnMovementFinished);

UCLASS(Abstract, editinlinenew, DefaultToInstanced, Blueprintable, meta = (ShowWorldContextPin))
class CHARACTERINTERACTIONSYSTEM_API UCISInteractionObjectMoveHandler : public UCISReplicatedObject
    , public FTickableGameObject
{
	GENERATED_BODY()
public:
	void StartMovementInternal(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide);
	void MovementAbortedInternal(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide);

	UFUNCTION(BlueprintCallable, Category = "CIS|InteractionObjectMoveHandler")
	void FinishMovement();

	void InvalidateHandler();

protected:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartMovement(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartMovement(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastMovementAborted(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide);
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMovementAborted(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide);

	UFUNCTION(BlueprintNativeEvent)
	void StartMovement(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, UPARAM(ref) const FCISInteractionObjectSide& InteractionObjectSide);

	UFUNCTION(BlueprintNativeEvent)
	void HandlerTick(float DeltaTime, ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, UPARAM(ref) const FCISInteractionObjectSide& InteractionObjectSide);

	UFUNCTION(BlueprintNativeEvent)
	void MovementAborted(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, UPARAM(ref) const FCISInteractionObjectSide& InteractionObjectSide);

	UFUNCTION(BlueprintNativeEvent)
	void MovementFinished(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, UPARAM(ref) const FCISInteractionObjectSide& InteractionObjectSide);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastInvalidateHandler();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerInvalidateHandler();

	virtual TStatId GetStatId() const override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFinishMovement();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFinishMovement();

	UFUNCTION(BlueprintPure, Category = "CIS|InteractionObjectMoveHandler")
	bool IsMovementFinished() const;

public:
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bContinueTickAfterFinishMovement = false;

protected:
	TWeakObjectPtr<ACharacter> CachedCharacter;
	TWeakObjectPtr<UCISCharacterInteractionComponent> CachedCharacterInteractionComponent;
	TWeakObjectPtr<UCISInteractionObjectComponent> CachedInteractionObjectComponent;
	TSharedPtr<FCISInteractionObjectSide> CachedInteractionObjectSide;

	bool bMovementStarted = false;

	bool bMovementFinished = false;

public:
	FCISOnMovementFinished OnMovementFinishedDelegate;
};
