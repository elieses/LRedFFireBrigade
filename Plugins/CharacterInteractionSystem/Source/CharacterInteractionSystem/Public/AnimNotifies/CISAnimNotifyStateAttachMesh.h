// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "CoreMinimal.h"

#include "CISAnimNotifyStateAttachMesh.generated.h"

class UStaticMesh;
class USkeletalMeshComponent;
class UAnimSequenceBase;

/**
 * 
 */
UCLASS()
class CHARACTERINTERACTIONSYSTEM_API UCISAnimNotifyStateAttachMesh : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	UStaticMesh* MeshToAttach;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FName SocketName;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> SpawnedMesh;
};
