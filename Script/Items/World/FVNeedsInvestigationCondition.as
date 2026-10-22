// True when the target is a readable that must be investigated before it can be read. Use it inverted on the Read offer.
class UFVNeedsInvestigationCondition : UFVScriptCondition
{
	UFUNCTION(BlueprintOverride)
	bool Evaluate(FFVConditionContext Context) const
	{
		AFVReadable Readable = Cast<AFVReadable>(Context.Target);

		return Readable != nullptr && Readable.NeedsInvestigation();
	}

	UFUNCTION(BlueprintOverride)
	FText GetDescription() const
	{
		return FText::FromString("Target needs investigation");
	}
}
