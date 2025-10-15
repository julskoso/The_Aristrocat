#pragma once

#include "CoreMinimal.h"
#include "HexCoord.generated.h"

USTRUCT(BlueprintType)
struct THE_ARISTROCAT_API FHexCoord
{
    GENERATED_BODY()

    FHexCoord() = default;

    FHexCoord(int32 InQ, int32 InR)
        : Q(InQ)
        , R(InR)
    {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
    int32 Q = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
    int32 R = 0;
};
