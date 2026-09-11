// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "Common/CISTypes.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "CISInteractionObjectComponent.generated.h"

class UCISCharacterInteractionComponent;
class ACharacter;
class UPrimitiveComponent;
class UCISInteractionDataObject;
class UCISInteractionObjectCondition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCISOnObjectStateChanged, FGameplayTag, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCISSimpleDelegate, ACharacter*, Character, UCISCharacterInteractionComponent*, CharacterInteractionComponent, UCISInteractionObjectComponent*, InteractionObjectComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FCISCharacterStateDelegate, ACharacter*, Character, UCISCharacterInteractionComponent*, CharacterInteractionComponent, UCISInteractionObjectComponent*, InteractionObjectComponent, bool, bValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FCISUseObjectEndDelegate, ACharacter*, Character, UCISCharacterInteractionComponent*, CharacterInteractionComponent, UCISInteractionObjectComponent*, InteractionObjectComponent, bool, bInterrupted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCISUseObjectStartDelegate, ACharacter*, Character, UCISCharacterInteractionComponent*, CharacterInteractionComponent, UCISInteractionObjectComponent*, InteractionObjectComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FCISUseObjectActionStartDelegate, UCISInteractionObjectAction*, Action, ACharacter*, Character, UCISCharacterInteractionComponent*, CharacterInteractionComponent, UCISInteractionObjectComponent*, InteractionObjectComponent);

USTRUCT(BlueprintType)
struct FCISInteractionObjectSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	TArray<FCISInteractionObjectSide> InteractionSides;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0.f, UIMin = 0.f), Category = "Settings")
	float InteractionDistance = 300.f;

	//To detect angle between character forward vector and intaraction object
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	FCISSceneComponentsOnActor AngleDetectionComponent;

	UPROPERTY(meta = (InlineEditConditionToggle))
	bool bOverrideCharacterInteractionAngle = false;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = bOverrideCharacterInteractionAngle, ClampMin = 0.f, UIMin = 0.f), Category = "Settings")
	float OverrideCharacterInteractionAngle = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	ECISObjectInterruptionType ObjectInterruptionType = ECISObjectInterruptionType::None;

	UPROPERTY(meta = (InlineEditConditionToggle))
	bool bOverrideObjectUseTime = false;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = bOverrideObjectUseTime, ClampMin = 0.f, UIMin = 0.f), Category = "Settings")
	float OverrideObjectUseTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Settings")
	TArray<UCISInteractionObjectCondition*> VisibleConditions;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Settings")
	TArray<UCISInteractionObjectCondition*> UsableConditions;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	bool bOverrideDataObjectClass = false;

	UPROPERTY(EditDefaultsOnly, Category = "Settings", meta = (EditCondition = "bOverrideDataObjectClass", EditConditionHides))
	TSubclassOf<UCISInteractionDataObject> DataObjectClass;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0, UIMin = 0, ClampMax = 10, UIMax = 10), Category = "Settings")
	int32 ObjectPriority = 0;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), editinlinenew)
class CHARACTERINTERACTIONSYSTEM_API UCISInteractionObjectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCISInteractionObjectComponent();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void OnRegister() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	UFUNCTION(BlueprintCallable, Category = "CIS|InteractionObjectComponent")
	void SetObjectIsUsing(bool bValue);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetObjectIsUsing(bool bValue);

	bool IsObjectUsing() const;

	UFUNCTION(BlueprintPure, Category = "CIS|InteractionObjectComponent")
	const FCISInteractionObjectSettings& GetSettings() const;

	UFUNCTION(BlueprintPure, Category = "CIS|InteractionObjectComponent")
	const FGameplayTag& GetObjectState() const;

	UFUNCTION(BlueprintCallable, Category = "CIS|InteractionObjectComponent")
	void SetObjectState(const FGameplayTag& NewState);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetObjectState(const FGameplayTag& NewState);

	UCISInteractionObjectAction* GetObjectActionForSide(const FCISInteractionObjectSide& Side) const;

	UFUNCTION(BlueprintCallable, Category = "CIS|InteractionObjectComponent")
	virtual void SetUsable(bool bValue);

	UFUNCTION(BlueprintPure, Category = "CIS|InteractionObjectComponent")
	bool IsObjectUsable() const { return bUsable; }

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool IsWithDebugInGame() const;

	void DrawDebug();

	UFUNCTION()
	void OnRep_ObjectState();

	bool IsInEditorWorld() const;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetUsable(bool bValue);

public:
	UPROPERTY(BlueprintAssignable, Category = "CIS|Delegates")
	FCISOnObjectStateChanged OnObjectStateChangedDelegate;

protected:
	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Settings")
	FCISInteractionObjectSettings InteractionSettings;

	UPROPERTY(ReplicatedUsing = OnRep_ObjectState)
	FGameplayTag ObjectState;

	UPROPERTY(EditAnywhere, Category = "Settings")
	FGameplayTag InitialObjectState;

	UPROPERTY(EditAnywhere, Category = "Settings|Debug")
	bool bWithDebugInGame = false;

	UPROPERTY(EditAnywhere, Category = "Settings|Debug")
	bool bWithDebugInEditor = true;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0, UIMin = 0, ClampMax = 1, UIMax = 1), Category = "Settings|Debug")
	int32 DebugLinesDephPriority = 0;

	UPROPERTY(Replicated)
	bool bObjectIsUsing = false;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bUsable = true;
};
