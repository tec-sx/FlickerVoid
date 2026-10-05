#include "FVDialogueSystem.h"

#include "Modules/ModuleManager.h"
#include "NativeGameplayTags.h"

DEFINE_LOG_CATEGORY(LogFVDialogueSystem);

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Dialogue_Mood, "Dialogue.Mood");

IMPLEMENT_MODULE(FDefaultModuleImpl, FVDialogueSystem)
