// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/IControllable.h"
#include "MainCarmeraPawn.generated.h"

class UFloatingPawnMovement;
class UCameraComponent;
class USpringArmComponent;
struct FInputActionValue;
UCLASS()
class LREDFFIREBRIGADE_API AMainCarmeraPawn : public APawn,public IControllable
{
	GENERATED_BODY()

public:

	AMainCarmeraPawn();
	virtual void Tick(float DeltaTime) override;
	
	
protected:
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
	UFloatingPawnMovement* FloatingMovement;
	
	virtual void Move(const FInputActionValue& Value) override;
	virtual void Zoom(const FInputActionValue& Value) override;
	virtual void Rotate(const FInputActionValue& Value) override;
	
	// 缩放速度 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomSpeed = 50.f;
	// 最小缩放距离 (最近看多近) 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MinZoom = 300.f;
	// 最大缩放距离 (最远看多远) 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxZoom = 1500.f;
	
	
	
	//旋转速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RotationSpeed = 100.0f;

	// 地图边界（世界坐标，单位cm）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Limits")
	float MinX = -5000.f;
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Limits")
	float MaxX = 5000.f;
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Limits")
	float MinY = -5000.f;
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Limits")
	float MaxY = 5000.f;
	// 边缘触发的像素宽度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|EdgeScroll")
	float EdgeMargin = 50.f;
 
	// 边缘滚动的速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|EdgeScroll")
	float EdgeScrollSpeed = 800.f;
	// 是否开启边缘滚动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|EdgeScroll")
	bool bEnableEdgeScroll = true;
private:
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;
	float TargetArmYaw;
	//边缘检测
	void HandleEdgeScroll(float DeltaTime);
	
};
