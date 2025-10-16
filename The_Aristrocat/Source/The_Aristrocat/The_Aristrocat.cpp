// Copyright notice placeholder.

#include "The_Aristrocat.h"
#include "Modules/ModuleManager.h"

#include "FreeFlyCameraPawn.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "HAL/FileManager.h"
#include "InputCoreTypes.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#endif // WITH_EDITOR

namespace
{
#if WITH_EDITOR
constexpr const TCHAR* FreeFlyInputFolder = TEXT("/Game/Input");

FString BuildPackagePath(const FString& AssetName)
{
    return FString::Printf(TEXT("%s/%s"), FreeFlyInputFolder, *AssetName);
}

FString BuildObjectPath(const FString& AssetName)
{
    const FString PackagePath = BuildPackagePath(AssetName);
    return FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName);
}

bool SaveAssetToDisk(UObject* Asset)
{
    if (!Asset)
    {
        return false;
    }

    UPackage* Package = Asset->GetOutermost();
    if (!Package)
    {
        return false;
    }

    const FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(PackageFileName), true);

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.Error = GWarn;
    SaveArgs.SaveFlags = SAVE_None;

    return UPackage::SavePackage(Package, Asset, *PackageFileName, SaveArgs);
}

UInputAction* LoadOrCreateInputAction(const FString& AssetName, EInputActionValueType ValueType, bool& bOutCreatedOrUpdated)
{
    bOutCreatedOrUpdated = false;

    const FString ObjectPath = BuildObjectPath(AssetName);
    if (UInputAction* ExistingAction = LoadObject<UInputAction>(nullptr, *ObjectPath))
    {
        if (ExistingAction->ValueType != ValueType)
        {
            ExistingAction->Modify();
            ExistingAction->ValueType = ValueType;
            ExistingAction->MarkPackageDirty();
            SaveAssetToDisk(ExistingAction);
            bOutCreatedOrUpdated = true;
        }

        return ExistingAction;
    }

    const FString PackagePath = BuildPackagePath(AssetName);
    UPackage* Package = CreatePackage(*PackagePath);
    if (!ensure(Package))
    {
        return nullptr;
    }

    Package->FullyLoad();

    UInputAction* NewAction = NewObject<UInputAction>(Package, *AssetName, RF_Public | RF_Standalone | RF_Transactional);
    if (!ensure(NewAction))
    {
        return nullptr;
    }

    NewAction->ValueType = ValueType;
    NewAction->MarkPackageDirty();
    Package->MarkPackageDirty();

    FAssetRegistryModule::AssetCreated(NewAction);
    SaveAssetToDisk(NewAction);

    bOutCreatedOrUpdated = true;
    return NewAction;
}

struct FMappingSpec
{
    UInputAction* Action = nullptr;
    FKey Key;
    FVector ValueScale = FVector::ZeroVector;
};

bool MappingMatchesSpec(const FEnhancedActionKeyMapping& Mapping, const FMappingSpec& Spec)
{
    if (Mapping.Action != Spec.Action || Mapping.Key != Spec.Key)
    {
        return false;
    }

    return Mapping.ValueScale.Equals(Spec.ValueScale, KINDA_SMALL_NUMBER);
}

bool ApplyMappingSpecs(UInputMappingContext& MappingContext, const TArray<FMappingSpec>& DesiredMappings)
{
    bool bNeedsUpdate = MappingContext.Mappings.Num() != DesiredMappings.Num();

    if (!bNeedsUpdate)
    {
        for (const FMappingSpec& Spec : DesiredMappings)
        {
            const bool bFound = MappingContext.Mappings.ContainsByPredicate([&Spec](const FEnhancedActionKeyMapping& Existing)
            {
                return MappingMatchesSpec(Existing, Spec);
            });

            if (!bFound)
            {
                bNeedsUpdate = true;
                break;
            }
        }
    }

    if (!bNeedsUpdate)
    {
        return false;
    }

    MappingContext.Modify();
    MappingContext.Mappings.Reset();

    for (const FMappingSpec& Spec : DesiredMappings)
    {
        if (!Spec.Action)
        {
            continue;
        }

        FEnhancedActionKeyMapping& KeyMapping = MappingContext.MapKey(Spec.Action, Spec.Key);
        KeyMapping.ValueScale = Spec.ValueScale;
    }

    MappingContext.MarkPackageDirty();
    return true;
}

void EnsureFreeFlyInputAssets()
{
    if (IsRunningCommandlet())
    {
        return;
    }

    bool bActionCreatedOrUpdated = false;
    bool bMappingCreatedOrUpdated = false;

    struct FActionInfo
    {
        const TCHAR* Name;
        EInputActionValueType Type;
    };

    const TArray<FActionInfo> ActionInfos = {
        { TEXT("IA_Move"), EInputActionValueType::Axis2D },
        { TEXT("IA_UpDown"), EInputActionValueType::Axis1D },
        { TEXT("IA_Look"), EInputActionValueType::Axis2D },
        { TEXT("IA_RMB"), EInputActionValueType::Boolean },
        { TEXT("IA_Sprint"), EInputActionValueType::Boolean },
        { TEXT("IA_Slow"), EInputActionValueType::Boolean },
        { TEXT("IA_SpeedStep"), EInputActionValueType::Axis1D },
    };

    TMap<FName, UInputAction*> CreatedActions;
    CreatedActions.Reserve(ActionInfos.Num());

    for (const FActionInfo& Info : ActionInfos)
    {
        bool bCreatedOrUpdated = false;
        if (UInputAction* Action = LoadOrCreateInputAction(Info.Name, Info.Type, bCreatedOrUpdated))
        {
            CreatedActions.Emplace(FName(Info.Name), Action);
            bActionCreatedOrUpdated |= bCreatedOrUpdated;
        }
    }

    const FString MappingName = TEXT("IMC_FreeFly");
    const FString MappingObjectPath = BuildObjectPath(MappingName);
    UInputMappingContext* MappingContext = LoadObject<UInputMappingContext>(nullptr, *MappingObjectPath);
    bool bMappingWasCreated = false;

    if (!MappingContext)
    {
        const FString PackagePath = BuildPackagePath(MappingName);
        UPackage* Package = CreatePackage(*PackagePath);
        if (!ensure(Package))
        {
            return;
        }

        Package->FullyLoad();

        MappingContext = NewObject<UInputMappingContext>(Package, *MappingName, RF_Public | RF_Standalone | RF_Transactional);
        if (!ensure(MappingContext))
        {
            return;
        }

        MappingContext->Priority = 0;
        MappingContext->MarkPackageDirty();
        Package->MarkPackageDirty();
        FAssetRegistryModule::AssetCreated(MappingContext);
        bMappingWasCreated = true;
        bMappingCreatedOrUpdated = true;
    }

    UInputAction* const MoveAction = CreatedActions.FindRef(TEXT("IA_Move"));
    UInputAction* const UpDownAction = CreatedActions.FindRef(TEXT("IA_UpDown"));
    UInputAction* const LookAction = CreatedActions.FindRef(TEXT("IA_Look"));
    UInputAction* const RMBAction = CreatedActions.FindRef(TEXT("IA_RMB"));
    UInputAction* const SprintAction = CreatedActions.FindRef(TEXT("IA_Sprint"));
    UInputAction* const SlowAction = CreatedActions.FindRef(TEXT("IA_Slow"));
    UInputAction* const SpeedStepAction = CreatedActions.FindRef(TEXT("IA_SpeedStep"));

    const TArray<FMappingSpec> DesiredMappings = {
        { MoveAction, EKeys::W, FVector(0.f, 1.f, 0.f) },
        { MoveAction, EKeys::S, FVector(0.f, -1.f, 0.f) },
        { MoveAction, EKeys::D, FVector(1.f, 0.f, 0.f) },
        { MoveAction, EKeys::A, FVector(-1.f, 0.f, 0.f) },
        { UpDownAction, EKeys::E, FVector(1.f, 0.f, 0.f) },
        { UpDownAction, EKeys::Q, FVector(-1.f, 0.f, 0.f) },
        { LookAction, EKeys::MouseX, FVector(1.f, 0.f, 0.f) },
        { LookAction, EKeys::MouseY, FVector(0.f, 1.f, 0.f) },
        { RMBAction, EKeys::RightMouseButton, FVector::OneVector },
        { SprintAction, EKeys::LeftShift, FVector::OneVector },
        { SlowAction, EKeys::LeftControl, FVector::OneVector },
        { SpeedStepAction, EKeys::MouseScrollUp, FVector(1.f, 0.f, 0.f) },
        { SpeedStepAction, EKeys::MouseScrollDown, FVector(-1.f, 0.f, 0.f) },
    };

    if (MappingContext)
    {
        if (MappingContext->Priority != 0)
        {
            MappingContext->Modify();
            MappingContext->Priority = 0;
            MappingContext->MarkPackageDirty();
            bMappingCreatedOrUpdated = true;
        }

        if (ApplyMappingSpecs(*MappingContext, DesiredMappings))
        {
            bMappingCreatedOrUpdated = true;
        }

        if (bMappingWasCreated || bMappingCreatedOrUpdated)
        {
            SaveAssetToDisk(MappingContext);
        }
    }

    AFreeFlyCameraPawn::InitializeDefaultInputAssets(MappingContext, MoveAction, UpDownAction, LookAction, RMBAction, SprintAction, SlowAction, SpeedStepAction);

    if (bActionCreatedOrUpdated || bMappingCreatedOrUpdated)
    {
        UE_LOG(LogTemp, Log, TEXT("Ensured free-fly input assets were created or updated."));
    }
}
#endif // WITH_EDITOR
}

class FThe_AristrocatModule : public FDefaultGameModuleImpl
{
public:
    virtual void StartupModule() override
    {
        FDefaultGameModuleImpl::StartupModule();

#if WITH_EDITOR
        if (GIsEditor)
        {
            EnsureFreeFlyInputAssets();
        }
#endif // WITH_EDITOR
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FThe_AristrocatModule, The_Aristrocat, "The_Aristrocat");
