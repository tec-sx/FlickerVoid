class UPrintStringTask : UConversationTaskNode
{    
	UPROPERTY()
	FText Text;

	UFUNCTION(BlueprintOverride)
	FConversationTaskResult ExecuteTaskNode(FConversationContext Context) const
	{
		Print(Text.ToString());

        return UConversationContextHelpers::AdvanceConversation(Context);
	}
}