#include "FreeFlyCameraGameMode.h"

#include "FreeFlyCameraPawn.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

AFreeFlyCameraGameMode::AFreeFlyCameraGameMode()
{
    static ConstructorHelpers::FClassFinder<APawn> FreeFlyPawnBP(TEXT("/Game/Blueprints/BP_FreeFlyCameraPawn"));
    if (FreeFlyPawnBP.Succeeded())
    {
        DefaultPawnClass = FreeFlyPawnBP.Class;
    }
    else
    {
        DefaultPawnClass = AFreeFlyCameraPawn::StaticClass();
        UE_LOG(LogTemp, Warning, TEXT("BP_FreeFlyCameraPawn not found. Falling back to AFreeFlyCameraPawn."));
    }

    static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBP(TEXT("/Game/Blueprints/BP_PlayerController_Hex"));
    if (PlayerControllerBP.Succeeded())
    {
        PlayerControllerClass = PlayerControllerBP.Class;
    }
}
