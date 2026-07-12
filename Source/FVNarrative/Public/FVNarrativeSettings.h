#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DataAsset.h"
#include "FVNarrativeSettings.generated.h"

class UFVDialogueContextBuilder;

UCLASS(Config=Game, DefaultConfig, meta = (DisplayName = "Narrative"))
class FLICKERVOIDNARRATIVE_API UFVNarrativeSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    // ── Data Tables ───────────────────────────────────────────────────────────

    /**
     * Global response table (DT_DialogueResponses).
     * All player response rows across every NPC live here.
     *
     * Hard reference — intentionally loaded and resident at all times.
     * Response rows are small (text + tags) and looked up constantly during
     * active sessions, so keeping the table hot is the right tradeoff.
     */
    UPROPERTY(
        Config,
        EditDefaultsOnly, 
        Category = Dialogue,
        meta = (RequiredAssetDataTags = "RowStructure=/Script/FlickerVoid.FVDialogueResponseRow"))
    TSoftObjectPtr<UDataTable> ResponseTable;

    /**
     * Optional: global/ambient line tables shared across all NPCs.
     * NPC-specific line tables are registered per-NPC at spawn time via
     * UDialogueDatabaseSubsystem::RegisterLineTable — these are the tables
     * that should be pre-registered before any NPC spawns (e.g. generic
     * ambient lines, system messages, tutorial prompts).
     */
    UPROPERTY(
        Config, 
        EditDefaultsOnly, 
        Category = Dialogue, 
        meta = (RequiredAssetDataTags = "RowStructure=/Script/FlickerVoid.FVDialogueLineRow"))
    TArray<TSoftObjectPtr<UDataTable>> GlobalLineTables;

    // ── Context Builder ───────────────────────────────────────────────────────

    /**
     * The context builder subclass to instantiate.
     * Default: UDialogueContextBuilder (stub implementations).
     * Set to your project-specific subclass that integrates with
     * your relationship, faction, and quest systems.
     */
    UPROPERTY(Config, EditDefaultsOnly, Category = Dialogue)
    TSoftClassPtr<UFVDialogueContextBuilder> ContextBuilderClass;

    // ── Timing Defaults ───────────────────────────────────────────────────────

    /**
     * Default auto-dismiss delay for lines with no voice audio and no
     * response options. Widgets can override this per-instance.
     */
    UPROPERTY(Config, EditDefaultsOnly, Category = Dialogue, meta = (ClampMin = "0.5", UIMin = "0.5"))
    float DefaultAutoDismissSeconds = 3.5f;

    /**
     * Default typewriter characters per second.
     * Set to 0 to disable typewriter effect globally.
     */
    UPROPERTY(Config, EditDefaultsOnly,  Category = Dialogue, meta = (ClampMin = "0.0"))
    float DefaultTypewriterCharsPerSecond = 40.f;
};