#pragma once

#include "CoreMinimal.h"
#include "HexGridTypes.generated.h"

/**
 * Axial coordinates for hex grid addressing.
 */
USTRUCT(BlueprintType)
struct FST_HexCoord
{
    GENERATED_BODY()

public:
    FST_HexCoord()
        : Q(0)
        , R(0)
    {
    }

    FST_HexCoord(int32 InQ, int32 InR)
        : Q(InQ)
        , R(InR)
    {
    }

    /** Axial Q coordinate. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
    int32 Q;

    /** Axial R coordinate. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
    int32 R;
};
