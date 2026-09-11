// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Status/UFFCharacterStatusWidget.h"
#include "Characters/Firefighters/UFFCharacterStatsComponent.h"
#include "Characters/Firefighters/UFFCharacterData.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UFFCharacterStatusWidget::BindCharacterData(UFFCharacterData* CharacterData)
{
	BoundCharacterData = CharacterData;

	if (AvatarImage)
	{
		if (CharacterData && CharacterData->ProfilePicture)
		{
			AvatarImage->SetBrushFromTexture(CharacterData->ProfilePicture);
			AvatarImage->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			AvatarImage->SetBrushFromTexture(nullptr);
			AvatarImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (CharacterNameText)
	{
		CharacterNameText->SetText(CharacterData ? CharacterData->DisplayName : FText::GetEmpty());
	}
}

void UFFCharacterStatusWidget::BindStats(UFFCharacterStatsComponent* Stats)
{
	if (bBound && BoundStats.IsValid())
	{
		BoundStats->OnStatsChanged.RemoveDynamic(this, &UFFCharacterStatusWidget::OnStatsChanged);
	}

	bBound = false;
	BoundStats = Stats;

	if (Stats)
	{
		Stats->OnStatsChanged.AddDynamic(this, &UFFCharacterStatusWidget::OnStatsChanged);
		bBound = true;
	}

	RefreshBars();
}

void UFFCharacterStatusWidget::OnStatsChanged()
{
	RefreshBars();
}

void UFFCharacterStatusWidget::RefreshBars()
{
	UFFCharacterStatsComponent* Stats = BoundStats.Get();
	if (!Stats)
	{
		return;
	}

	// 血量条 + 当前值(HealthText) + 最大值(HealthPercentText)
	if (HealthBar)
	{
		HealthBar->SetPercent(Stats->GetHealthPercent());
	}
	if (HealthText)
	{
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("%d"),
			FMath::RoundToInt(Stats->GetHealth()))));
	}
	if (HealthPercentText)
	{
		HealthPercentText->SetText(FText::FromString(FString::Printf(TEXT("%d"),
			FMath::RoundToInt(Stats->GetMaxHealth()))));
	}

	// 体力条 + 当前值(StaminaText) + 最大值(StaminaPercentText)
	if (StaminaBar)
	{
		StaminaBar->SetPercent(Stats->GetStaminaPercent());
	}
	if (StaminaText)
	{
		StaminaText->SetText(FText::FromString(FString::Printf(TEXT("%d"),
			FMath::RoundToInt(Stats->GetStamina()))));
	}
	if (StaminaPercentText)
	{
		StaminaPercentText->SetText(FText::FromString(FString::Printf(TEXT("%d"),
			FMath::RoundToInt(Stats->GetMaxStamina()))));
	}
}