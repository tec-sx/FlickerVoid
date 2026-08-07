class UAIConfigDataAsset : UDataAsset
{   
    UPROPERTY(EditDefaultsOnly, Category = "AI Config")
    TSubclassOf<UAISense> DominantSense;
    
    UPROPERTY(EditDefaultsOnly, Instanced, Category = "AI Config")
    TArray<UAISenseConfig> SensesConfig;
}