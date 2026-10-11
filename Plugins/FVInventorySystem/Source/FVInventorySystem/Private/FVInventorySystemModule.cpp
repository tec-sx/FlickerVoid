#include "FVInventorySystem.h"

#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogFVInventorySystem);

class FFVInventorySystemModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		// Example categories and slots ship as editable tags; registered explicitly so loading doesn't depend on plugin scanning.
		if (const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("FVInventorySystem")))
		{
			UGameplayTagsManager::Get().AddTagIniSearchPath(FPaths::Combine(Plugin->GetBaseDir(), TEXT("Config"), TEXT("Tags")));
		}
	}
};

IMPLEMENT_MODULE(FFVInventorySystemModule, FVInventorySystem)
