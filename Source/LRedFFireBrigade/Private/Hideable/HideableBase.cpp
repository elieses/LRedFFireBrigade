// Fill out your copyright notice in the Description page of Project Settings.

#include "Hideable/HideableBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

// FogReveal = ECC_GameTraceChannel4, FireFighter = ECC_GameTraceChannel2 (DefaultEngine.ini)
namespace
{
	const ECollisionChannel FogRevealChannel = ECC_GameTraceChannel4;
	const ECollisionChannel FireFighterChannel = ECC_GameTraceChannel2;
}

AHideableBase::AHideableBase()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	FogCollider = CreateDefaultSubobject<USphereComponent>(TEXT("FogCollider"));
	FogCollider->SetupAttachment(Mesh);
	FogCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FogCollider->SetCollisionObjectType(FogRevealChannel);
	FogCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
	FogCollider->SetCollisionResponseToChannel(FireFighterChannel, ECR_Overlap);
	FogCollider->SetGenerateOverlapEvents(true);
	FogCollider->InitSphereRadius(60.f);
}

void AHideableBase::BeginPlay()
{
	Super::BeginPlay();
	SetActorHiddenInGame(bStartHidden && RevealSourceCount <= 0);
}

void AHideableBase::AddRevealSource()
{
	++RevealSourceCount;
	UpdateVisibility();
}

void AHideableBase::RemoveRevealSource()
{
	if (RevealSourceCount > 0)
	{
		--RevealSourceCount;
	}
	UpdateVisibility();
}

void AHideableBase::UpdateVisibility()
{
	SetActorHiddenInGame(RevealSourceCount <= 0);
}
