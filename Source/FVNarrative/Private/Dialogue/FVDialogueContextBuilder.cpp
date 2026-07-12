#include "Dialogue/FVDialogueContextBuilder.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Dialogue/FVDialogueTargetComponent.h"
#include "Subsystems/FVWorldStateSubsystem.h"

// Project-specific forward declare — your NPC component
// Replace with your actual header
// #include "NPC/DialogueNPCComponent.h"

// ─────────────────────────────────────────────────────────────────────────────

FFVDialogueContext UFVDialogueContextBuilder::Build(
    const FGameplayTag& SpeakerID,
    const FGameplayTag& Signal,
    AActor* Speaker,
    AActor* Listener,
    const TSet<FName>& ConsumedLineIDs) const
{
    FFVDialogueContext Context;

    if (!Speaker || !Listener)
    {
        UE_LOG(LogTemp, Warning, TEXT("UFVDialogueContextBuilder::Build — null actor(s) passed."));
        return Context;
    }

    UWorld* World = Speaker->GetWorld();

    Context.SpeakerID  = SpeakerID;
    Context.Speaker    = Speaker;
    Context.Listener   = Listener;
    Context.Signal     = Signal;
    Context.Timestamp  = FDateTime::UtcNow();

    Context.WorldStateTags     = BuildWorldStateTags(World);
    Context.InstigatorTags     = BuildInstigatorTags(Speaker);
    Context.TargetTags         = BuildTargetTags(Listener);
    Context.RelationshipValue  = ResolveRelationshipValue(Speaker, Listener);
    Context.NotorietyValue     = ResolveNotorietyValue(Speaker);
    Context.FactionStandings   = BuildFactionStandings(Speaker);
    Context.ActiveQuestTags    = BuildActiveQuestTags(World);
    Context.ConsumedLineIDs    = ConsumedLineIDs;
    
    Context.SessionPlayedLineIDs.Empty();

    return Context;
}

// ─────────────────────────────────────────────────────────────────────────────

FGameplayTagContainer UFVDialogueContextBuilder::BuildWorldStateTags(UWorld* World) const
{
    FGameplayTagContainer Tags;

    if (!World) return Tags;
    
    const UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance) return Tags;
    
    if (const UFVWorldStateSubsystem* WorldStateSubsystem = GameInstance->GetSubsystem<UFVWorldStateSubsystem>())
    {
        Tags = WorldStateSubsystem->GetAllWorldStateTags();
        return Tags;
    }

    return Tags;
}

FGameplayTagContainer UFVDialogueContextBuilder::BuildInstigatorTags(AActor* InstigatorActor) const
{
    FGameplayTagContainer Tags;

    if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(InstigatorActor))
    {
        if (const UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
        {
            ASC->GetOwnedGameplayTags(Tags);
        }
    }

    return Tags;
}

FGameplayTagContainer UFVDialogueContextBuilder::BuildTargetTags(AActor* TargetActor) const
{
    FGameplayTagContainer Tags;

    if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetActor))
    {
        if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
        {
            ASC->GetOwnedGameplayTags(Tags);
        }
    }

    return Tags;
}

int32 UFVDialogueContextBuilder::ResolveRelationshipValue(AActor* Instigator, AActor* Target) const
{
    // TODO: Replace with your relationship tracking system.
    // Example:
    //   if (URelationshipSubsystem* RS = Speaker->GetWorld()->GetSubsystem<URelationshipSubsystem>())
    //       return RS->GetRelationship(Speaker, Listener);
    return 0;
}

float UFVDialogueContextBuilder::ResolveNotorietyValue(AActor* Instigator) const
{
    // TODO: Replace with your stats/attribute query.
    // GAS example:
    //   if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Speaker))
    //       return ASI->GetAbilitySystemComponent()->GetNumericAttributeBase(
    //           UMyPlayerAttributeSet::GetNotorietyAttribute());
    return 0.f;
}

TMap<FGameplayTag, EFVFactionStanding> UFVDialogueContextBuilder::BuildFactionStandings(AActor* Instigator) const
{
    TMap<FGameplayTag, EFVFactionStanding> Standings;

    // TODO: Replace with your faction system query.
    // Example:
    //   if (UFactionSubsystem* FS = Speaker->GetWorld()->GetSubsystem<UFactionSubsystem>())
    //       return FS->GetAllStandingsForPlayer(Speaker);

    return Standings;
}

FGameplayTagContainer UFVDialogueContextBuilder::BuildActiveQuestTags(UWorld* World) const
{
    FGameplayTagContainer Tags;

    // TODO: Replace with your quest/narrative system.
    // Example:
    //   if (UQuestSubsystem* QS = World->GetSubsystem<UQuestSubsystem>())
    //       Tags = QS->GetAllActiveAndCompletedQuestTags();

    return Tags;
}