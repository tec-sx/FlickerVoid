#include "FactDB/Flow/FVFlowNode_LoadFactPreset.h"

#include "FactDB/FVFactPreset.h"
#include "FactDB/FVFactSubsystem.h"
#include "FactDB/Flow/FVFlowNode_FactBase.h"
#include "Interfaces/FlowPredicateInterface.h"
#include "Logging/FVLogCategories.h"
#include "Logging/FVLogSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_LoadFactPreset)

UFVFlowNode_LoadFactPreset::UFVFlowNode_LoadFactPreset()
{
#if WITH_EDITOR
	NodeDisplayStyle = FlowNodeStyle::Fact;
	Category = TEXT("Fact");
#endif

	InputPins = {FFlowPin(TEXT("In"))};
	OutputPins = {FFlowPin(TEXT("Out"))};
}

EFlowAddOnAcceptResult UFVFlowNode_LoadFactPreset::AcceptFlowNodeAddOnChild_Implementation(
	const UFlowNodeAddOn* AddOnTemplate,
	const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	if (IFlowPredicateInterface::ImplementsInterfaceSafe(AddOnTemplate))
	{
		return EFlowAddOnAcceptResult::TentativeAccept;
	}

	return Super::AcceptFlowNodeAddOnChild_Implementation(AddOnTemplate, AdditionalAddOnsToAssumeAreChildren);
}

void UFVFlowNode_LoadFactPreset::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);

	if (const UWorld* World = GetWorld())
	{
#if !UE_BUILD_SHIPPING
		for (const UFVFactPreset* Preset : Presets)
		{
			if (Preset == nullptr)
			{
				FV_LOG_ERROR(LogFVCore, "%hs: Null preset in TArray", __FUNCTION__);
				continue;
			}

			LoadFactPreset(World, Preset);
		}
#endif

		TriggerFirstOutput(true);
	}
	else
	{
		LogError(TEXT("No valid world"));
	}
}

#if WITH_EDITOR
FString UFVFlowNode_LoadFactPreset::GetNodeDescription() const
{
	FTextBuilder Builder;

	for (const UFVFactPreset* Preset : Presets)
	{
		Builder.AppendLine(Preset ? Preset->GetName() : "None");
	}

	return Builder.ToText().ToString();
}

EDataValidationResult UFVFlowNode_LoadFactPreset::ValidateNode()
{
	EDataValidationResult ValidationResult = Super::ValidateNode();

	for (const UFVFactPreset* Preset : Presets)
	{
		if (Preset == nullptr)
		{
			ValidationLog.Warning(TEXT("Some preset is not set"), this);
			ValidationResult = EDataValidationResult::Invalid;
			break;
		}
	}

	return ValidationResult;
}
#endif

void UFVFlowNode_LoadFactPreset::LoadFactPreset(const UObject* WorldContextObject, const UFVFactPreset* Preset)
{
#if !UE_BUILD_SHIPPING
	if (WorldContextObject == nullptr)
	{
		FV_LOG_ERROR(LogFVCore, "%hs: WorldContextObject is null", __FUNCTION__);
		return;
	}

	if (Preset == nullptr)
	{
		FV_LOG_ERROR(LogFVCore, "%hs: Preset is null", __FUNCTION__);
		return;
	}

	UFVFactSubsystem& FactSubsystem = UFVFactSubsystem::Get(WorldContextObject);
	for (auto [Tag, Value] : Preset->PresetValues)
	{
		FactSubsystem.ChangeFactValue(Tag, Value, EFVFactValueChangeType::Set);
	}
#endif
}
