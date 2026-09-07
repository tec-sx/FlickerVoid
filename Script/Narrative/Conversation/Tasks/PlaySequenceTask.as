class UPlaySequenceTask : UConversationTaskNode
{
    
	UPROPERTY()
	ULevelSequence Sequence;

	UFUNCTION(BlueprintOverride)
	FConversationTaskResult ExecuteTaskNode(FConversationContext Context) const
	{
		FMovieSceneSequencePlaybackSettings Settings;
		ALevelSequenceActor SequenceActor;
		ULevelSequencePlayer SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(Sequence, Settings, SequenceActor);

		SequenceActor.bOverrideInstanceData = true;
		UDefaultLevelSequenceInstanceData SequenceData = Cast<UDefaultLevelSequenceInstanceData>(SequenceActor.DefaultInstanceData);

		if (IsValid(SequenceData))
		{
			AActor Player = UConversationContextHelpers::GetConversationParticipantActor(Context, GameplayTags::Conversation_Participant_Player);
			FVector Location = FVector(Player.ActorTransform.Location.X, Player.ActorTransform.Location.Y, Player.ActorTransform.Location.Z -90);

			SequenceData.TransformOrigin = FTransform(Player.ActorTransform.Rotation, Location);

			SequencePlayer.Play();
		}

        return UConversationContextHelpers::AdvanceConversation(Context);
	}
}