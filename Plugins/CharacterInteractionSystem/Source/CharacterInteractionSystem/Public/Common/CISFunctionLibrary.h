// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "CISTypes.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "CISFunctionLibrary.generated.h"

class UActorComponent;
class AActor;
class UPrimitiveComponent;
class ACharacter;

/**
 * 
 */
UCLASS()
class CHARACTERINTERACTIONSYSTEM_API UCISFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "CIS|FunctionLibrary")
	static UActorComponent* AddComponentToActor(AActor* Owner, TSubclassOf<UActorComponent> ComponentClass);

	UFUNCTION(BlueprintPure, Category = "CIS|FunctionLibrary")
	static UPrimitiveComponent* GetPrimitiveComponentWithName(AActor* Actor, UPARAM(ref) const FCISPrimitiveComponentsOnActor& NameData, FVector& SocketLocation);

	UFUNCTION(BlueprintPure, Category = "CIS|FunctionLibrary")
	static USceneComponent* GetSceneComponentWithName(AActor* Actor, UPARAM(ref) const FCISSceneComponentsOnActor& NameData, FVector& SocketLocation);
};
