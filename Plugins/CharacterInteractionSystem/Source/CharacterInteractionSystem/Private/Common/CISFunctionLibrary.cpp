// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#include "Common/CISFunctionLibrary.h"

#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UActorComponent* UCISFunctionLibrary::AddComponentToActor(AActor* Owner, TSubclassOf<UActorComponent> ComponentClass)
{
	if (ComponentClass != nullptr && Owner != nullptr)
	{
		auto newComponent = NewObject<UActorComponent>(Owner, ComponentClass);
		newComponent->RegisterComponent();
		Owner->AddOwnedComponent(newComponent);
		return newComponent;
	}

	return nullptr;
}

UPrimitiveComponent* UCISFunctionLibrary::GetPrimitiveComponentWithName(AActor* Actor, const FCISPrimitiveComponentsOnActor& NameData, FVector& SocketLocation)
{
	if (Actor == nullptr)
	{
		return nullptr;
	}

	UPrimitiveComponent* returnData = nullptr;

	TArray<UPrimitiveComponent*> componentsOnOwner;
	Actor->GetComponents<UPrimitiveComponent>(componentsOnOwner);

	for (auto component : componentsOnOwner)
	{
		if (component->GetClass()->IsChildOf<UPrimitiveComponent>() && component->GetName() == NameData.SelectedPrimitiveComponentName)
		{
			returnData = Cast<UPrimitiveComponent>(component);
			SocketLocation = returnData->GetSocketLocation(NameData.SocketName);
			break;
		}
	}

	return returnData;
}

USceneComponent* UCISFunctionLibrary::GetSceneComponentWithName(AActor* Actor, const FCISSceneComponentsOnActor& NameData, FVector& SocketLocation)
{
	if (Actor == nullptr)
	{
		return nullptr;
	}

	USceneComponent* returnData = nullptr;

	TArray<USceneComponent*> componentsOnOwner;
	Actor->GetComponents<USceneComponent>(componentsOnOwner);

	for (auto component : componentsOnOwner)
	{
		if (component->GetClass()->IsChildOf<USceneComponent>() && component->GetName() == NameData.SelectedSceneComponentName)
		{
			returnData = Cast<USceneComponent>(component);
			SocketLocation = returnData->GetSocketLocation(NameData.SocketName);
			break;
		}
	}

	return returnData;
}
