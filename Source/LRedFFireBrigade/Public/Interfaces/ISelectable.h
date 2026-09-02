// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ISelectable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class USelectable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LREDFFIREBRIGADE_API ISelectable
{
	GENERATED_BODY()

	
public:
	virtual void SetSelected(bool bSelected) = 0;
};
