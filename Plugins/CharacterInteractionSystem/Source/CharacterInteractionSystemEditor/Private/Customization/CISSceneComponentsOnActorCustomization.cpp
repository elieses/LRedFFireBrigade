// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#include "Customization/CISSceneComponentsOnActorCustomization.h"

#include "Common/CISTypes.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "GameFramework/Actor.h"
#include "Internationalization/Internationalization.h"
#include "Templates/SharedPointer.h"
#include "UObject/Script.h"

#define LOCTEXT_NAMESPACE "FWZFFunctionPickerCustomization"

TSharedRef<IPropertyTypeCustomization> FCISSceneComponentsOnActorCustomization::MakeInstance()
{
	return MakeShareable(new FCISSceneComponentsOnActorCustomization());
}

void FCISSceneComponentsOnActorCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TArray<UObject*> outerObjects;
	PropertyHandle->GetOuterObjects(outerObjects);
	if (outerObjects.Num() != 0)
	{
		OuterObject = outerObjects[0];
	}

	if (!OuterObject.IsValid())
	{
		return;
	}

	UObject* outerFinder = OuterObject.Get();

	while (outerFinder != nullptr)
	{
		if (auto outerClass = Cast<UClass>(outerFinder))
		{
			if (outerClass->GetDefaultObject<AActor>())
			{
				GeneratedClass = outerClass;
				break;
			}
		}
		else if (auto outerActor = Cast<AActor>(outerFinder))
		{
			GeneratedClass = outerActor->GetClass();
			break;
		}

		outerFinder = outerFinder->GetOuter();
	}

	HeaderRow
	    .NameContent()
	        [PropertyHandle->CreatePropertyNameWidget(FText::GetEmpty(), FText::GetEmpty())];
}

void FCISSceneComponentsOnActorCustomization::CustomizeChildren(TSharedRef<class IPropertyHandle> PropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	SelectedComponentNameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FCISSceneComponentsOnActor, SelectedSceneComponentName));

	if (!SelectedComponentNameHandle.IsValid() || !SelectedComponentNameHandle->IsValidHandle())
	{
		SelectedComponentNameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FCISPrimitiveComponentsOnActor, SelectedPrimitiveComponentName));
	}
	else
	{
		bTakeSceneComponents = true;
	}

	if (!SelectedComponentNameHandle.IsValid() || !SelectedComponentNameHandle->IsValidHandle() || !OuterObject.IsValid())
	{
		return;
	}

	FillComponentsOnOwner();

	FString value;
	SelectedComponentNameHandle->GetValue(value);
	CurrentSelectedItem = MakeShareable(new FString(value));

	if (!ComponentsOnOwnerUnshared.Contains(*CurrentSelectedItem.Get()))
	{
		CurrentSelectedItem.Reset();
	}

	auto socketNameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FCISSceneComponentsOnActor, SocketName));

	StructBuilder.AddCustomRow(LOCTEXT("ComponentsInOwner", "Components In Owner"))
	    .NameContent()
	        [SNew(SVerticalBox)
	         + SVerticalBox::Slot()
	               .Padding(5, 5)
	                   [SNew(STextBlock)
	                        .Text(LOCTEXT("Component", "Component"))
	                        .Font(StructCustomizationUtils.GetRegularFont())]

	         + SVerticalBox::Slot()
	               .Padding(5, 5)
	                   [SNew(STextBlock)
	                        .Text(LOCTEXT("SocketName", "Socket Name"))
	                        .Font(StructCustomizationUtils.GetRegularFont())]]
	    .ValueContent()
	        [SNew(SVerticalBox)
	         + SVerticalBox::Slot()
	               .Padding(5, 5)
	                   [SAssignNew(ComboBox, SComboBox<TSharedPtr<FString>>)
	                        .OptionsSource(&ComponentsOnOwner)
	                        .OnGenerateWidget(this, &FCISSceneComponentsOnActorCustomization::HandleRowContentsComboBoxGenarateWidget)
	                        .OnSelectionChanged(this, &FCISSceneComponentsOnActorCustomization::OnComboBoxSelectionChanged)
	                        .InitiallySelectedItem(MakeShareable(new FString()))
	                            [SNew(STextBlock)
	                                 .Text(this, &FCISSceneComponentsOnActorCustomization::GetComboBoxContentText)]]
	         + SVerticalBox::Slot()
	               .Padding(5, 5)
	                   [socketNameHandle->CreatePropertyValueWidget(false)]

	];
}

FText FCISSceneComponentsOnActorCustomization::GetComboBoxContentText() const
{
	return CurrentSelectedItem.IsValid() && !CurrentSelectedItem.Get()->IsEmpty() ? FText::FromString(*CurrentSelectedItem.Get()) : FText::FromString("Select Component");
}

TSharedRef<SWidget> FCISSceneComponentsOnActorCustomization::HandleRowContentsComboBoxGenarateWidget(TSharedPtr<FString> Item)
{
	return SNew(STextBlock)
	    .Text(FText::FromString(*Item));
}

void FCISSceneComponentsOnActorCustomization::OnComboBoxSelectionChanged(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedComponentNameHandle.IsValid() && SelectedComponentNameHandle->IsValidHandle() && SelectedItem.IsValid())
	{
		CurrentSelectedItem = SelectedItem;
		SelectedComponentNameHandle->SetValue(*SelectedItem);
	}
}

void FCISSceneComponentsOnActorCustomization::FillComponentsOnOwner()
{
	if (!GeneratedClass.IsValid())
	{
		return;
	}

	ComponentsOnOwner.Empty();

	for (auto objIt = TFieldIterator<FObjectProperty>(GeneratedClass.Get()); objIt; ++objIt)
	{
		auto value = *objIt;

		if ((bTakeSceneComponents && value->PropertyClass->IsChildOf<USceneComponent>()) || (value->PropertyClass->IsChildOf<UPrimitiveComponent>()))
		{
			auto name = value->GetFName().ToString();
			ComponentsOnOwner.Add(MakeShareable(new FString(name)));
			ComponentsOnOwnerUnshared.Add(name);
		}
	}
}

#undef LOCTEXT_NAMESPACE
