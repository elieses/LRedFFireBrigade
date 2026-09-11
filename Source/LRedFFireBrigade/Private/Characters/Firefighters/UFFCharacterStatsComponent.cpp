// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Firefighters/UFFCharacterStatsComponent.h"

void UFFCharacterStatsComponent::ApplyHealthDelta(float Delta)
{
	SetHealth(Health + Delta);
}

void UFFCharacterStatsComponent::ApplyStaminaDelta(float Delta)
{
	SetStamina(Stamina + Delta);
}

void UFFCharacterStatsComponent::SetHealth(float Value)
{
	const float Clamped = FMath::Clamp(Value, 0.f, FMath::Max(0.f, MaxHealth));
	if (!FMath::IsNearlyEqual(Health, Clamped))
	{
		Health = Clamped;
		OnStatsChanged.Broadcast();
	}
}

void UFFCharacterStatsComponent::SetStamina(float Value)
{
	const float Clamped = FMath::Clamp(Value, 0.f, FMath::Max(0.f, MaxStamina));
	if (!FMath::IsNearlyEqual(Stamina, Clamped))
	{
		Stamina = Clamped;
		OnStatsChanged.Broadcast();
	}
}

void UFFCharacterStatsComponent::SetMaxHealth(float Value)
{
	MaxHealth = FMath::Max(0.f, Value);

	// 新上限生效：重新夹取当前血量
	const float Clamped = FMath::Clamp(Health, 0.f, MaxHealth);
	if (!FMath::IsNearlyEqual(Health, Clamped))
	{
		Health = Clamped;
		OnStatsChanged.Broadcast();
	}
	else
	{
		// 上限本身变了也要刷新 UI（数字的"最大"部分）
		OnStatsChanged.Broadcast();
	}
}

void UFFCharacterStatsComponent::SetMaxStamina(float Value)
{
	MaxStamina = FMath::Max(0.f, Value);

	const float Clamped = FMath::Clamp(Stamina, 0.f, MaxStamina);
	if (!FMath::IsNearlyEqual(Stamina, Clamped))
	{
		Stamina = Clamped;
		OnStatsChanged.Broadcast();
	}
	else
	{
		OnStatsChanged.Broadcast();
	}
}