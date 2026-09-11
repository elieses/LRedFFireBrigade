// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#include "Common/CISReplicatedObject.h"

#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

void UCISReplicatedObject::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (auto BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(UCISReplicatedObject, bReplicatedBool);
}

bool UCISReplicatedObject::CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack)
{
	check(!HasAnyFlags(RF_ClassDefaultObject));
	check(GetOuter() != nullptr);

	AActor* Owner = CastChecked<AActor>(GetOuter()->GetOuter());

	bool bProcessed = false;

	FWorldContext* const Context = GEngine->GetWorldContextFromWorld(GetWorld());
	if (Context != nullptr)
	{
		for (FNamedNetDriver& Driver : Context->ActiveNetDrivers)
		{
			if (Driver.NetDriver != nullptr && Driver.NetDriver->ShouldReplicateFunction(Owner, Function))
			{
				Driver.NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);
				bProcessed = true;
			}
		}
	}

	return bProcessed;
}

int32 UCISReplicatedObject::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !IsSupportedForNetworking())
	{
		return FunctionCallspace::Local;
	}

	check(GetOuter() != nullptr);

	return GetOuter()->GetFunctionCallspace(Function, Stack);
}
