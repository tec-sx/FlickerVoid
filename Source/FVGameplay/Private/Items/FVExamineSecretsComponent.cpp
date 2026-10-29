#include "Items/FVExamineSecretsComponent.h"

#include "Knowledge/FVKnowledge.h"
#include "Knowledge/FVKnowledgeDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVExamineSecretsComponent)

#define LOCTEXT_NAMESPACE "FVExamineSecretsComponent"

int32 UFVExamineSecretsComponent::CountFound() const
{
	int32 Found = 0;
	for (const FFVExamineSecret& Secret : Secrets)
	{
		if (Secret.Clue && UFVKnowledgeLibrary::Knows(this, Secret.Clue))
		{
			++Found;
		}
	}
	return Found;
}

bool UFVExamineSecretsComponent::AllFound() const
{
	for (const FFVExamineSecret& Secret : Secrets)
	{
		if (Secret.Clue && !UFVKnowledgeLibrary::Knows(this, Secret.Clue))
		{
			return false;
		}
	}
	return true;
}

bool UFVExamineSecretsComponent::TryFind(const FRotator& ViewRotation)
{
	bool bFoundNew = false;
	for (const FFVExamineSecret& Secret : Secrets)
	{
		if (!Secret.Clue || UFVKnowledgeLibrary::Knows(this, Secret.Clue) || !IsWithinTolerance(ViewRotation, Secret.ViewAngle))
		{
			continue;
		}

		if (UFVKnowledgeLibrary::Learn(this, Secret.Clue))
		{
			bFoundNew = true;
		}
	}
	return bFoundNew;
}

#if WITH_EDITOR
EDataValidationResult UFVExamineSecretsComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	// Rule of three.
	if (Secrets.Num() > 3)
	{
		Context.AddWarning(LOCTEXT("TooManySecrets", "Keep at most three secrets per actor."));
	}

	for (int32 Index = 0; Index < Secrets.Num(); ++Index)
	{
		if (!Secrets[Index].Clue)
		{
			Context.AddWarning(FText::Format(LOCTEXT("NoClue", "Secret {0} has no Clue and can't be found."), Index));
		}
	}

	return Result;
}
#endif

bool UFVExamineSecretsComponent::IsWithinTolerance(const FRotator& ViewRotation, const FRotator& ViewAngle) const
{
	return FMath::Abs(FMath::FindDeltaAngleDegrees(ViewRotation.Pitch, ViewAngle.Pitch)) <= AngleTolerance
		&& FMath::Abs(FMath::FindDeltaAngleDegrees(ViewRotation.Yaw, ViewAngle.Yaw)) <= AngleTolerance
		&& FMath::Abs(FMath::FindDeltaAngleDegrees(ViewRotation.Roll, ViewAngle.Roll)) <= AngleTolerance;
}

#undef LOCTEXT_NAMESPACE
