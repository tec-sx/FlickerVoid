#include "Customizations/FVConditionSetCustomization.h"

#include "Conditions/FVCondition.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyHandle.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FVConditionSetCustomization"

TSharedRef<IPropertyTypeCustomization> FFVConditionSetCustomization::MakeInstance()
{
	return MakeShared<FFVConditionSetCustomization>();
}

void FFVConditionSetCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& Utils)
{
	Handle = PropertyHandle;

	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(300.f)
		[
			SNew(STextBlock)
			.Text(this, &FFVConditionSetCustomization::GetSummary)
			.Font(Utils.GetRegularFont())
			.AutoWrapText(true)
		];
}

void FFVConditionSetCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& Utils)
{
	uint32 NumChildren = 0;
	PropertyHandle->GetNumChildren(NumChildren);
	for (uint32 Index = 0; Index < NumChildren; ++Index)
	{
		ChildBuilder.AddProperty(PropertyHandle->GetChildHandle(Index).ToSharedRef());
	}
}

FText FFVConditionSetCustomization::GetSummary() const
{
	if (!Handle.IsValid())
	{
		return FText::GetEmpty();
	}

	void* Data = nullptr;
	if (Handle->GetValueData(Data) != FPropertyAccess::Success || !Data)
	{
		return LOCTEXT("Multiple", "Multiple Values");
	}

	const FFVConditionSet* Set = static_cast<const FFVConditionSet*>(Data);
	if (Set->IsEmpty())
	{
		return LOCTEXT("Always", "Always");
	}

	const FText Description = Set->GetDescription();
	return Description.IsEmpty() ? FText::Format(LOCTEXT("Count", "{0} condition(s)"), Set->Conditions.Num()) : Description;
}

#undef LOCTEXT_NAMESPACE
