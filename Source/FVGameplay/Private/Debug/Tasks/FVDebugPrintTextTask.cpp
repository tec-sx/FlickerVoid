// Fill out your copyright notice in the Description page of Project Settings.


#include "Debug/Tasks/FVDebugPrintTextTask.h"
#include "DrawDebugHelpers.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDebugPrintTextTask)

EStateTreeRunStatus UFVDebugPrintTextTask::EnterState(
    FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);
	
	// Resolve world
    UWorld* World = Actor ? Actor->GetWorld() : Context.GetWorld();

    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UFVDebugFormattedText: No valid World!"));
        return EStateTreeRunStatus::Failed;
    }

    // Build named arguments
    FFormatNamedArguments Args;
    for (const auto& [Key, Type, FloatValue, IntValue, StringValue, NameValue, TextValue, BoolValue] : Params)
    {
        const FString KeyString = Key.ToString();

        switch (Type)
        {
            case EFVDebugParamType::Float:
                Args.Add(KeyString, FFormatArgumentValue(FloatValue));
                break;

            case EFVDebugParamType::Int:
                Args.Add(KeyString, FFormatArgumentValue(IntValue));
                break;

            case EFVDebugParamType::String:
                // Convert FString → FText → FFormatArgumentValue
                Args.Add(KeyString, FFormatArgumentValue(FText::FromString(StringValue)));
                break;

            case EFVDebugParamType::Name:
                // Convert FName → FText → FFormatArgumentValue
                Args.Add(KeyString, FFormatArgumentValue(FText::FromName(NameValue)));
                break;

            case EFVDebugParamType::Text:
                // Already an FText
                Args.Add(KeyString, FFormatArgumentValue(TextValue));
                break;

            case EFVDebugParamType::Bool:
                Args.Add(KeyString, FFormatArgumentValue(BoolValue));
        }
    }

    // Format the text
    const FText FormattedText = FText::Format(FormatText, Args);
    const FString Message = FormattedText.ToString();

    // Pick a draw location
    const FVector DrawLoc = Actor ? Actor->GetActorLocation() : FVector::ZeroVector;

    // Draw (Duration=0 → one frame)
    DrawDebugString(World, DrawLoc, Message, Actor.Get(), FColor::White, /*Duration=*/2.f, /*bDrawShadow=*/true);

    // Optionally print to screen (black, two seconds)
    if (PrintToScreen && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            /*Key=*/-1,
            /*TimeToDisplay=*/2.0f,
            /*Color=*/FColor::Black,
            /*Message=*/Message
        );
    }

    return EStateTreeRunStatus::Succeeded;
}
