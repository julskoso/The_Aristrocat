#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HexGridTypes.h"
#include "HexGridManager.generated.h"

class AHexTile;

/**
 * Base manager actor responsible for tracking hex tiles on the grid.
 */
UCLASS(Abstract, Blueprintable)
class THE_ARISTROCAT_API AHexGridManager : public AActor
{
    GENERATED_BODY()

public:
    AHexGridManager();

    /** Registers a tile at the given axial coordinate. Blueprint should implement storage logic. */
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Hex Grid")
    void RegisterTile(const FST_HexCoord& Coord, AHexTile* Tile);
};
