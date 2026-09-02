// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MainCarmeraPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "InputMappingContext.h"

AMainCarmeraPawn::AMainCarmeraPawn()
{
 	PrimaryActorTick.bCanEverTick = true;
	//根组件
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	// 创建并附加弹簧臂（Spring Arm）
	SpringArm=CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(GetRootComponent());
	//长度
	SpringArm->TargetArmLength = 800.f;
	//旋转
	SpringArm->SetRelativeRotation(FRotator(-70.f, 0.f, 0.f));
	//摄像机延迟跟随效果
	SpringArm->bEnableCameraLag = true;
	//延迟速度
	SpringArm->CameraLagSpeed = 6.0f;
	//禁用碰撞检测
	SpringArm->bDoCollisionTest = false;

	//摄像机
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	FloatingMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));
	//移动组件要移动根组件
	FloatingMovement->UpdatedComponent = RootComponent;
	FloatingMovement->MaxSpeed = 1200.f;
	//加速度
	FloatingMovement->Acceleration = 4000.0f;
	//减速度
	FloatingMovement->Deceleration = 4000.0f;
}


void AMainCarmeraPawn::BeginPlay()
{
	Super::BeginPlay();
	if (SpringArm)
	{
		// 用弹簧臂当前的Yaw初始化
		TargetArmYaw = SpringArm->GetRelativeRotation().Yaw;
	}
}



void AMainCarmeraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//限制范围
	FVector Location = GetActorLocation();
	
	Location.X = FMath::Clamp(Location.X, MinX, MaxX);
	Location.Y = FMath::Clamp(Location.Y, MinY, MaxY);
	SetActorLocation(Location);

	if (bEnableEdgeScroll)
	{
		HandleEdgeScroll(DeltaTime);
	}
}

void AMainCarmeraPawn::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller)
	{
		const FRotator YawRotation(0, TargetArmYaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMainCarmeraPawn::Zoom(const FInputActionValue& Value)
{
	float ZoomDelta = Value.Get<float>();
	if (SpringArm && Controller)
	{
		//计算新的目标长度
		float NewTargetLength = SpringArm->TargetArmLength - (ZoomDelta * ZoomSpeed);
		//使用 FMath::Clamp 限制范围 
		SpringArm->TargetArmLength = FMath::Clamp(NewTargetLength, MinZoom, MaxZoom);
	}
}

void AMainCarmeraPawn::Rotate(const FInputActionValue& Value)
{
	if (Controller)
	{
		TargetArmYaw += 90.f;
		
		SpringArm->SetRelativeRotation(FRotator(SpringArm->GetRelativeRotation().Pitch, TargetArmYaw, 0.f));
	}
}

void AMainCarmeraPawn::HandleEdgeScroll(float DeltaTime)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
 
	float MouseX, MouseY;
	int32 ViewportSizeX, ViewportSizeY;
 
	// 1. 获取当前鼠标位置和视口大小
	if (PC->GetMousePosition(MouseX, MouseY))
	{
		PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
 
		FVector MoveDirection = FVector::ZeroVector;
 
		// 2. 检测边缘并确定移动方向
		// X 轴（左右边缘）
		if (MouseX <= EdgeMargin) 
		{
			MoveDirection.Y = -1.f; // 向左
		}
		else if (MouseX >= ViewportSizeX - EdgeMargin)
		{
			MoveDirection.Y = 1.f;  // 向右
		}
 
		// Y 轴（上下边缘）
		if (MouseY <= EdgeMargin)
		{
			MoveDirection.X = 1.f;  // 向前
		}
		else if (MouseY >= ViewportSizeY - EdgeMargin)
		{
			MoveDirection.X = -1.f; // 向后
		}
 
		//将移动方向转换到当前摄像机的 Yaw 空间
		if (!MoveDirection.IsNearlyZero())
		{
			// 使用我们之前优化的 CurrentArmYaw，确保移动方向随镜头旋转而改变
			const FRotator Rotation(0.f, TargetArmYaw, 0.f);
			const FVector Forward = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
			const FVector Right = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
 
			// 计算最终位移
			FVector DeltaLocation = (Forward * MoveDirection.X + Right * MoveDirection.Y).GetSafeNormal();
			DeltaLocation *= EdgeScrollSpeed * DeltaTime;
			// 应用位移
			AddActorWorldOffset(DeltaLocation, true);
		}
	}
}