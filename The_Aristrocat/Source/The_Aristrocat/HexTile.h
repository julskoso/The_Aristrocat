#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HexGridTypes.h"
#include "HexTile.generated.h"

class AHexGridManager;
class UStaticMeshComponent;

/**
 * Base actor for hex tiles that can register to a grid and toggle selection visuals.
 */
UCLASS(Blueprintable)
class THE_ARISTROCAT_API AHexTile : public AActor
{
    GENERATED_BODY()

public:
    AHexTile();

    virtual void BeginPlay() override;

    /** Updates the selection state and outline for the tile. */
    UFUNCTION(BlueprintCallable, Category = "Hex Tile")
    void SetSelected(bool bSelected);

    /** Finds the grid manager and registers this tile. */
    UFUNCTION(BlueprintCallable, Category = "Hex Tile")
    void RegisterToGrid();

    /** Axial Q coordinate, exposed for construction scripts. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile", meta = (ExposeOnSpawn = "true"))
    int32 AxialQ;

    /** Axial R coordinate, exposed for construction scripts. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile", meta = (ExposeOnSpawn = "true"))
    int32 AxialR;

protected:
    /** Static mesh representing the tile. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> HexMesh;

    /** Selection flag replicated in visuals, kept private to blueprints. */
    UPROPERTY(BlueprintReadOnly, Category = "Hex Tile", meta = (AllowPrivateAccess = "true"))
    bool bIsSelected;

    /** Class of the grid manager used when registering. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex Tile", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<AHexGridManager> GridManagerClass;

    /** Custom depth stencil value used for selection outline. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex Tile", meta = (AllowPrivateAccess = "true"))
    int32 SelectedStencilValue;

private:
    void UpdateSelectionVisuals();

    AHexGridManager* FindGridManager() const;
};
