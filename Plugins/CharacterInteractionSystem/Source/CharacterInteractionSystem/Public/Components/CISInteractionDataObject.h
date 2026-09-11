// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "Common/CISTypes.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"

#include "CISInteractionDataObject.generated.h"

class UUserWidget;
class UCISInteractionObjectHandler;
class UCISInteractionObjectComponent;
class ACharacter;
class UCISInteractionObjectCondition;
class UCISInteractionObjectMoveHandler;
class UCISInteractionObjectAction;
class AActor;

DECLARE_MULTICAST_DELEGATE_TwoParams(FCISOnUseDataObject, TWeakObjectPtr<UCISInteractionObjectComponent> /*InteractionObjectComponent*/, TWeakObjectPtr<UCISInteractionDataObject> /*InteractionDataObject*/);

UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class CHARACTERINTERACTIONSYSTEM_API UCISInteractionDataObject : public UObject
{
	GENERATED_BODY()
public:
	virtual void Init(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent);

	bool IsCharacterCanUseObject() const;
	bool IsObjectVisible() const;
	bool IsOverlapped() const;
	bool IsCurrentUseSideValid() const;

protected:
	friend class UCISCharacterInteractionComponent;
	void Overlapped(bool bOverlapped);

	virtual void Tick(float DeltaSeconds);
	virtual bool CheckVisibleConditions() const;
	virtual bool CheckUsableConditions() const;
	virtual bool CheckAngleAndDistance() const;
	virtual void DetermineUseSide();

	virtual void UseObject();
	virtual void InterruptUsing();

	void ResetMovementHandler();

	void SetCurrentUseSide(const FCISInteractionObjectSide* NewSide);

	UCISInteractionObjectComponent* GetInteractionObjectComponent() const;
	UCISCharacterInteractionComponent* GetCharacterInteractionComponent() const;
	ACharacter* GetInteractionCharacter() const;

	virtual void OnDataRemovedFromCharacterComponent();

	const FCISInteractionObjectSide& GetCurrentSide() const;

	virtual void OnUsableChanged(bool bCanUse);

	virtual void OnVisibleChanged(bool bEnter);

	virtual void FinishUseObject(bool bInterrupted);

	virtual void OnMovementToObjectFinished();

	void StartObjectAction();

	virtual void OnObjectActionUseFinished(UCISInteractionObjectAction* Action, bool bInterrupted);

	void SetUsable(bool bValue);
	void SetVisible(bool bEnter);

	bool IsInitialized() const;
	bool IsUsingObject() const;

	UFUNCTION(BlueprintNativeEvent)
	void OnInitialized(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent);

	UFUNCTION(BlueprintNativeEvent)
	void OnOverlapped(ACharacter* InteractionCharacter, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, bool bOverlap);

	UFUNCTION(BlueprintNativeEvent)
	void OnObjectUseFinished(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, bool bInterrupted);

	UFUNCTION(BlueprintNativeEvent)
	void OnObjectActionStarted(UCISInteractionObjectAction* Action, ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent);

	UFUNCTION(BlueprintNativeEvent)
	void OnObjectUseStarted(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent);

	UFUNCTION(BlueprintNativeEvent)
	void OnVisibleChanged(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, bool bEnter);

	UFUNCTION(BlueprintNativeEvent)
	void OnUsableChanged(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent, bool bUsable);

	UFUNCTION(BlueprintNativeEvent)
	void OnMovementStarted(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent);

	UFUNCTION(BlueprintNativeEvent)
	void OnMovementEnded(ACharacter* Character, UCISCharacterInteractionComponent* CharacterInteractionComponent, UCISInteractionObjectComponent* InteractionObjectComponent);

protected:
	FCISOnUseDataObject OnEndUseObjectDelegate;
	TWeakObjectPtr<UCISInteractionObjectMoveHandler> CurrentMoveHandler;
	TOptional<FCISInteractionObjectSide> CurrentSide;
	TWeakObjectPtr<UCISInteractionObjectAction> CurrentAction;

	TOptional<bool> ObjectUsable;
	TOptional<bool> ObjectVisible;
	bool bWasOverlapped = false;
	bool bInitialized = false;

	bool bUsingObject = false;

	UPROPERTY(BlueprintReadOnly, Category = "CIS|InteractionDataObject")
	TWeakObjectPtr<UCISInteractionObjectComponent> CachedInteractionObjectComponent;

	UPROPERTY(BlueprintReadOnly, Category = "CIS|InteractionDataObject")
	TWeakObjectPtr<ACharacter> CachedInteractionCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "CIS|InteractionDataObject")
	TWeakObjectPtr<UCISCharacterInteractionComponent> CachedCharacterInteractionComponent;
};
