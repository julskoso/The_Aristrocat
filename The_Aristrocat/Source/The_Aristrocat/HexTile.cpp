#include "HexTile.h"

#include "HexGridManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AHexTile::AHexTile()
{
    PrimaryActorTick.bCanEverTick = false;

    HexMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HexMesh"));
    SetRootComponent(HexMesh);
    HexMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    HexMesh->SetGenerateOverlapEvents(false);

    bIsSelected = false;
    SelectedStencilValue = 1;
    GridManagerClass = AHexGridManager::StaticClass();
}

void AHexTile::BeginPlay()
{
    Super::BeginPlay();

    RegisterToGrid();
}

void AHexTile::SetSelected(bool bSelected)
{
    if (bIsSelected == bSelected)
    {
        return;
    }

    bIsSelected = bSelected;
    UpdateSelectionVisuals();
}

void AHexTile::RegisterToGrid()
{
    AHexGridManager* Manager = FindGridManager();
    if (!Manager)
    {
        return;
    }

    Manager->RegisterTile(FST_HexCoord(AxialQ, AxialR), this);
}

void AHexTile::UpdateSelectionVisuals()
{
    if (!HexMesh)
    {
        return;
    }

    HexMesh->SetRenderCustomDepth(bIsSelected);
    if (bIsSelected)
    {
        HexMesh->SetCustomDepthStencilValue(SelectedStencilValue);
    }
}

AHexGridManager* AHexTile::FindGridManager() const
{
    if (!GridManagerClass)
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    TArray<AActor*> Managers;
    UGameplayStatics::GetAllActorsOfClass(World, GridManagerClass, Managers);
    for (AActor* Actor : Managers)
    {
        if (AHexGridManager* Manager = Cast<AHexGridManager>(Actor))
        {
            return Manager;
        }
    }

    return nullptr;
}
