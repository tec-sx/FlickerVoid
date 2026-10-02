#include "Quest/FVQuest.h"

#include "Engine/World.h"
#include "Facts/FVFactDatabase.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DataValidation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVQuest)

#define LOCTEXT_NAMESPACE "FVQuest"

#if WITH_EDITOR
EDataValidationResult UFVQuestDefinition::IsDataValid(FDataValidationContext& Context) const
{
EDataValidationResult Result = Super::IsDataValid(Context);
for (int32 Index = 0; Index < Objectives.Num(); ++Index)
{
if (!Objectives[Index].Fact.IsValid())
{
Context.AddError(FText::Format(LOCTEXT("MissingObjectiveFact", "Objective {0} has no Fact."), Index));
Result = EDataValidationResult::Invalid;
}
}
return Result;
}
#endif

UFVQuestSubsystem* UFVQuestSubsystem::Get(const UObject* WorldContext)
{
const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
return World ? World->GetSubsystem<UFVQuestSubsystem>() : nullptr;
}

bool UFVQuestSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UFVQuestSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
Super::OnWorldBeginPlay(InWorld);

for (const TSoftObjectPtr<UFVQuestDefinition>& Soft : GetDefault<UFVQuestSettings>()->Quests)
{
if (UFVQuestDefinition* Quest = Soft.LoadSynchronous())
{
Quests.AddUnique(Quest);
}
}

if (UFVFactDatabase* Facts = GetFacts())
{
FactHandle = Facts->OnFactChangedNative().AddUObject(this, &UFVQuestSubsystem::HandleFactChanged);
}
Evaluate();
}

void UFVQuestSubsystem::Deinitialize()
{
if (UFVFactDatabase* Facts = GetFacts())
{
Facts->OnFactChangedNative().Remove(FactHandle);
}
Super::Deinitialize();
}

EFVQuestState UFVQuestSubsystem::GetState(const UFVQuestDefinition* Quest) const
{
const UFVFactDatabase* Facts = GetFacts();
return Facts && Quest ? static_cast<EFVQuestState>(Facts->GetFact(Quest->Id)) : EFVQuestState::NotStarted;
}

bool UFVQuestSubsystem::IsObjectiveComplete(const FFVQuestObjective& Objective) const
{
const UFVFactDatabase* Facts = GetFacts();
return Facts && Facts->GetFact(Objective.Fact) > 0;
}

bool UFVQuestSubsystem::IsObjectiveVisible(const FFVQuestObjective& Objective) const
{
return Objective.VisibleWhen.IsEmpty() || Objective.VisibleWhen.Evaluate(MakeContext());
}

bool UFVQuestSubsystem::StartQuest(UFVQuestDefinition* Quest)
{
if (!SetState(Quest, EFVQuestState::Active, EFVQuestState::NotStarted))
{
return false;
}
Quest->OnStarted.Apply(MakeContext());
Evaluate();
return true;
}

bool UFVQuestSubsystem::CompleteQuest(UFVQuestDefinition* Quest)
{
if (!SetState(Quest, EFVQuestState::Completed, EFVQuestState::Active))
{
return false;
}
Quest->OnCompleted.Apply(MakeContext());
return true;
}

bool UFVQuestSubsystem::FailQuest(UFVQuestDefinition* Quest)
{
if (!SetState(Quest, EFVQuestState::Failed, EFVQuestState::Active))
{
return false;
}
Quest->OnFailed.Apply(MakeContext());
return true;
}

TArray<UFVQuestDefinition*> UFVQuestSubsystem::GetQuests(EFVQuestState State) const
{
TArray<UFVQuestDefinition*> Result;
for (UFVQuestDefinition* Quest : Quests)
{
if (GetState(Quest) == State)
{
Result.Add(Quest);
}
}
return Result;
}

void UFVQuestSubsystem::Evaluate()
{
if (bEvaluating)
{
bDirty = true;
return;
}

TGuardValue<bool> Guard(bEvaluating, true);
do
{
bDirty = false;
for (UFVQuestDefinition* Quest : Quests)
{
EvaluateQuest(Quest);
}
}
while (bDirty);
}

TArray<FSoftObjectPath> UFVQuestSubsystem::GetTrackedQuestPaths() const
{
	TArray<FSoftObjectPath> Paths;
	for (const UFVQuestDefinition* Quest : Quests)
	{
		Paths.Add(FSoftObjectPath(Quest));
	}
	return Paths;
}

void UFVQuestSubsystem::RestoreTrackedQuests(const TArray<FSoftObjectPath>& Paths)
{
	for (const FSoftObjectPath& Path : Paths)
	{
		if (UFVQuestDefinition* Quest = Cast<UFVQuestDefinition>(Path.TryLoad()))
		{
			Quests.AddUnique(Quest);
		}
	}
	Evaluate();
	OnQuestsChanged.Broadcast();
}

void UFVQuestSubsystem::HandleFactChanged(FGameplayTag Tag, int32 OldValue, int32 NewValue)
{
Evaluate();
}

bool UFVQuestSubsystem::SetState(UFVQuestDefinition* Quest, EFVQuestState NewState, EFVQuestState RequiredState)
{
UFVFactDatabase* Facts = GetFacts();
if (!Facts || !Quest || GetState(Quest) != RequiredState)
{
return false;
}

Quests.AddUnique(Quest);
Facts->SetFact(Quest->Id, static_cast<int32>(NewState));
Notify(Quest);
return true;
}

void UFVQuestSubsystem::EvaluateQuest(UFVQuestDefinition* Quest)
{
const FFVConditionContext Context = MakeContext();
const EFVQuestState State = GetState(Quest);

if (State == EFVQuestState::NotStarted)
{
if (!Quest->AutoStartWhen.IsEmpty() && Quest->AutoStartWhen.Evaluate(Context))
{
StartQuest(Quest);
}
return;
}

if (State != EFVQuestState::Active)
{
return;
}

if (!Quest->FailWhen.IsEmpty() && Quest->FailWhen.Evaluate(Context))
{
FailQuest(Quest);
return;
}

if (UpdateObjectives(Quest))
{
Notify(Quest);
}

if (AreRequiredObjectivesDone(Quest))
{
CompleteQuest(Quest);
}
}

bool UFVQuestSubsystem::UpdateObjectives(const UFVQuestDefinition* Quest)
{
UFVFactDatabase* Facts = GetFacts();
const FFVConditionContext Context = MakeContext();
bool bChanged = false;

for (const FFVQuestObjective& Objective : Quest->Objectives)
{
if (IsObjectiveComplete(Objective) || !IsObjectiveVisible(Objective))
{
continue;
}
if (!Objective.CompleteWhen.IsEmpty() && Objective.CompleteWhen.Evaluate(Context))
{
Facts->SetFact(Objective.Fact, 1);
bChanged = true;
}
}
return bChanged;
}

bool UFVQuestSubsystem::AreRequiredObjectivesDone(const UFVQuestDefinition* Quest) const
{
bool bAnyRequired = false;
for (const FFVQuestObjective& Objective : Quest->Objectives)
{
if (Objective.bOptional)
{
continue;
}
bAnyRequired = true;
if (!IsObjectiveComplete(Objective))
{
return false;
}
}
return bAnyRequired;
}

void UFVQuestSubsystem::Notify(UFVQuestDefinition* Quest)
{
LastChanged = Quest;
OnQuestsChanged.Broadcast();
}

FFVConditionContext UFVQuestSubsystem::MakeContext() const
{
FFVConditionContext Context;
Context.WorldContext = GetWorld();
Context.Instigator = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
return Context;
}

UFVFactDatabase* UFVQuestSubsystem::GetFacts() const
{
return UFVFactDatabase::Get(GetWorld());
}

bool FFVCondition_QuestState::EvaluateImpl(const FFVConditionContext& Context) const
{
const UFVQuestSubsystem* Subsystem = UFVQuestSubsystem::Get(Context.WorldContext);
return Subsystem && Quest && Subsystem->GetState(Quest) == State;
}

FText FFVCondition_QuestState::GetDescription() const
{
const FText Name = Quest ? Quest->Display.Name : LOCTEXT("None", "<none>");
return FText::Format(LOCTEXT("State", "{0} is {1}"), Name, StaticEnum<EFVQuestState>()->GetDisplayNameTextByValue(static_cast<int64>(State)));
}

void FFVEffect_SetQuestState::Apply(const FFVConditionContext& Context) const
{
UFVQuestSubsystem* Subsystem = UFVQuestSubsystem::Get(Context.WorldContext);
if (!Subsystem)
{
return;
}

switch (State)
{
case EFVQuestState::Active:    Subsystem->StartQuest(Quest); break;
case EFVQuestState::Completed: Subsystem->CompleteQuest(Quest); break;
case EFVQuestState::Failed:    Subsystem->FailQuest(Quest); break;
default: break;
}
}

FText FFVEffect_SetQuestState::GetDescription() const
{
const FText Name = Quest ? Quest->Display.Name : LOCTEXT("None", "<none>");
return FText::Format(LOCTEXT("Set", "Set {0} to {1}"), Name, StaticEnum<EFVQuestState>()->GetDisplayNameTextByValue(static_cast<int64>(State)));
}

#undef LOCTEXT_NAMESPACE