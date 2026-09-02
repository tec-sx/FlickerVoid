#include "FVCoreTags.h"

namespace FVCoreTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Mouse, "InputTag.Look.Mouse", "Look (mouse) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Stick, "InputTag.Look.Stick", "Look (stick) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Crouch, "InputTag.Crouch", "Crouch input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Walk, "InputTag.Walk", "Walk input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Sprint, "InputTag.Sprint", "Sprint input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Jump, "InputTag.Jump", "Jump input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Aim, "InputTag.Aim", "Aim input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interaction, "InputTag.Interaction", "Root of interaction inputs.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interaction_Primary, "InputTag.Interaction.Primary", "Primary interaction.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interaction_Secondary, "InputTag.Interaction.Secondary", "Secondary interaction.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interaction_Ternary, "InputTag.Interaction.Ternary", "Ternary interaction.");
	
	UE_DEFINE_GAMEPLAY_TAG(Player_Pawn, "Player.Pawn");
	
	UE_DEFINE_GAMEPLAY_TAG(Flow_Common_Enable, "Flow.Common.Enable");
	UE_DEFINE_GAMEPLAY_TAG(Flow_Common_Disable, "Flow.Common.Disable");
	UE_DEFINE_GAMEPLAY_TAG(Flow_Common_TriggerZone, "Flow.Common.TriggerZone");
	UE_DEFINE_GAMEPLAY_TAG(Flow_Common_InteractionZone, "Flow.Common.InteractionZone");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact, "Fact", "Root of the fact database. Every fact lives under this tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_System, "Fact.System",
								   "Engine/system level facts, e.g. tutorial completion or session bookkeeping.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_Debug, "Fact.Debug",
								   "Facts used only by cheats and debugging. Never ship gameplay logic reading these.");
	
	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
	{
		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

		if (!Tag.IsValid() && bMatchPartialString)
		{
			FGameplayTagContainer AllTags;
			Manager.RequestAllGameplayTags(AllTags, true);

			for (const FGameplayTag& TestTag : AllTags)
			{
				if (TestTag.ToString().Contains(TagString))
				{
					UE_LOG(LogTemp, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}
}