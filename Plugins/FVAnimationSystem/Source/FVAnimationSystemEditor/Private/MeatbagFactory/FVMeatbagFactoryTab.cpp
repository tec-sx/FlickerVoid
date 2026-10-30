#include "MeatbagFactory/FVMeatbagFactoryTab.h"

#include "IPersonaToolkit.h"
#include "ISkeletalMeshEditor.h"
#include "ISkeletonEditor.h"
#include "MeatbagFactory/SFVMeatbagFactory.h"
#include "WorkflowOrientedApp/ApplicationMode.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"

#define LOCTEXT_NAMESPACE "FVMeatbagFactoryTab"

namespace
{
	const FName TabId("FVMeatbagFactory");
	FDelegateHandle ExtenderHandle;
	TArray<TWeakPtr<FApplicationMode>> ExtendedModes;

	class FMeatbagFactoryTabSummoner final : public FWorkflowTabFactory
	{
	public:
		explicit FMeatbagFactoryTabSummoner(TSharedPtr<FAssetEditorToolkit> InHostingApp)
			: FWorkflowTabFactory(TabId, InHostingApp)
		{
			bIsSingleton = true;
			TabLabel = LOCTEXT("TabLabel", "Meatbag Factory");
			ViewMenuDescription = LOCTEXT("ViewMenuDescription", "Meatbag Factory");
			ViewMenuTooltip = LOCTEXT("ViewMenuTooltip", "Author and mirror the poses of a Pose Asset on this mesh.");
		}

		virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override
		{
			TSharedPtr<IPersonaToolkit> PersonaToolkit;
			const TSharedPtr<FAssetEditorToolkit> Host = HostingApp.Pin();
			if (Host.IsValid() && Host->GetToolkitFName() == FName("SkeletalMeshEditor"))
			{
				PersonaToolkit = StaticCastSharedPtr<ISkeletalMeshEditor>(Host)->GetPersonaToolkit();
			}
			else if (Host.IsValid() && Host->GetToolkitFName() == FName("SkeletonEditor"))
			{
				PersonaToolkit = StaticCastSharedPtr<ISkeletonEditor>(Host)->GetPersonaToolkit();
			}
			return SNew(SFVMeatbagFactory).PersonaToolkit(PersonaToolkit);
		}
	};

	TSharedRef<FApplicationMode> ExtendMode(const FName ModeName, TSharedRef<FApplicationMode> Mode)
	{
		if (ModeName == FName("SkeletalMeshEditorMode") || ModeName == FName("SkeletonEditorMode"))
		{
			Mode->AddTabFactory(FCreateWorkflowTabFactory::CreateLambda([](TSharedPtr<FAssetEditorToolkit> Host) -> TSharedRef<FWorkflowTabFactory>
			{
				return MakeShared<FMeatbagFactoryTabSummoner>(Host);
			}));
			ExtendedModes.Add(Mode);
		}
		return Mode;
	}
}

void FVMeatbagFactoryTab::Register()
{
	FWorkflowApplicationModeExtender Extender = FWorkflowApplicationModeExtender::CreateStatic(&ExtendMode);
	ExtenderHandle = Extender.GetHandle();
	FWorkflowCentricApplication::GetModeExtenderList().Add(Extender);
}

void FVMeatbagFactoryTab::Unregister()
{
	FWorkflowCentricApplication::GetModeExtenderList().RemoveAll([](const FWorkflowApplicationModeExtender& Extender)
	{
		return Extender.GetHandle() == ExtenderHandle;
	});
	for (const TWeakPtr<FApplicationMode>& WeakMode : ExtendedModes)
	{
		if (const TSharedPtr<FApplicationMode> Mode = WeakMode.Pin())
		{
			Mode->RemoveTabFactory(TabId);
		}
	}
	ExtendedModes.Reset();
}

#undef LOCTEXT_NAMESPACE
