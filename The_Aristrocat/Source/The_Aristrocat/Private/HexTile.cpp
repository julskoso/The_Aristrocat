#include "HexTile.h"

#include "HexGridManager.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AHexTile::AHexTile()
{
    PrimaryActorTick.bCanEverTick = false;

    HexMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HexMesh"));
    RootComponent = HexMesh;
}

void AHexTile::BeginPlay()
{
    Super::BeginPlay();

    RegisterToGrid();
}

void AHexTile::SetSelected(bool bSelected)
{
    bIsSelected = bSelected;

    if (HexMesh)
    {
        HexMesh->SetRenderCustomDepth(bSelected);

        if (bSelected)
        {
            HexMesh->SetCustomDepthStencilValue(1);
        }
    }
}

void AHexTile::RegisterToGrid()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<AActor*> Managers;
    UGameplayStatics::GetAllActorsOfClass(World, AHexGridManager::StaticClass(), Managers);

    if (Managers.Num() == 0)
    {
        return;
    }

    AHexGridManager* GridManager = Cast<AHexGridManager>(Managers[0]);
    if (!GridManager)
    {
        return;
    }

    GridManager->RegisterTile(FHexCoord(AxialQ, AxialR), this);
}
