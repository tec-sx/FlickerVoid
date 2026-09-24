#include "Core/FVInteractionLogger.h"

#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY(LogInteraction);

void PrintString(const ELogVerbosity::Type Verbosity, const FString& Message, FLinearColor Color, float Duration)
{
	if (!GWorld) return;

#if WITH_EDITOR
	FMsg::Logf(__FILE__, __LINE__, LogInteraction.GetCategoryName(), Verbosity, TEXT("%s"), *Message);
#endif
		
	UKismetSystemLibrary::PrintString(GWorld, Message, true, true, Color, Duration);
}
