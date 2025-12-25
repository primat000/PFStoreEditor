// MIT Licensed. Copyright (c) 2025 Olga Taranova

#include "PFStoreModule.h"

#include "Modules/ModuleManager.h"

class FPFStoreModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FPFStoreModule, PFStore)
