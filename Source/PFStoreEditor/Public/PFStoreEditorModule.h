class FMyPluginModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    //virtual void ShutdownModule() override;

private:
    TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);
};

