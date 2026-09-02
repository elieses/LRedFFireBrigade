// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IControllable.generated.h"

struct FInputActionValue;
// This class does not need to be modified.
UINTERFACE()
class UControllable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LREDFFIREBRIGADE_API IControllable
{
	GENERATED_BODY()
public:
	virtual void Move(const FInputActionValue& Value) = 0;
	virtual void Zoom(const FInputActionValue& Value) = 0;
	virtual void Rotate(const FInputActionValue& Value) = 0;
};
