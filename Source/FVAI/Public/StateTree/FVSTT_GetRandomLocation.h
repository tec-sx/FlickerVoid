// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "FVSTT_GetRandomLocation.generated.h"

class AFVAICharacterController;
class AFVAICharacter;

USTRUCT(
	BlueprintType, 
	Category = "Flicker Void",
	meta = (DisplayName = "Get Random Location Instance Data",
		Description = "Contains information about instance data."))
struct FGetRandomLocationTaskInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = Context, meta = (Context))
	TObjectPtr<AFVAICharacter> Actor;
	
	UPROPERTY(BlueprintReadOnly, Category = Context, meta = (Context))
	TObjectPtr<AFVAICharacterController> AIController;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	float SearchRadius = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Output, meta = (Output))
	FVector RandomLocation = FVector::ZeroVector;
};

USTRUCT(Category = "Flicker Void",
		meta = (DisplayName = "Get Random Location",
		Description = "StateTree task to get a random reachable location within a radius around an actor."))
struct FLICKERVOIDAI_API FFVSTT_GetRandomLocation : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FGetRandomLocationTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FGetRandomLocationTaskInstanceData::StaticStruct(); }
    
	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context, 
		const FStateTreeTransitionResult& Transition) const override;
	
#if WITH_EDITOR
	virtual FName GetIconName() const override { return FName("GenericPlay"); }
	virtual FColor GetIconColor() const override { return FColor::Silver; }
	virtual FText GetDescription(const FGuid& ID,
		FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override
	{
		if (const FGetRandomLocationTaskInstanceData* Data = InstanceDataView.GetPtr<FGetRandomLocationTaskInstanceData>())
		{
			const float Radius = Data->SearchRadius;

			return FText::Format(
				NSLOCTEXT("FlickerVoid", "GetRandomLocationDescriptionDetailed", "Get random location around {0}, radius: {1} units"),
				FText::FromString("Actor"), // Placeholder for Actor name, if needed
				FText::AsNumber(Radius)
			);
		}

		// Fallback if data isn’t bound yet
		return NSLOCTEXT("FlickerVoid", "GetRandomLocationTask_NoData", "Get Random Location (No Radius Defined)");
	}
#endif
};
