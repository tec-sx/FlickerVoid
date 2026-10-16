// A detail seen only from one examine angle; finding it teaches its clue.
struct FFVExamineSecret
{
	UPROPERTY(EditAnywhere, Category = "Examine", meta = (ToolTip = "Rotation of the actor in the examine view that reveals the secret."))
	FRotator ViewAngle;

	UPROPERTY(EditAnywhere, Category = "Examine", meta = (ToolTip = "Learned when the secret is found."))
	UFVKnowledgeDefinition Clue;
};

// Secrets found by turning the owner in the examine view; each one teaches a clue. Keep at most three per actor.
class UFVExamineSecretsComponent : UActorComponent
{
	UPROPERTY(EditAnywhere, Category = "Examine", meta = (TitleProperty = "Clue"))
	TArray<FFVExamineSecret> Secrets;

	UPROPERTY(EditAnywhere, Category = "Examine", meta = (ClampMin = 0, Units = "Degrees", ToolTip = "How far pitch, yaw and roll may each be from a secret's View Angle."))
	float AngleTolerance = 12.0;

	UFUNCTION(BlueprintPure)
	bool HasSecrets() const
	{
		return Secrets.Num() > 0;
	}

	UFUNCTION(BlueprintPure, meta = (ToolTip = "Secrets whose clue is known."))
	int CountFound() const
	{
		int Found = 0;

		for (int Index = 0; Index < Secrets.Num(); Index++)
		{
			if (Secrets[Index].Clue != nullptr && FVKnowledge::Knows(Secrets[Index].Clue))
			{
				Found++;
			}
		}

		return Found;
	}

	UFUNCTION(BlueprintPure, meta = (ToolTip = "Every secret's clue is known; secrets without a clue don't count."))
	bool AllFound() const
	{
		for (int Index = 0; Index < Secrets.Num(); Index++)
		{
			if (Secrets[Index].Clue != nullptr && !FVKnowledge::Knows(Secrets[Index].Clue))
			{
				return false;
			}
		}

		return true;
	}

	UFUNCTION(meta = (ToolTip = "Learns the clue of every unknown secret within Angle Tolerance of the view rotation. True if one was learned now."))
	bool TryFind(FRotator ViewRotation)
	{
		bool bFoundNew = false;

		for (int Index = 0; Index < Secrets.Num(); Index++)
		{
			UFVKnowledgeDefinition Clue = Secrets[Index].Clue;

			if (Clue == nullptr || FVKnowledge::Knows(Clue) || !IsWithinTolerance(ViewRotation, Secrets[Index].ViewAngle))
			{
				continue;
			}

			if (FVKnowledge::Learn(Clue))
			{
				bFoundNew = true;
			}
		}

		return bFoundNew;
	}

	private bool IsWithinTolerance(FRotator A, FRotator B) const
	{
		return Math::Abs(Math::FindDeltaAngleDegrees(A.Pitch, B.Pitch)) <= AngleTolerance
			&& Math::Abs(Math::FindDeltaAngleDegrees(A.Yaw, B.Yaw)) <= AngleTolerance
			&& Math::Abs(Math::FindDeltaAngleDegrees(A.Roll, B.Roll)) <= AngleTolerance;
	}
}
