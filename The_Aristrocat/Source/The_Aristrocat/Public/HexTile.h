#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HexCoord.h"
#include "HexTile.generated.h"

class UStaticMeshComponent;

UCLASS(Blueprintable)
class THE_ARISTROCAT_API AHexTile : public AActor
{
    GENERATED_BODY()

public:
    AHexTile();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Hex Tile")
    void SetSelected(bool bSelected);

    UFUNCTION(BlueprintCallable, Category = "Hex Tile")
    void RegisterToGrid();

    UFUNCTION(BlueprintPure, Category = "Hex Tile")
    bool IsSelected() const { return bIsSelected; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> HexMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile", meta = (ExposeOnSpawn = "true"))
    int32 AxialQ = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile", meta = (ExposeOnSpawn = "true"))
    int32 AxialR = 0;

private:
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Hex Tile", meta = (AllowPrivateAccess = "true"))
    bool bIsSelected = false;
};
