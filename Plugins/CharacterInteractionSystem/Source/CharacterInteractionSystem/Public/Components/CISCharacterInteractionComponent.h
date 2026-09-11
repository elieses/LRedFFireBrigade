// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "Common/CISTypes.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "CISCharacterInteractionComponent.generated.h"

class UPrimitiveComponent;
class UCISInteractionObjectComponent;
class AActor;
class ACharacter;
class UCISCharacterInteractionComponent;
class UCISInteractionDataObject;
class UCISDetermineUseObjectTest;

USTRUCT(BlueprintType)
struct FCISCharacterInteractionSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0.f, UIMin = 0.f), Category = "Settings")
	float InteractionAngle = 45.f;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0.f, UIMin = 0.f), Category = "Settings")
	float TimeToUseObject = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	TArray<FCISPrimitiveComponentsOnActor> TriggerComponents;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Settings")
	TArray<UCISDetermineUseObjectTest*> TestsToDetermineObjectForUse;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 1.f, UIMin = 1.f), Category = "Settings")
	float TimeToClearInvalidObjectsData = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = "Settings", meta = (ClampMin = 0.1f, UIMin = 0.1f))
	float UpdateCurrentUseObjectInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	TSubclassOf<UCISInteractionDataObject> DataObjectClass;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCISOnUseObjectDynamic, UCISInteractionObjectComponent*, InteractionObjectComponent);
DECLARE_MULTICAST_DELEGATE_TwoParams(FCISOnUseObject, TWeakObjectPtr<UCISInteractionObjectComponent> /*InteractionObjectComponent*/, TWeakObjectPtr<UCISInteractionDataObject> /*InteractionDataObject*/);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHARACTERINTERACTIONSYSTEM_API UCISCharacterInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCISCharacterInteractionComponent();

	UFUNCTION(BlueprintCallable, Category = "CIS|CharacterInteractionComponent")
	virtual void InitInteractionTriggers(TArray<UPrimitiveComponent*> TriggersForOverlap);

	UFUNCTION(BlueprintCallable, Category = "CIS|CharacterInteractionComponent")
	virtual void TryUseObject();

	UFUNCTION(BlueprintCallable, Category = "CIS|CharacterInteractionComponent")
	virtual bool TryInterruptObjectUsing(ECISObjectInterruptionType InterruptionType);

	UFUNCTION(BlueprintPure, Category = "CIS|CharacterInteractionComponent")
	virtual bool IsCanUseObject(UCISInteractionObjectComponent* ObjectToCheck) const;

	const FCISCharacterInteractionSettings& GetCharacterInteractionSettings() const;

	virtual void RemoveInteractionObjectData(UCISInteractionObjectComponent* FromComponent);

	UFUNCTION(BlueprintPure, Category = "CIS|CharacterInteractionComponent")
	int32 GetCountOfUsableObjects() const;

	UFUNCTION(BlueprintPure, Category = "CIS|CharacterInteractionComponent")
	bool IsCharacterUseAnyObject() const;

	void SetOwnerOfActor(AActor* Actor, bool bClear = false);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetOwnerOfActor(AActor* Actor, bool bClear = false);

	UFUNCTION(BlueprintPure, Category = "CIS|CharacterInteractionComponent")
	UCISInteractionObjectComponent* GetCurrentUsingObject() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "CIS|CharacterInteractionComponent")
	virtual UCISInteractionObjectComponent* GetUsableInteractObject() const;

	UFUNCTION(BlueprintPure, Category = "CIS|CharacterInteractionComponent")
	float GetDelayedObjectProgress() const;

protected:
	virtual float CalculateScoreForObject(UCISInteractionObjectComponent* InteractionObject) const;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void InitializeComponent() override;

	UFUNCTION(BlueprintCallable, Category = "CIS|CharacterInteractionComponent")
	virtual void InitCharacterInteractionComponent();

	virtual void OnObjectEndUse(TWeakObjectPtr<UCISInteractionObjectComponent> InteractionObjectComponent, TWeakObjectPtr<UCISInteractionDataObject> InteractionDataObject);

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool IsWithDebug() const;

	virtual void Debug(float DeltaTime);

	virtual void InteractionObjectStartOverlap(UCISInteractionObjectComponent* Object);
	virtual void InteractionObjectEndOverlap(UCISInteractionObjectComponent* Object);

	virtual TSubclassOf<UCISInteractionDataObject> GetInteractionDataObjectClass(UCISInteractionObjectComponent* ForComponent) const;

	UCISInteractionDataObject* GetInteractionObjectData(UCISInteractionObjectComponent* FromComponent);

	void CheckDelayedObjectTime(float DeltaTime);

	UFUNCTION()
	virtual void OnObjectDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	virtual void OnCharacterDestroyed(AActor* DestroyedCharacter);

	void StartDelayedUseObject(UCISInteractionObjectComponent* Object, UCISInteractionDataObject* ObjectData);

	virtual void ClearDelayedObjectData();

	UFUNCTION(BlueprintPure, Category = "CIS|CharacterInteractionComponent")
	bool IsHasDelayedObjectData() const;

	void SetCurrentUseObject(UCISInteractionObjectComponent* Object);

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerSetCurrentUseObject(UCISInteractionObjectComponent* Object);

	virtual void UpdateBestObjectToUse();

private:
	virtual void ClearInvalidObjectsData();

public:
	UPROPERTY(BlueprintAssignable, Category = "CIS|Delegates")
	FCISOnUseObjectDynamic OnDelayedObjectDataSet;

	UPROPERTY(BlueprintAssignable, Category = "CIS|Delegates")
	FCISOnUseObjectDynamic OnClearDelayedObjectData;

	UPROPERTY(BlueprintAssignable, Category = "CIS|Delegates")
	FCISOnUseObjectDynamic OnStartUseObjectDelegate;

	UPROPERTY(BlueprintAssignable, Category = "CIS|Delegates")
	FCISOnUseObjectDynamic OnEndUseObjectDelegate;

	UPROPERTY(BlueprintAssignable, Category = "CIS|Delegates")
	FCISOnUseObjectDynamic OnBestObjectForUseChangedDelegate;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	FCISCharacterInteractionSettings CharacterInteractionSettings;

	UPROPERTY()
	TSet<UPrimitiveComponent*> Triggers;

	UPROPERTY()
	ACharacter* Character;

	UPROPERTY(EditAnywhere, Category = "Settings|Debug")
	bool bWithDebug = false;

	UPROPERTY()
	TMap<TWeakObjectPtr<UCISInteractionObjectComponent>, TObjectPtr<UCISInteractionDataObject>> OverlappedObjectsData;

	UPROPERTY(Replicated)
	UCISInteractionObjectComponent* CurrentUsingObject;

	bool bCharacterUseSomeObject = false;

	UPROPERTY(BlueprintReadOnly, Category = "CIS|CharacterInteractionComponent")
	float CurrentDelayedObjectTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "CIS|CharacterInteractionComponent")
	float DelayedUsingTime = 1.f;

	struct FCISDelayedUseObjectData
	{
	public:
		FCISDelayedUseObjectData(UCISInteractionObjectComponent* InInteractionObjectComponent, UCISInteractionDataObject* InInteractionObjectData);
		FCISDelayedUseObjectData() = default;

		TWeakObjectPtr<UCISInteractionObjectComponent> InteractionObjectComponent;
		TWeakObjectPtr<UCISInteractionDataObject> InteractionObjectData;
	};

	TOptional<FCISDelayedUseObjectData> DelayedUseObjectData;

	//Possible to use object
	UPROPERTY(BlueprintReadOnly, Category = "CIS|CharacterInteractionComponent")
	TWeakObjectPtr<UCISInteractionObjectComponent> BestObjectToUse;

	bool bBestObjectNullCheck = false;

private:
	friend class UCISInteractionDataObject;

	int32 CountOfUsableObjects = 0;

	FTimerHandle InvalidObjectDataCleanTimerHandle;

	FTimerHandle UpdateCurrentUsableObjectTimerHandle;
};
