// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "Common/CISReplicatedObject.h"
#include "Common/CISTypes.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"

#include "CISInteractionObjectAction.generated.h"

class ACharacter;
class UCISInteractionObjectComponent;
class UCISCharacterInteractionComponent;

UCLASS(Abstract, editinlinenew, DefaultToInstanced, Blueprintable, meta = (ShowWorldContextPin))
class CHARACTERINTERACTIONSYSTEM_API UCISInteractionObjectAction : public UCISReplicatedObject
{
	GENERATED_BODY()
public:
	DECLARE_EVENT_TwoParams(UCISInteractionObjectAction, FCISOnInteractionObjectActionFinished, UCISInteractionObjectAction*, bool /*bInterrupted*/);
	FCISOnInteractionObjectActionFinished OnInteractionObjectActionFinishedDelegate;

	virtual void FinishActionInternal(TWeakObjectPtr<ACharacter> Character, TWeakObjectPtr<UCISCharacterInteractionComponent> CharacterInteractionComponent, TWeakObjectPtr<UCISInteractionObjectComponent> InteractionObjectComponent, bool bInterrupted);

	void StartInteractionObjectActionInternal(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide);

protected:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartObjectAction(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartObjectAction(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, const FCISInteractionObjectSide& InteractionObjectSide);

	UFUNCTION(BlueprintNativeEvent)
	void StartInteractionObjectAction(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, UPARAM(ref) const FCISInteractionObjectSide& InteractionObjectSide);

	UFUNCTION(BlueprintNativeEvent)
	void OnUseFinished(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, bool bWasInterrupted);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFinishAction(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, bool bInterrupted);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFinishAction(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, bool bInterrupted);

	UFUNCTION(BlueprintCallable, Category = "CIS|InteractionObjectAction")
	virtual void FinishAction();

protected:
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle), Category = "Settings")
	bool bSetNewStateAfterFinishAction = true;

	UPROPERTY(EditAnywhere, meta = (EditCondition = bSetNewStateAfterFinishAction), Category = "Settings")
	FGameplayTag NewStateAfterFinishAction;

	TWeakObjectPtr<UCISInteractionObjectComponent> CachedInteractionObject;

	TWeakObjectPtr<ACharacter> CachedCharacter;

	TWeakObjectPtr<UCISCharacterInteractionComponent> CachedCharacterInteractionComponent;

	bool bActionStarted = false;
};
