#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HexCoord.h"
#include "HexGridManager.generated.h"

class AHexTile;

UCLASS(Abstract, Blueprintable)
class THE_ARISTROCAT_API AHexGridManager : public AActor
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "Hex Grid")
    void RegisterTile(const FHexCoord& Coordinate, AHexTile* Tile);
};
