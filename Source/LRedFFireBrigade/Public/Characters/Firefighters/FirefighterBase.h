// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/ISelectable.h"
#include "Interfaces/IMoveable.h"
#include "FirefighterBase.generated.h"

class UStaticMeshComponent;
class USphereComponent;
struct FHitResult;

UCLASS()
class LREDFFIREBRIGADE_API AFirefighterBase : public ACharacter, public ISelectable, public IMoveable
{
	GENERATED_BODY()

public:
	AFirefighterBase();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
protected:
	virtual void BeginPlay() override;

public:
	// 选中指示：脚下圆环
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
	UStaticMeshComponent* SelectionMesh;

	virtual void SetSelected(bool bSelected) override;
	virtual void MoveToLocation(const FVector& Destination) override;

	// 显示球体: 笼罩范围内的 IFogRevealable 物体会显示, 离开后隐藏
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FogReveal")
	USphereComponent* RevealSphere;
	// 显示范围 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogReveal")
	float RevealRange = 800.f;

protected:
	// 球体重叠回调
	UFUNCTION()
	void OnRevealSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnRevealSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	// 处理开局已处于球体内的物体 (重叠事件可能不触发)
	void ScanInitialReveals();
};
