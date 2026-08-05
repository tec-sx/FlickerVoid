#include "FVAssetManager.h"
#include "FVGameData.h"
#include "AbilitySystemGlobals.h"
#include "Character/FVPawnData.h"
#include "Configuration/FlickerVoidDeveloperSettings.h"
#include "Misc/App.h"
#include "Stats/StatsMisc.h"
#include "Engine/Engine.h"
#include "Misc/ScopedSlowTask.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAssetManager)

const FName FFVBundles::Equipped("Equipped");

//////////////////////////////////////////////////////////////////////

static FAutoConsoleCommand CVarDumpLoadedAssets(
	TEXT("FV.DumpLoadedAssets"),
	TEXT("Shows all assets that were loaded via the asset manager and are currently in memory."),
	FConsoleCommandDelegate::CreateStatic(UFVAssetManager::DumpLoadedAssets)
);

//////////////////////////////////////////////////////////////////////

#define STARTUP_JOB_WEIGHTED(JobFunc, JobWeight) StartupJobs.Add(FFVAssetManagerStartupJob(#JobFunc, [this](const FFVAssetManagerStartupJob& StartupJob, TSharedPtr<FStreamableHandle>& LoadHandle){JobFunc;}, JobWeight))
#define STARTUP_JOB(JobFunc) STARTUP_JOB_WEIGHTED(JobFunc, 1.f)

//////////////////////////////////////////////////////////////////////

UFVAssetManager::UFVAssetManager()
{
	DefaultPawnData = nullptr;
}

UFVAssetManager& UFVAssetManager::Get()
{
	check(GEngine);

	if (UFVAssetManager* Singleton = Cast<UFVAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}

	UE_LOG(LogTemp, Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini.  It must be set to LyraAssetManager!"));

	// Fatal error above prevents this from being called.
	return *NewObject<UFVAssetManager>();
}

void UFVAssetManager::DumpLoadedAssets()
{
	UE_LOG(LogTemp, Log, TEXT("========== Start Dumping Loaded Assets =========="));

	for (const UObject* LoadedAsset : Get().LoadedAssets)
	{
		UE_LOG(LogTemp, Log, TEXT("  %s"), *GetNameSafe(LoadedAsset));
	}

	UE_LOG(LogTemp, Log, TEXT("... %d assets in loaded pool"), Get().LoadedAssets.Num());
	UE_LOG(LogTemp, Log, TEXT("========== Finish Dumping Loaded Assets =========="));
}

const UFVGameData& UFVAssetManager::GetGameData()
{
	return GetOrLoadTypedGameData<UFVGameData>(FVGameDataPath);
}

const UFVPawnData* UFVAssetManager::GetDefaultPawnData() const
{
	return GetAsset(DefaultPawnData);
}

UObject* UFVAssetManager::SynchronousLoadAsset(const FSoftObjectPath& AssetPath)
{
	if (AssetPath.IsValid())
	{
		TUniquePtr<FScopeLogTime> LogTimePtr;

		if (ShouldLogAssetLoads())
		{
			LogTimePtr = MakeUnique<FScopeLogTime>(*FString::Printf(TEXT("Synchronously loaded asset [%s]"), *AssetPath.ToString()), nullptr, FScopeLogTime::ScopeLog_Seconds);
		}

		if (UAssetManager::IsInitialized())
		{
			return UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false);
		}

		// Use LoadObject if asset manager isn't ready yet.
		return AssetPath.TryLoad();
	}

	return nullptr;
}

bool UFVAssetManager::ShouldLogAssetLoads()
{
	static bool bLogAssetLoads = FParse::Param(FCommandLine::Get(), TEXT("LogAssetLoads"));
	return bLogAssetLoads;
}

void UFVAssetManager::AddLoadedAsset(const UObject* Asset)
{
	if (ensureAlways(Asset))
	{
		FScopeLock LoadedAssetsLock(&LoadedAssetsCritical);
		LoadedAssets.Add(Asset);
	}
}

void UFVAssetManager::StartInitialLoading()
{
	SCOPED_BOOT_TIMING("ULyraAssetManager::StartInitialLoading");

	// This does all of the scanning, need to do this now even if loads are deferred
	Super::StartInitialLoading();

	STARTUP_JOB(InitializeGameplayCueManager());

	{
		// Load base game data asset
		STARTUP_JOB_WEIGHTED(GetGameData(), 25.f);
	}

	// Run all the queued up startup jobs
	DoAllStartupJobs();
}

#if WITH_EDITOR
void UFVAssetManager::PreBeginPIE(bool bStartSimulate)
{
	Super::PreBeginPIE(bStartSimulate);

	{
		FScopedSlowTask SlowTask(0, NSLOCTEXT("FVEditor", "BeginLoadingPIEData", "Loading PIE Data"));
		const bool bShowCancelButton = false;
		const bool bAllowInPIE = true;
		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);

		const UFVGameData& LocalGameDataCommon = GetGameData();

		// Intentionally after GetGameData to avoid counting GameData time in this timer
		SCOPE_LOG_TIME_IN_SECONDS(TEXT("PreBeginPIE asset preloading complete"), nullptr);

	}
}
#endif

UPrimaryDataAsset* UFVAssetManager::LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass, const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath, FPrimaryAssetType PrimaryAssetType)
{
	UPrimaryDataAsset* Asset = nullptr;

	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Loading GameData Object"), STAT_GameData, STATGROUP_LoadTime);
	if (!DataClassPath.IsNull())
	{
#if WITH_EDITOR
		FScopedSlowTask SlowTask(0, FText::Format(NSLOCTEXT("FlickerVoidEditor", "BeginLoadingGameDataTask", "Loading GameData {0}"), FText::FromName(DataClass->GetFName())));
		const bool bShowCancelButton = false;
		const bool bAllowInPIE = true;
		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);
#endif
		UE_LOG(LogTemp, Log, TEXT("Loading GameData: %s ..."), *DataClassPath.ToString());
		SCOPE_LOG_TIME_IN_SECONDS(TEXT("    ... GameData loaded!"), nullptr);

		// This can be called recursively in the editor because it is called on demand from PostLoad so force a sync load for primary asset and async load the rest in that case
		if (GIsEditor)
		{
			Asset = DataClassPath.LoadSynchronous();
			LoadPrimaryAssetsWithType(PrimaryAssetType);
		}
		else
		{
			TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(PrimaryAssetType);
			if (Handle.IsValid())
			{
				Handle->WaitUntilComplete(0.0f, false);

				// This should always work
				Asset = Cast<UPrimaryDataAsset>(Handle->GetLoadedAsset());
			}
		}
	}

	if (Asset)
	{
		GameDataMap.Add(DataClass, Asset);
	}
	else
	{
		// It is not acceptable to fail to load any GameData asset. It will result in soft failures that are hard to diagnose.
		UE_LOG(LogTemp, Fatal, TEXT("Failed to load GameData asset at %s. Type %s. This is not recoverable and likely means you do not have the correct data to run %s."), *DataClassPath.ToString(), *PrimaryAssetType.ToString(), FApp::GetProjectName());
	}

	return Asset;
}

void UFVAssetManager::AsyncLoadSoftPath(
	UObject* WorldContext, 
	FSoftObjectPath Path, 
	UObject* Payload,
	TFunction<void(UObject*, UObject*)> OnComplete)
{
    if (!Path.IsValid())
    {
        OnComplete(nullptr, Payload);
        return;
    }
	
    if (UObject* AlreadyLoaded = Path.ResolveObject())
    {
        if (WorldContext && WorldContext->GetWorld())
        {
            FTimerHandle DeferHandle;
            WorldContext->GetWorld()->GetTimerManager().SetTimer(
                DeferHandle,
                [OnComplete, AlreadyLoaded, Payload]()
                {
                    OnComplete(AlreadyLoaded, Payload);
                },
                0.001f,
                false);
        }
        else
        {
            OnComplete(AlreadyLoaded, Payload);
        }
        return;
    }
	
    FStreamableManager& Manager = GetStreamableManager();
	
    TSharedPtr<FStreamableHandle> Handle = Manager.RequestAsyncLoad(
        Path,
        [Path, Payload, OnComplete]()
        {
            UObject* Loaded = Path.ResolveObject();
            OnComplete(Loaded, Payload);   // Loaded may be null if cook failed
        });
}

void UFVAssetManager::AsyncLoadSound(
    UObject* WorldContext,
    TSoftObjectPtr<USoundBase> SoftAsset,
    UObject* Payload,
    FOnSoundAssetLoaded OnLoaded)
{
    AsyncLoadSoftPath(WorldContext, SoftAsset.ToSoftObjectPath(), Payload,
        [OnLoaded](UObject* Loaded, UObject* P)
        {
            OnLoaded.ExecuteIfBound(Cast<USoundBase>(Loaded), P);
        });
}
 
void UFVAssetManager::AsyncLoadSkeletalMesh(
    UObject* WorldContext,
    TSoftObjectPtr<USkeletalMesh> SoftAsset,
    UObject* Payload,
    FOnSkeletalMeshAssetLoaded OnLoaded)
{
    AsyncLoadSoftPath(WorldContext, SoftAsset.ToSoftObjectPath(), Payload,
        [OnLoaded](UObject* Loaded, UObject* P)
        {
            OnLoaded.ExecuteIfBound(Cast<USkeletalMesh>(Loaded), P);
        });
}
 
void UFVAssetManager::AsyncLoadDataTable(
    UObject* WorldContext,
    TSoftObjectPtr<UDataTable> SoftAsset,
    UObject* Payload,
    FOnDataTableAssetLoaded OnLoaded)
{
    AsyncLoadSoftPath(WorldContext, SoftAsset.ToSoftObjectPath(), Payload,
        [OnLoaded](UObject* Loaded, UObject* P)
        {
            OnLoaded.ExecuteIfBound(Cast<UDataTable>(Loaded), P);
        });
}
 
void UFVAssetManager::AsyncLoadBlueprintClass(
    UObject* WorldContext,
    TSoftClassPtr<UObject> SoftClass,
    UObject* Payload,
    FOnBlueprintClassLoaded OnLoaded)
{
    // TSoftClassPtr uses a different path accessor than TSoftObjectPtr
    AsyncLoadSoftPath(WorldContext, SoftClass.ToSoftObjectPath(), Payload,
        [OnLoaded](UObject* Loaded, UObject* P)
        {
            // ResolveObject on a class path returns the UClass object itself
            UClass* LoadedClass = Cast<UClass>(Loaded);
            OnLoaded.ExecuteIfBound(LoadedClass, P);
        });
}
 
void UFVAssetManager::AsyncLoadObject(
    UObject* WorldContext,
    TSoftObjectPtr<UObject> SoftAsset,
    UObject* Payload,
    FOnObjectAssetLoaded OnLoaded)
{
    AsyncLoadSoftPath(WorldContext, SoftAsset.ToSoftObjectPath(), Payload,
        [OnLoaded](UObject* Loaded, UObject* P)
        {
            OnLoaded.ExecuteIfBound(Loaded, P);
        });
}

void UFVAssetManager::DoAllStartupJobs()
{
	SCOPED_BOOT_TIMING("ULyraAssetManager::DoAllStartupJobs");
	const double AllStartupJobsStartTime = FPlatformTime::Seconds();

	if (IsRunningDedicatedServer())
	{
		// No need for periodic progress updates, just run the jobs
		for (const FFVAssetManagerStartupJob& StartupJob : StartupJobs)
		{
			StartupJob.DoJob();
		}
	}
	else
	{
		if (StartupJobs.Num() > 0)
		{
			float TotalJobValue = 0.0f;
			for (const FFVAssetManagerStartupJob& StartupJob : StartupJobs)
			{
				TotalJobValue += StartupJob.JobWeight;
			}

			float AccumulatedJobValue = 0.0f;
			for (FFVAssetManagerStartupJob& StartupJob : StartupJobs)
			{
				const float JobValue = StartupJob.JobWeight;
				StartupJob.SubstepProgressDelegate.BindLambda([This = this, AccumulatedJobValue, JobValue, TotalJobValue](float NewProgress)
					{
						const float SubstepAdjustment = FMath::Clamp(NewProgress, 0.0f, 1.0f) * JobValue;
						const float OverallPercentWithSubstep = (AccumulatedJobValue + SubstepAdjustment) / TotalJobValue;

						This->UpdateInitialGameContentLoadPercent(OverallPercentWithSubstep);
					});

				StartupJob.DoJob();

				StartupJob.SubstepProgressDelegate.Unbind();

				AccumulatedJobValue += JobValue;

				UpdateInitialGameContentLoadPercent(AccumulatedJobValue / TotalJobValue);
			}
		}
		else
		{
			UpdateInitialGameContentLoadPercent(1.0f);
		}
	}

	StartupJobs.Empty();

	UE_LOG(LogTemp, Display, TEXT("All startup jobs took %.2f seconds to complete"), FPlatformTime::Seconds() - AllStartupJobsStartTime);
}

void UFVAssetManager::InitializeGameplayCueManager()
{
	//SCOPED_BOOT_TIMING("ULyraAssetManager::InitializeGameplayCueManager");

	//ULyraGameplayCueManager* GCM = ULyraGameplayCueManager::Get();
	//check(GCM);
	//GCM->LoadAlwaysLoadedCues();
}

void UFVAssetManager::UpdateInitialGameContentLoadPercent(float GameContentPercent)
{
	// Could route this to the early startup loading screen
}
