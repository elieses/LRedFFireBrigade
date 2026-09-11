// Copyright 2023 Dmitriy Vergasov All Rights Reserved
#include "AnimNotifies/CISAnimNotifyStateAttachMesh.h"

#include "Animation/AnimSequenceBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

void UCISAnimNotifyStateAttachMesh::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (const auto ownedActor = MeshComp->GetOwner())
	{
		const auto newStaticMesh = NewObject<UStaticMeshComponent>(ownedActor);
		newStaticMesh->RegisterComponent();
		newStaticMesh->SetStaticMesh(MeshToAttach);
		newStaticMesh->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);

		SpawnedMesh = newStaticMesh;
	}
}
void UCISAnimNotifyStateAttachMesh::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (IsValid(SpawnedMesh))
	{
		SpawnedMesh->DestroyComponent();
	}
}