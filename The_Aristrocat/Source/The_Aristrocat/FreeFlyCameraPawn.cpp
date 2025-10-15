#include "FreeFlyCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerController.h"

AFreeFlyCameraPawn::AFreeFlyCameraPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(Root);

    Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
    if (Movement)
    {
        Movement->UpdatedComponent = RootComponent;
        Movement->Acceleration = 20000.f;
        Movement->Deceleration = 20000.f;
        Movement->TurningBoost = 0.f;
    }

    YawAngle = GetActorRotation().Yaw;
    PitchAngle = GetActorRotation().Pitch;
}

void AFreeFlyCameraPawn::BeginPlay()
{
    Super::BeginPlay();

    const FRotator CurrentRotation = GetActorRotation();
    YawAngle = CurrentRotation.Yaw;
    PitchAngle = CurrentRotation.Pitch;

    UpdatePlayerController();

    if (CachedPlayerController)
    {
        CachedPlayerController->bShowMouseCursor = true;
    }
}

void AFreeFlyCameraPawn::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    UpdatePlayerController();
}

void AFreeFlyCameraPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    const float CurrentSpeed = GetCurrentSpeed();

    FVector Direction = (GetActorForwardVector() * ForwardInput) +
        (GetActorRightVector() * RightInput) +
        (FVector::UpVector * UpInput);

    if (!Direction.IsNearlyZero())
    {
        Direction = Direction.GetSafeNormal();
        const FVector MoveDelta = Direction * CurrentSpeed * DeltaSeconds;
        AddActorWorldOffset(MoveDelta, true);

        if (Movement)
        {
            Movement->Velocity = Direction * CurrentSpeed;
            Movement->UpdateComponentVelocity();
        }
    }
    else if (Movement)
    {
        Movement->StopMovementImmediately();
    }

    ApplyRotation();
}

void AFreeFlyCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (!PlayerInputComponent)
    {
        return;
    }

    /*
     * Axis Mappings (Project Settings -> Input):
     *   MoveForward: W=1, S=-1
     *   MoveRight: D=1, A=-1
     *   MoveUp: E=1, Q=-1
     *   Turn: Mouse X
     *   LookUp: Mouse Y
     *   AdjustSpeed: Mouse Wheel Axis
     *
     * Action Mappings:
     *   Look: Right Mouse Button
     *   BoostSpeed: Left Shift
     *   SlowSpeed: Left Control
     */

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AFreeFlyCameraPawn::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AFreeFlyCameraPawn::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("MoveUp"), this, &AFreeFlyCameraPawn::MoveUp);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AFreeFlyCameraPawn::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AFreeFlyCameraPawn::LookUp);
    PlayerInputComponent->BindAxis(TEXT("AdjustSpeed"), this, &AFreeFlyCameraPawn::AdjustSpeed);

    PlayerInputComponent->BindAction(TEXT("Look"), IE_Pressed, this, &AFreeFlyCameraPawn::StartLook);
    PlayerInputComponent->BindAction(TEXT("Look"), IE_Released, this, &AFreeFlyCameraPawn::StopLook);
    PlayerInputComponent->BindAction(TEXT("BoostSpeed"), IE_Pressed, this, &AFreeFlyCameraPawn::OnBoostPressed);
    PlayerInputComponent->BindAction(TEXT("BoostSpeed"), IE_Released, this, &AFreeFlyCameraPawn::OnBoostReleased);
    PlayerInputComponent->BindAction(TEXT("SlowSpeed"), IE_Pressed, this, &AFreeFlyCameraPawn::OnSlowPressed);
    PlayerInputComponent->BindAction(TEXT("SlowSpeed"), IE_Released, this, &AFreeFlyCameraPawn::OnSlowReleased);
}

void AFreeFlyCameraPawn::MoveForward(float Value)
{
    ForwardInput = Value;
}

void AFreeFlyCameraPawn::MoveRight(float Value)
{
    RightInput = Value;
}

void AFreeFlyCameraPawn::MoveUp(float Value)
{
    UpInput = Value;
}

void AFreeFlyCameraPawn::Turn(float Value)
{
    if (!bIsLooking)
    {
        return;
    }

    YawAngle += Value * MouseSensitivity;
}

void AFreeFlyCameraPawn::LookUp(float Value)
{
    if (!bIsLooking)
    {
        return;
    }

    PitchAngle = FMath::Clamp(PitchAngle + Value * MouseSensitivity, -85.f, 85.f);
}

void AFreeFlyCameraPawn::StartLook()
{
    bIsLooking = true;
    UpdatePlayerController();

    if (CachedPlayerController)
    {
        CachedPlayerController->bShowMouseCursor = false;

        FInputModeGameOnly InputMode;
        CachedPlayerController->SetInputMode(InputMode);
    }
}

void AFreeFlyCameraPawn::StopLook()
{
    bIsLooking = false;
    UpdatePlayerController();

    if (CachedPlayerController)
    {
        CachedPlayerController->bShowMouseCursor = true;

        FInputModeGameAndUI InputMode;
        InputMode.SetHideCursorDuringCapture(false);
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        CachedPlayerController->SetInputMode(InputMode);
    }
}

void AFreeFlyCameraPawn::OnBoostPressed()
{
    bBoostPressed = true;
}

void AFreeFlyCameraPawn::OnBoostReleased()
{
    bBoostPressed = false;
}

void AFreeFlyCameraPawn::OnSlowPressed()
{
    bSlowPressed = true;
}

void AFreeFlyCameraPawn::OnSlowReleased()
{
    bSlowPressed = false;
}

void AFreeFlyCameraPawn::AdjustSpeed(float Value)
{
    if (FMath::IsNearlyZero(Value))
    {
        return;
    }

    constexpr float SpeedStep = 100.f;
    BaseSpeed = FMath::Clamp(BaseSpeed + Value * SpeedStep, 200.f, 6000.f);
}

float AFreeFlyCameraPawn::GetCurrentSpeed() const
{
    float Speed = BaseSpeed;

    if (bBoostPressed)
    {
        Speed *= 3.f;
    }

    if (bSlowPressed)
    {
        Speed *= 0.35f;
    }

    return Speed;
}

void AFreeFlyCameraPawn::UpdatePlayerController()
{
    CachedPlayerController = Cast<APlayerController>(GetController());
    if (!CachedPlayerController && GetWorld())
    {
        CachedPlayerController = GetWorld()->GetFirstPlayerController();
    }
}

void AFreeFlyCameraPawn::ApplyRotation()
{
    const FRotator NewRotation(PitchAngle, YawAngle, 0.f);
    SetActorRotation(NewRotation);
}
