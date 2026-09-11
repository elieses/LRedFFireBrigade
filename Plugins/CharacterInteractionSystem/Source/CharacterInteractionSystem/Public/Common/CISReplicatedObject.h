// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "CISReplicatedObject.generated.h"

UCLASS(Abstract, editinlinenew, DefaultToInstanced)
class CHARACTERINTERACTIONSYSTEM_API UCISReplicatedObject : public UObject
{
	GENERATED_BODY()

protected:
	virtual bool IsSupportedForNetworking() const override { return bWithReplication; };
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;

	UPROPERTY(EditAnywhere, Category = "Replication")
	bool bWithReplication = true;

	UPROPERTY(Replicated)
	bool bReplicatedBool = false;
};
