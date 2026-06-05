// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/FVDialogueSubsystem.h"
#include "Dialogue/FVDialogueQuery.h"
#include "Dialogue/FVDialogueConditionEvaluator.h"
#include "Subsystems/FVQuestSubsystem.h"
#include "Subsystems/FVWorldStateSubsystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDialogueSubsystem)

void UFVDialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Ensure dependencies are initialized
	Collection.InitializeDependency<UFVWorldStateSubsystem>();
	Collection.InitializeDependency<UFVQuestSubsystem>();

	DialogueQuery = NewObject<UFVDialogueQuery>(this);
}

void UFVDialogueSubsystem::Deinitialize()
{
	if (CurrentAudioComponent)
	{
		CurrentAudioComponent->Stop();
		CurrentAudioComponent = nullptr;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LineTimerHandle);
	}

	Super::Deinitialize();
}

void UFVDialogueSubsystem::SetDialogueTable(UDataTable* DialogueTable)
{
	if (DialogueQuery && DialogueTable)
	{
		DialogueQuery->Initialize(DialogueTable, nullptr, GetWorld());
	}
}

void UFVDialogueSubsystem::SetConditionsTable(UDataTable* ConditionsTable)
{
	if (DialogueQuery)
	{
		DialogueQuery->Initialize(nullptr, ConditionsTable, GetWorld());
	}
}

void UFVDialogueSubsystem::SetResponsesTable(UDataTable* InResponsesTable)
{
	ResponsesTable = InResponsesTable;
}

void UFVDialogueSubsystem::SetSpeakersTable(UDataTable* InSpeakersTable)
{
	SpeakersTable = InSpeakersTable;
}

void UFVDialogueSubsystem::AddDialogueTable(UDataTable* AdditionalTable)
{
	if (DialogueQuery)
	{
		DialogueQuery->AddDialogueTable(AdditionalTable);
	}
}

bool UFVDialogueSubsystem::SendSignal(FName SignalName, const FFVDialogueContext& Context)
{
	if (!DialogueQuery || SignalName == NAME_None)
	{
		return false;
	}

	FFVDialogueQueryResult Result = DialogueQuery->QueryDialogue(SignalName, Context);
	
	if (Result.bSuccess && CheckUsageLimits(Result.SelectedLine))
	{
		PlayLine(Result.SelectedLine, Context);
		return true;
	}

	return false;
}

bool UFVDialogueSubsystem::SendSignalFromActor(FName SignalName, AActor* Speaker, AActor* Listener)
{
	FFVDialogueContext Context;
	
	if (Speaker)
	{
		Context.SpeakerActor = Speaker;
		Context.SpeakerId = FName(*Speaker->GetName());
	}
	
	if (Listener)
	{
		Context.ListenerActor = Listener;
	}
	else if (UWorld* World = GetWorld())
	{
		// Default to player pawn
		Context.ListenerActor = UGameplayStatics::GetPlayerPawn(World, 0);
	}

	return SendSignal(SignalName, Context);
}

bool UFVDialogueSubsystem::StartConversation(FName ConversationId, FName InitialSignal, const FFVDialogueContext& Context)
{
	if (IsInConversation())
	{
		EndConversation();
	}

	ActiveConversation = FFVActiveConversation();
	ActiveConversation.ConversationId = ConversationId;
	ActiveConversation.CurrentSignal = InitialSignal;
	ActiveConversation.Context = Context;

	OnConversationStarted.Broadcast();

	return SendSignal(InitialSignal, Context);
}

void UFVDialogueSubsystem::SelectResponse(int32 ResponseIndex)
{
	if (!ActiveConversation.bWaitingForResponse ||
		ResponseIndex < 0 ||
		ResponseIndex >= ActiveConversation.AvailableResponses.Num())
	{
		return;
	}

	SelectResponseById(ActiveConversation.AvailableResponses[ResponseIndex].ResponseId);
}

void UFVDialogueSubsystem::SelectResponseById(FName ResponseId)
{
	if (!ActiveConversation.bWaitingForResponse)
	{
		return;
	}

	const FFVDialogueResponse* SelectedResponse = nullptr;
	for (const FFVDialogueResponse& Response : ActiveConversation.AvailableResponses)
	{
		if (Response.ResponseId == ResponseId)
		{
			SelectedResponse = &Response;
			break;
		}
	}

	if (!SelectedResponse)
	{
		return;
	}

	ActiveConversation.bWaitingForResponse = false;

	// Apply response effects
	for (const FString& EffectString : SelectedResponse->InlineEffects)
	{
		ApplyEffectString(EffectString);
	}

	OnDialogueResponseSelected.Broadcast(*SelectedResponse);

	// Determine next signal based on skill check (if applicable)
	FName NextSignal = SelectedResponse->ResultSignal;
	
	if (SelectedResponse->SkillCheckType != NAME_None && SelectedResponse->SkillCheckDifficulty > 0.0f)
	{
		// TODO: Implement skill check logic
		// For now, always succeed
		bool bPassed = true; // PerformSkillCheck(SelectedResponse->SkillCheckType, SelectedResponse->SkillCheckDifficulty, ActiveConversation.Context);
		
		if (!bPassed && SelectedResponse->FailureSignal != NAME_None)
		{
			NextSignal = SelectedResponse->FailureSignal;
		}
	}

	// Continue conversation
	if (NextSignal != NAME_None)
	{
		ActiveConversation.CurrentSignal = NextSignal;
		SendSignal(NextSignal, ActiveConversation.Context);
	}
	else
	{
		EndConversation();
	}
}

void UFVDialogueSubsystem::SkipCurrentLine()
{
	if (bIsPlayingLine)
	{
		if (CurrentAudioComponent)
		{
			CurrentAudioComponent->Stop();
		}

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(LineTimerHandle);
		}

		OnDialogueLineEnded.Broadcast(CurrentLine, true);
		OnLineFinished();
	}
}

void UFVDialogueSubsystem::EndConversation()
{
	if (!IsInConversation())
	{
		return;
	}

	if (bIsPlayingLine)
	{
		SkipCurrentLine();
	}

	ActiveConversation = FFVActiveConversation();
	OnConversationEnded.Broadcast();
}

bool UFVDialogueSubsystem::IsInConversation() const
{
	return ActiveConversation.ConversationId != NAME_None;
}

bool UFVDialogueSubsystem::IsLineOnCooldown(FName LineId) const
{
	const float* CooldownEnd = LineCooldowns.Find(LineId);
	if (!CooldownEnd)
	{
		return false;
	}

	if (UFVWorldStateSubsystem* WorldState = GetGameInstance()->GetSubsystem<UFVWorldStateSubsystem>())
	{
		return WorldState->GetCurrentGameTime() < *CooldownEnd;
	}

	return false;
}

int32 UFVDialogueSubsystem::GetLineUseCount(FName LineId) const
{
	const int32* Count = LineUseCounts.Find(LineId);
	return Count ? *Count : 0;
}

void UFVDialogueSubsystem::ResetUsageTracking()
{
	LineCooldowns.Empty();
	LineUseCounts.Empty();
}

bool UFVDialogueSubsystem::GetSpeakerInfo(FName SpeakerId, FFVSpeakerInfo& OutInfo) const
{
	if (!SpeakersTable)
	{
		return false;
	}

	if (const FFVSpeakerInfo* Info = SpeakersTable->FindRow<FFVSpeakerInfo>(SpeakerId, TEXT("")))
	{
		OutInfo = *Info;
		return true;
	}

	return false;
}

void UFVDialogueSubsystem::PlayLine(const FFVDialogueLine& Line, const FFVDialogueContext& Context)
{
	CurrentLine = Line;
	bIsPlayingLine = true;

	// Update usage tracking
	UpdateUsageTracking(Line);

	// Add to conversation history if in conversation
	if (IsInConversation())
	{
		ActiveConversation.History.Add(Line);
	}

	// Broadcast line started
	OnDialogueLineStarted.Broadcast(Line, Context);

	// Play audio if available
	if (!Line.VoiceAudio.IsNull())
	{
		USoundBase* Sound = Line.VoiceAudio.LoadSynchronous();
		if (Sound)
		{
			// Play at speaker location if available
			if (AActor* Speaker = Context.SpeakerActor.Get())
			{
				CurrentAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
					GetWorld(),
					Sound,
					Speaker->GetActorLocation(),
					FRotator::ZeroRotator,
					1.0f, 1.0f, 0.0f,
					nullptr, nullptr, true
				);
			}
			else
			{
				CurrentAudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), Sound);
			}
		}
	}

	// Set timer for line duration
	float Duration = CalculateLineDuration(Line);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			LineTimerHandle,
			this,
			&UFVDialogueSubsystem::OnLineFinished,
			Duration,
			false
		);
	}
}

void UFVDialogueSubsystem::OnLineFinished()
{
	if (!bIsPlayingLine)
	{
		return;
	}

	bIsPlayingLine = false;
	CurrentAudioComponent = nullptr;

	// Apply effects
	ApplyLineEffects(CurrentLine);

	OnDialogueLineEnded.Broadcast(CurrentLine, false);

	// Check if conversation should end
	if (CurrentLine.bEndsConversation)
	{
		EndConversation();
		return;
	}

	// Process next signal or show responses
	ProcessNextSignal();
}

void UFVDialogueSubsystem::ApplyLineEffects(const FFVDialogueLine& Line)
{
	for (const FString& EffectString : Line.InlineEffects)
	{
		ApplyEffectString(EffectString);
	}
}

void UFVDialogueSubsystem::ApplyEffectString(const FString& EffectString)
{
	TArray<FString> Parts;
	EffectString.ParseIntoArray(Parts, TEXT(":"), true);

	if (Parts.Num() < 2)
	{
		return;
	}

	const FString& EffectType = Parts[0].ToLower();
	const FString& Target = Parts[1];

	UFVWorldStateSubsystem* WorldState = GetGameInstance()->GetSubsystem<UFVWorldStateSubsystem>();
	UFVQuestSubsystem* QuestSubsystem = GetGameInstance()->GetSubsystem<UFVQuestSubsystem>();

	if (EffectType == TEXT("addtag") || EffectType == TEXT("add"))
	{
		if (WorldState)
		{
			FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Target), false);
			if (Tag.IsValid())
			{
				WorldState->AddWorldStateTag(Tag);
			}
		}
	}
	else if (EffectType == TEXT("removetag") || EffectType == TEXT("remove"))
	{
		if (WorldState)
		{
			FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Target), false);
			if (Tag.IsValid())
			{
				WorldState->RemoveWorldStateTag(Tag);
			}
		}
	}
	else if (EffectType == TEXT("modifyrelationship") || EffectType == TEXT("relationship") || EffectType == TEXT("rel"))
	{
		if (WorldState && Parts.Num() >= 3)
		{
			float Delta = FCString::Atof(*Parts[2]);
			WorldState->ModifyNPCRelationship(FName(*Target), Delta);
		}
	}
	else if (EffectType == TEXT("startquest") || EffectType == TEXT("quest"))
	{
		if (QuestSubsystem)
		{
			// TODO: Start quest by ID
		}
	}
	// Add more effect types as needed
}

void UFVDialogueSubsystem::ProcessNextSignal()
{
	if (!IsInConversation())
	{
		return;
	}

	// Check for next signal
	if (CurrentLine.NextSignal != NAME_None)
	{
		// Delay if specified
		if (CurrentLine.NextSignalDelay > 0.0f)
		{
			if (UWorld* World = GetWorld())
			{
				FTimerHandle DelayHandle;
				World->GetTimerManager().SetTimer(
					DelayHandle,
					[this]()
					{
						if (IsInConversation())
						{
							ActiveConversation.CurrentSignal = CurrentLine.NextSignal;
							SendSignal(CurrentLine.NextSignal, ActiveConversation.Context);
						}
					},
					CurrentLine.NextSignalDelay,
					false
				);
			}
		}
		else
		{
			ActiveConversation.CurrentSignal = CurrentLine.NextSignal;
			SendSignal(CurrentLine.NextSignal, ActiveConversation.Context);
		}
	}
	else
	{
		// Check for player responses
		if (DialogueQuery && ResponsesTable)
		{
			TArray<FFVDialogueResponse> Responses = DialogueQuery->QueryResponses(
				ActiveConversation.CurrentSignal,
				ActiveConversation.Context,
				ResponsesTable
			);

			if (Responses.Num() > 0)
			{
				ActiveConversation.bWaitingForResponse = true;
				ActiveConversation.AvailableResponses = Responses;
				OnDialogueResponsesAvailable.Broadcast(Responses);
			}
			else
			{
				// No responses and no next signal - end conversation
				EndConversation();
			}
		}
		else
		{
			EndConversation();
		}
	}
}

void UFVDialogueSubsystem::UpdateUsageTracking(const FFVDialogueLine& Line)
{
	// Update use count
	int32& UseCount = LineUseCounts.FindOrAdd(Line.LineId);
	UseCount++;

	// Set cooldown
	if (Line.CooldownHours > 0.0f)
	{
		if (UFVWorldStateSubsystem* WorldState = GetGameInstance()->GetSubsystem<UFVWorldStateSubsystem>())
		{
			LineCooldowns.Add(Line.LineId, WorldState->GetCurrentGameTime() + Line.CooldownHours);
		}
	}
}

bool UFVDialogueSubsystem::CheckUsageLimits(const FFVDialogueLine& Line) const
{
	// Check cooldown
	if (IsLineOnCooldown(Line.LineId))
	{
		return false;
	}

	// Check max use count
	if (Line.MaxUseCount > 0)
	{
		int32 UseCount = GetLineUseCount(Line.LineId);
		if (UseCount >= Line.MaxUseCount)
		{
			return false;
		}
	}

	return true;
}

float UFVDialogueSubsystem::CalculateLineDuration(const FFVDialogueLine& Line) const
{
	// Use override if specified
	if (Line.DurationOverride > 0.0f)
	{
		return Line.DurationOverride;
	}

	// Try to get audio duration
	if (!Line.VoiceAudio.IsNull())
	{
		USoundBase* Sound = Line.VoiceAudio.LoadSynchronous();
		if (Sound)
		{
			return Sound->GetDuration();
		}
	}

	// Calculate from text length (rough estimate: 15 characters per second)
	const FString TextString = Line.Text.ToString();
	float TextDuration = TextString.Len() / 15.0f;

	return FMath::Max(1.0f, TextDuration);
}