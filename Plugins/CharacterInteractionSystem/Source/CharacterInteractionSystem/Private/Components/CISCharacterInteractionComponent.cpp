// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#include "Components/CISCharacterInteractionComponent.h"

#include "Components/CISInteractionDataObject.h"
#include "Components/CISInteractionObjectComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "DetermineUseObjectTests/CISDetermineUseObjectTest.h"
#include "DetermineUseObjectTests/CISUseObjectTest_ObjectPriority.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

UCISCharacterInteractionComponent::UCISCharacterInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	bTickInEditor = true;
	SetIsReplicatedByDefault(true);

	CharacterInteractionSettings.TestsToDetermineObjectForUse.AddUninitialized();
	CharacterInteractionSettings.TestsToDetermineObjectForUse[0] = CreateDefaultSubobject<UCISUseObjectTest_ObjectPriority>(TEXT("UCISUseObjectTest_ObjectPriority"));

	CharacterInteractionSettings.DataObjectClass = UCISInteractionDataObject::StaticClass();
}

void UCISCharacterInteractionComponent::InitInteractionTriggers(TArray<UPrimitiveComponent*> TriggersForOverlap)
{
	for (const auto currentTrigger : Triggers)
	{
		if (currentTrigger != nullptr)
		{
			currentTrigger->OnComponentBeginOverlap.RemoveDynamic(this, &UCISCharacterInteractionComponent::OnTriggerBeginOverlap);
			currentTrigger->OnComponentEndOverlap.RemoveDynamic(this, &UCISCharacterInteractionComponent::OnTriggerEndOverlap);
		}
	}

	Triggers.Empty();

	for (auto trigger : TriggersForOverlap)
	{
		bool bAlreadySet = false;
		if (trigger != nullptr)
		{
			Triggers.Add(trigger, &bAlreadySet);
		}

		if (!bAlreadySet)
		{
			trigger->OnComponentBeginOverlap.AddDynamic(this, &UCISCharacterInteractionComponent::OnTriggerBeginOverlap);
			trigger->OnComponentEndOverlap.AddDynamic(this, &UCISCharacterInteractionComponent::OnTriggerEndOverlap);

			TArray<AActor*> overlappingActors;
			trigger->GetOverlappingActors(overlappingActors);

			for (auto actor : overlappingActors)
			{
				TArray<UCISInteractionObjectComponent*> interactionComponents;
				actor->GetComponents<UCISInteractionObjectComponent>(interactionComponents);

				if (interactionComponents.Num() != 0)
				{
					InteractionObjectStartOverlap(interactionComponents[0]);
				}
			}
		}
	}
}

void UCISCharacterInteractionComponent::TryUseObject()
{
	if (!bCharacterUseSomeObject && !DelayedUseObjectData.IsSet())
	{
		if (const auto objectForInteraction = GetUsableInteractObject())
		{
			StartDelayedUseObject(objectForInteraction, OverlappedObjectsData.Find(objectForInteraction)->Get());
		}
	}
}

bool UCISCharacterInteractionComponent::TryInterruptObjectUsing(ECISObjectInterruptionType InterruptionType)
{
	auto returnValue = false;

	if (InterruptionType != ECISObjectInterruptionType::None && bCharacterUseSomeObject && GetCurrentUsingObject() != nullptr && GetCurrentUsingObject()->GetSettings().ObjectInterruptionType == InterruptionType)
	{
		if (const auto data = GetInteractionObjectData(GetCurrentUsingObject()))
		{
			data->InterruptUsing();
			returnValue = true;
		}
	}

	ClearDelayedObjectData();

	return returnValue;
}

bool UCISCharacterInteractionComponent::IsCanUseObject(UCISInteractionObjectComponent* ObjectToCheck) const
{
	return OverlappedObjectsData.Contains(ObjectToCheck) && OverlappedObjectsData[ObjectToCheck]->IsCharacterCanUseObject();
}

const FCISCharacterInteractionSettings& UCISCharacterInteractionComponent::GetCharacterInteractionSettings() const
{
	return CharacterInteractionSettings;
}

void UCISCharacterInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	InitCharacterInteractionComponent();
}

void UCISCharacterInteractionComponent::InitCharacterInteractionComponent()
{
	if (!Character->IsValidLowLevel() || !Character->IsLocallyControlled())
	{
		return;
	}

	TArray<UPrimitiveComponent*> triggers;
	TArray<UPrimitiveComponent*> componentsOnOwner;
	GetOwner()->GetComponents<UPrimitiveComponent>(componentsOnOwner);

	for (auto component : componentsOnOwner)
	{
		if (CharacterInteractionSettings.TriggerComponents.ContainsByPredicate(
		        [&](const FCISPrimitiveComponentsOnActor& iteratedData) {
			        return iteratedData.SelectedPrimitiveComponentName == component->GetName();
		        }))
		{
			triggers.Add(component);
		}
	}

	InitInteractionTriggers(triggers);

	GetWorld()->GetTimerManager().SetTimer(UpdateCurrentUsableObjectTimerHandle, FTimerDelegate::CreateUObject(this, &UCISCharacterInteractionComponent::UpdateBestObjectToUse), GetCharacterInteractionSettings().UpdateCurrentUseObjectInterval, true);
	GetWorld()->GetTimerManager().SetTimer(InvalidObjectDataCleanTimerHandle, FTimerDelegate::CreateUObject(this, &UCISCharacterInteractionComponent::ClearInvalidObjectsData), GetCharacterInteractionSettings().TimeToClearInvalidObjectsData, true);
}

void UCISCharacterInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if WITH_EDITOR
	Debug(DeltaTime);
#endif

	for (auto data : OverlappedObjectsData)
	{
		if (data.Key.IsValid())
		{
			data.Value.Get()->Tick(DeltaTime);
		}
	}

	CheckDelayedObjectTime(DeltaTime);
}

void UCISCharacterInteractionComponent::InitializeComponent()
{
	Super::InitializeComponent();

	Character = Cast<ACharacter>(GetOwner());
	Character->OnDestroyed.AddDynamic(this, &UCISCharacterInteractionComponent::OnCharacterDestroyed);
}

void UCISCharacterInteractionComponent::OnObjectEndUse(TWeakObjectPtr<UCISInteractionObjectComponent> InteractionObjectComponent, TWeakObjectPtr<UCISInteractionDataObject> InteractionDataObject)
{
	bCharacterUseSomeObject = false;
	SetCurrentUseObject(nullptr);

	if (InteractionObjectComponent.IsValid() && InteractionDataObject.IsValid())
	{
		InteractionDataObject.Get()->OnEndUseObjectDelegate.RemoveAll(this);
		OnEndUseObjectDelegate.Broadcast(InteractionObjectComponent.Get());
	}
}

void UCISCharacterInteractionComponent::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TArray<UCISInteractionObjectComponent*> interactionComponents;
	OtherActor->GetComponents<UCISInteractionObjectComponent>(interactionComponents);

	if (interactionComponents.Num() != 0)
	{
		InteractionObjectStartOverlap(interactionComponents[0]);
	}
}

void UCISCharacterInteractionComponent::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	TArray<UCISInteractionObjectComponent*> interactionComponents;
	OtherActor->GetComponents<UCISInteractionObjectComponent>(interactionComponents);

	if (interactionComponents.Num() != 0)
	{
		InteractionObjectEndOverlap(interactionComponents[0]);
	}
}

bool UCISCharacterInteractionComponent::IsWithDebug() const
{
#if UE_BUILD_SHIPPING
	return false;
#else
	return bWithDebug;
#endif
}

void UCISCharacterInteractionComponent::Debug(float DeltaTime)
{
	if (IsWithDebug())
	{
		::DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + FRotator(0.f, -CharacterInteractionSettings.InteractionAngle * 0.5f, 0.f).RotateVector(GetOwner()->GetActorForwardVector()) * 300.f, FColor::Green, false, -1, 1, 2.f);
		::DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + FRotator(0.f, CharacterInteractionSettings.InteractionAngle * 0.5f, 0.f).RotateVector(GetOwner()->GetActorForwardVector()) * 300.f, FColor::Green, false, -1, 1, 2.f);

		for (auto object : OverlappedObjectsData)
		{
			if (!object.Key.IsValid() || !IsValid(object.Value))
			{
				continue;
			}

			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, FString::Printf(TEXT("Overlapped Interact Object: %s"), *object.Key->GetOuter()->GetName()));
		}
	}
}

void UCISCharacterInteractionComponent::InteractionObjectStartOverlap(UCISInteractionObjectComponent* Object)
{
	if (const auto data = GetInteractionObjectData(Object))
	{
		if (!data->IsOverlapped())
		{
			data->Overlapped(true);
		}
	}
}

void UCISCharacterInteractionComponent::InteractionObjectEndOverlap(UCISInteractionObjectComponent* Object)
{
	if (const auto data = GetInteractionObjectData(Object))
	{
		if (data->IsOverlapped())
		{
			data->Overlapped(false);
		}
	}
}

TSubclassOf<UCISInteractionDataObject> UCISCharacterInteractionComponent::GetInteractionDataObjectClass(UCISInteractionObjectComponent* ForComponent) const
{
	if (ForComponent != nullptr && ForComponent->GetSettings().bOverrideDataObjectClass && ForComponent->GetSettings().DataObjectClass != nullptr)
	{
		return ForComponent->GetSettings().DataObjectClass;
	}

	return CharacterInteractionSettings.DataObjectClass != nullptr ? CharacterInteractionSettings.DataObjectClass : TSubclassOf<UCISInteractionDataObject>(UCISInteractionDataObject::StaticClass());
}

UCISInteractionDataObject* UCISCharacterInteractionComponent::GetInteractionObjectData(UCISInteractionObjectComponent* FromComponent)
{
	if (!FromComponent->GetOwner()->IsValidLowLevel() || FromComponent->GetOwner()->IsActorBeingDestroyed() || !Character->IsValidLowLevel() || Character->IsActorBeingDestroyed())
	{
		return nullptr;
	}

	UCISInteractionDataObject* returnData = nullptr;

	if (!OverlappedObjectsData.Contains(FromComponent))
	{
		const auto newData = NewObject<UCISInteractionDataObject>(this, GetInteractionDataObjectClass(FromComponent));

		check(newData);

		OverlappedObjectsData.Add(FromComponent, TObjectPtr<UCISInteractionDataObject>(newData));
		newData->Init(Character, this, FromComponent);
		returnData = newData;
		FromComponent->GetOwner()->OnDestroyed.AddDynamic(this, &UCISCharacterInteractionComponent::OnObjectDestroyed);
	}
	else
	{
		returnData = OverlappedObjectsData.Find(FromComponent)->Get();
	}

	return returnData;
}

void UCISCharacterInteractionComponent::CheckDelayedObjectTime(float DeltaTime)
{
	if (DelayedUseObjectData.IsSet())
	{
		if (DelayedUseObjectData->InteractionObjectComponent.IsValid() && DelayedUseObjectData->InteractionObjectData.IsValid() && OverlappedObjectsData.Contains(DelayedUseObjectData->InteractionObjectComponent) && DelayedUseObjectData.GetValue().InteractionObjectData->IsCharacterCanUseObject())
		{
			CurrentDelayedObjectTime += DeltaTime;

			if (GetDelayedObjectProgress() >= 1.f)
			{
				bCharacterUseSomeObject = true;
				SetCurrentUseObject(DelayedUseObjectData->InteractionObjectComponent.Get());
				DelayedUseObjectData->InteractionObjectData->OnEndUseObjectDelegate.AddUObject(this, &UCISCharacterInteractionComponent::OnObjectEndUse);
				OnStartUseObjectDelegate.Broadcast(DelayedUseObjectData->InteractionObjectComponent.Get());
				DelayedUseObjectData->InteractionObjectData->UseObject();
				ClearDelayedObjectData();
			}
		}
		else
		{
			ClearDelayedObjectData();
		}
	}
}

void UCISCharacterInteractionComponent::OnObjectDestroyed(AActor* DestroyedActor)
{
	if (DelayedUseObjectData.IsSet() && DelayedUseObjectData->InteractionObjectComponent->GetOwner() == DestroyedActor)
	{
		ClearDelayedObjectData();
	}

	for (auto& data : OverlappedObjectsData)
	{
		if (data.Key.Get()->GetOwner() == DestroyedActor)
		{
			RemoveInteractionObjectData(data.Key.Get());
			break;
		}
	}
}

void UCISCharacterInteractionComponent::OnCharacterDestroyed(AActor* DestroyedCharacter)
{
	TArray<TWeakObjectPtr<UCISInteractionObjectComponent>> overlappedComponents;
	OverlappedObjectsData.GenerateKeyArray(overlappedComponents);

	GetWorld()->GetTimerManager().ClearTimer(InvalidObjectDataCleanTimerHandle);

	for (auto comp : overlappedComponents)
	{
		if (comp.IsValid())
		{
			RemoveInteractionObjectData(comp.Get());
		}
	}
}

void UCISCharacterInteractionComponent::StartDelayedUseObject(UCISInteractionObjectComponent* Object, UCISInteractionDataObject* ObjectData)
{
	if (!bCharacterUseSomeObject && !DelayedUseObjectData.IsSet())
	{
		DelayedUsingTime = Object->GetSettings().bOverrideObjectUseTime ? Object->GetSettings().OverrideObjectUseTime : GetCharacterInteractionSettings().TimeToUseObject;
		DelayedUseObjectData = FCISDelayedUseObjectData(Object, ObjectData);
		OnDelayedObjectDataSet.Broadcast(Object);
	}
}

void UCISCharacterInteractionComponent::ClearDelayedObjectData()
{
	if (DelayedUseObjectData.IsSet())
	{
		CurrentDelayedObjectTime = 0.f;

		OnClearDelayedObjectData.Broadcast(DelayedUseObjectData->InteractionObjectComponent.Get());

		DelayedUseObjectData.Reset();
	}
}

bool UCISCharacterInteractionComponent::IsHasDelayedObjectData() const
{
	return DelayedUseObjectData.IsSet();
}

void UCISCharacterInteractionComponent::SetCurrentUseObject(UCISInteractionObjectComponent* Object)
{
	if (Character->GetLocalRole() < ROLE_Authority)
	{
		ServerSetCurrentUseObject(Object);
	}

	CurrentUsingObject = Object;
}

void UCISCharacterInteractionComponent::UpdateBestObjectToUse()
{
	const auto bestObject = GetUsableInteractObject();

	if (BestObjectToUse != bestObject || (!BestObjectToUse.IsValid() && !bBestObjectNullCheck))
	{
		bBestObjectNullCheck = bestObject == nullptr;

		BestObjectToUse = bestObject;
		OnBestObjectForUseChangedDelegate.Broadcast(BestObjectToUse.Get());
	}
}

void UCISCharacterInteractionComponent::ClearInvalidObjectsData()
{
	TArray<TObjectPtr<UCISInteractionDataObject>> overlappedData;

	OverlappedObjectsData.GenerateValueArray(overlappedData);

	for (const auto& data : overlappedData)
	{
		if (!data->IsOverlapped())
		{
			RemoveInteractionObjectData(data->GetInteractionObjectComponent());
		}
	}
}

void UCISCharacterInteractionComponent::ServerSetCurrentUseObject_Implementation(UCISInteractionObjectComponent* Object)
{
	SetCurrentUseObject(Object);
}

bool UCISCharacterInteractionComponent::ServerSetCurrentUseObject_Validate(UCISInteractionObjectComponent* Object)
{
	return true;
}

void UCISCharacterInteractionComponent::RemoveInteractionObjectData(UCISInteractionObjectComponent* FromComponent)
{
	if (OverlappedObjectsData.Contains(FromComponent))
	{
		const auto data = *OverlappedObjectsData.Find(FromComponent);

		if (DelayedUseObjectData.IsSet() && data.Get() == DelayedUseObjectData->InteractionObjectData.Get())
		{
			ClearDelayedObjectData();
		}

		FromComponent->GetOwner()->OnDestroyed.RemoveAll(this);

		data->OnDataRemovedFromCharacterComponent();
		OverlappedObjectsData.Remove(FromComponent);
	}
}

int32 UCISCharacterInteractionComponent::GetCountOfUsableObjects() const
{
	return CountOfUsableObjects;
}

bool UCISCharacterInteractionComponent::IsCharacterUseAnyObject() const
{
	return bCharacterUseSomeObject;
}

void UCISCharacterInteractionComponent::ServerSetOwnerOfActor_Implementation(AActor* Actor, bool bClear /*= false*/)
{
	if (Actor != nullptr)
	{
		Actor->SetOwner(bClear ? nullptr : Character->GetController());
	}
}

bool UCISCharacterInteractionComponent::ServerSetOwnerOfActor_Validate(AActor* Actor, bool bClear /*= false*/)
{
	return true;
}

void UCISCharacterInteractionComponent::SetOwnerOfActor(AActor* Actor, bool bClear /*= false*/)
{
	if (Actor->IsValidLowLevel() && !Actor->IsActorBeingDestroyed())
	{
		Actor->SetOwner(bClear ? nullptr : Character->GetController());
		ServerSetOwnerOfActor(Actor);
	}
}

UCISInteractionObjectComponent* UCISCharacterInteractionComponent::GetCurrentUsingObject() const
{
	return CurrentUsingObject;
}

void UCISCharacterInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UCISCharacterInteractionComponent, CurrentUsingObject, COND_SkipOwner);
}

UCISInteractionObjectComponent* UCISCharacterInteractionComponent::GetUsableInteractObject() const
{
	struct FObjectScorePair
	{
		UCISInteractionObjectComponent* Object;
		float Score;

		FObjectScorePair(UCISInteractionObjectComponent* InObject, float InScore)
		    : Object(InObject), Score(InScore)
		{
		}
	};

	TArray<FObjectScorePair> validObjectsWithScores;
	float minScore = FLT_MAX; // Initialize minScore to the highest possible float value
	float maxScore = FLT_MIN; // Initialize maxScore to the lowest possible float value

	// Collect all valid objects and their scores, and find the minimum and maximum score
	for (const auto& objectDataPair : OverlappedObjectsData)
	{
		UCISInteractionObjectComponent* interactionObject = objectDataPair.Key.Get();
		// Check if object is valid and can be used by the character
		if (interactionObject->IsValidLowLevel() && IsValid(objectDataPair.Value) && objectDataPair.Value->IsCharacterCanUseObject())
		{
			float currentScore = CalculateScoreForObject(interactionObject);
			minScore = FMath::Min(minScore, currentScore); // Update MinScore with the lowest score found
			maxScore = FMath::Max(maxScore, currentScore); // Update MaxScore with the highest score found
			validObjectsWithScores.Emplace(interactionObject, currentScore); // Store object and its score
		}
	}

	// If no valid objects are found, return nullptr
	if (validObjectsWithScores.Num() == 0)
	{
		return nullptr;
	}

	// Normalize scores and find the object with the highest normalized score
	UCISInteractionObjectComponent* bestObject = nullptr;
	float bestNormalizedScore = FLT_MIN; // Initialize to lowest float value to find the maximum normalized score

	// Check if all scores are equal (to avoid division by zero in normalization)
	if (FMath::IsNearlyEqual(minScore, maxScore))
	{
		// Since all scores are the same, we can return the first valid object
		bestObject = validObjectsWithScores[0].Object;
	}
	else
	{
		for (const FObjectScorePair& ObjectScorePair : validObjectsWithScores)
		{
			// Normalize the score based on the min and max scores found
			const float normalizedScore = (ObjectScorePair.Score - minScore) / (maxScore - minScore);
			// If the normalized score is the best found so far, update the best object and score
			if (normalizedScore > bestNormalizedScore)
			{
				bestNormalizedScore = normalizedScore;
				bestObject = ObjectScorePair.Object;
			}
		}
	}

	// Return the object with the highest normalized score, or nullptr if none are found
	return bestObject;
}

float UCISCharacterInteractionComponent::GetDelayedObjectProgress() const
{
	if (DelayedUseObjectData.IsSet())
	{
		return CurrentDelayedObjectTime / DelayedUsingTime;
	}

	return 0.f;
}

float UCISCharacterInteractionComponent::CalculateScoreForObject(UCISInteractionObjectComponent* InteractionObject) const
{
	float score = 0.f;

	if (OverlappedObjectsData.Contains(InteractionObject))
	{
		for (const auto& test : GetCharacterInteractionSettings().TestsToDetermineObjectForUse)
		{
			if (test != nullptr)
			{
				score += test->CalculateObjectScoreInternal(Character, this, InteractionObject, OverlappedObjectsData[InteractionObject]->GetCurrentSide(), score);
			}
		}
	}

	return score;
}

UCISCharacterInteractionComponent::FCISDelayedUseObjectData::FCISDelayedUseObjectData(UCISInteractionObjectComponent* InInteractionObjectComponent, UCISInteractionDataObject* InInteractionObjectData)
    : InteractionObjectComponent(InInteractionObjectComponent), InteractionObjectData(InInteractionObjectData)
{
}
