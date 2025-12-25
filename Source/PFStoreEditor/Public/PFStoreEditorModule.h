// MIT Licensed. Copyright (c) 2025 Olga Taranova

class FMyPluginModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    //virtual void ShutdownModule() override;

private:
    TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);
};

