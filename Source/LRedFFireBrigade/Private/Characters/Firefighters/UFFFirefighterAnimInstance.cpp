// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Firefighters/UFFFirefighterAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

void UFFFirefighterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	AFirefighterBase* FF = Cast<AFirefighterBase>(TryGetPawnOwner());
	if (!FF)
	{
		// 无持有者 → 复位默认
		CachedCharacterState = ECharacterState::ECS_UnEquipped;
		CachedMovementState = EFFMovementState::Walk;
		bCachedBusy = false;
		CachedGroundSpeed = 0.f;
		bCachedInAir = false;
		return;
	}

	CachedCharacterState = FF->GetCharacterState();
	CachedMovementState = FF->GetMovementState();
	bCachedBusy = FF->IsBusy();
	CachedGroundSpeed = FF->GetVelocity().Size2D();
	bCachedInAir = FF->GetCharacterMovement() ? FF->GetCharacterMovement()->IsFalling() : false;
}