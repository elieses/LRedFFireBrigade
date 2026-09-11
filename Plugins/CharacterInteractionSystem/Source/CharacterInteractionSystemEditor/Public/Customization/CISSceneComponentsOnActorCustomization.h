// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"
#include "Widgets/Input/SComboBox.h"

class UObject;
class UActorComponent;
class AActor;
class UBlueprintGeneratedClass;

class FCISSceneComponentsOnActorCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils);
	virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils);
	FText GetComboBoxContentText() const;

	TSharedRef<SWidget> HandleRowContentsComboBoxGenarateWidget(TSharedPtr<FString> Item);
	void OnComboBoxSelectionChanged(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);

private:
	void FillComponentsOnOwner();

private:
	TSharedPtr<SComboBox<TSharedPtr<FString>>> ComboBox;
	TSharedPtr<IPropertyHandle> SelectedComponentNameHandle;
	TArray<TSharedPtr<FString>> ComponentsOnOwner;
	TArray<FString> ComponentsOnOwnerUnshared;
	TWeakObjectPtr<UObject> OuterObject;
	TWeakObjectPtr<UClass> GeneratedClass;
	TSharedPtr<FString> CurrentSelectedItem;

	bool bTakeSceneComponents = false;
};
