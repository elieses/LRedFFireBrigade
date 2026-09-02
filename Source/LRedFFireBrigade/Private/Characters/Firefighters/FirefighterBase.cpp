// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Firefighters/FirefighterBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "TimerManager.h"
#include "Interfaces/IFogRevealable.h"

// FireFighter = ECC_GameTraceChannel2, FogReveal = ECC_GameTraceChannel4 (DefaultEngine.ini)
namespace
{
	const ECollisionChannel FireFighterChannel = ECC_GameTraceChannel2;
	const ECollisionChannel FogRevealChannel = ECC_GameTraceChannel4;
}

AFirefighterBase::AFirefighterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 使用 AI 控制器，以便接收 MoveToLocation 导航移动命令
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 选中指示：脚下扁圆环（StaticMesh 留空，在编辑器里指定）
	SelectionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectionMesh"));
	SelectionMesh->SetupAttachment(RootComponent);
	SelectionMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SelectionMesh->SetHiddenInGame(true);
	SelectionMesh->SetRelativeLocation(FVector(0, 0, 10.f));
	SelectionMesh->SetRelativeScale3D(FVector(1, 1, 0.05f));

	// 显示球体：只与 FogReveal 通道重叠 (要隐藏的物体实现该通道)
	RevealSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RevealSphere"));
	RevealSphere->SetupAttachment(RootComponent);
	RevealSphere->SetSphereRadius(RevealRange);
	RevealSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RevealSphere->SetCollisionObjectType(FireFighterChannel);
	RevealSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	RevealSphere->SetCollisionResponseToChannel(FogRevealChannel, ECR_Overlap);
	RevealSphere->SetGenerateOverlapEvents(true);
}


void AFirefighterBase::BeginPlay()
{
	Super::BeginPlay();

	if (RevealSphere)
	{
		RevealSphere->SetSphereRadius(RevealRange);
		RevealSphere->OnComponentBeginOverlap.AddDynamic(this, &AFirefighterBase::OnRevealSphereOverlap);
		RevealSphere->OnComponentEndOverlap.AddDynamic(this, &AFirefighterBase::OnRevealSphereEndOverlap);
		// 先扫一次, 再等一帧补一次 (覆盖出生先后顺序导致的漏判)
		ScanInitialReveals();
		GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &AFirefighterBase::ScanInitialReveals));
	}
}


void AFirefighterBase::ScanInitialReveals()
{
	if (!RevealSphere)
	{
		return;
	}
	TArray<AActor*> Overlapping;
	RevealSphere->GetOverlappingActors(Overlapping);
	for (AActor* A : Overlapping)
	{
		if (IFogRevealable* Revealable = Cast<IFogRevealable>(A))
		{
			Revealable->AddRevealSource();
		}
	}
}

void AFirefighterBase::OnRevealSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IFogRevealable* Revealable = Cast<IFogRevealable>(OtherActor))
	{
		Revealable->AddRevealSource();
	}
}

void AFirefighterBase::OnRevealSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IFogRevealable* Revealable = Cast<IFogRevealable>(OtherActor))
	{
		Revealable->RemoveRevealSource();
	}
}


void AFirefighterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AFirefighterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


void AFirefighterBase::SetSelected(bool bSelected)
{
	if (SelectionMesh)
	{
		SelectionMesh->SetHiddenInGame(!bSelected);
	}
}


void AFirefighterBase::MoveToLocation(const FVector& Destination)
{
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->MoveToLocation(Destination);
	}
}

