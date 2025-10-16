#include "FreeFlyCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"

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

    ApplyMappingContext();
}

void AFreeFlyCameraPawn::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    UpdatePlayerController();
    ApplyMappingContext();
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

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        /*
         * Configure Enhanced Input assets in the editor:
         *   - Create an Input Mapping Context for the free-fly camera controls.
         *   - Add Input Actions for Move (Vector2), MoveVertical (Axis1D), Look (Vector2),
         *     LookToggle (Boolean), Boost (Boolean), Slow (Boolean), and AdjustSpeed (Axis1D).
         *   - Map the actions as follows: WASD -> Move, QE -> MoveVertical, Mouse X/Y -> Look,
         *     Right Mouse -> LookToggle, Left Shift -> Boost, Left Ctrl -> Slow, Mouse Wheel -> AdjustSpeed.
         *   - Assign those assets to the pawn defaults so they are available at runtime.
         */

        if (MoveAction)
        {
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFreeFlyCameraPawn::HandleMove);
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &AFreeFlyCameraPawn::HandleMove);
        }

        if (MoveVerticalAction)
        {
            EnhancedInput->BindAction(MoveVerticalAction, ETriggerEvent::Triggered, this, &AFreeFlyCameraPawn::HandleMoveUp);
            EnhancedInput->BindAction(MoveVerticalAction, ETriggerEvent::Completed, this, &AFreeFlyCameraPawn::HandleMoveUp);
        }

        if (LookAction)
        {
            EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFreeFlyCameraPawn::HandleLook);
            EnhancedInput->BindAction(LookAction, ETriggerEvent::Completed, this, &AFreeFlyCameraPawn::HandleLook);
        }

        if (LookToggleAction)
        {
            EnhancedInput->BindAction(LookToggleAction, ETriggerEvent::Started, this, &AFreeFlyCameraPawn::HandleLookStarted);
            EnhancedInput->BindAction(LookToggleAction, ETriggerEvent::Completed, this, &AFreeFlyCameraPawn::HandleLookCompleted);
        }

        if (BoostAction)
        {
            EnhancedInput->BindAction(BoostAction, ETriggerEvent::Started, this, &AFreeFlyCameraPawn::HandleBoostStarted);
            EnhancedInput->BindAction(BoostAction, ETriggerEvent::Completed, this, &AFreeFlyCameraPawn::HandleBoostCompleted);
        }

        if (SlowAction)
        {
            EnhancedInput->BindAction(SlowAction, ETriggerEvent::Started, this, &AFreeFlyCameraPawn::HandleSlowStarted);
            EnhancedInput->BindAction(SlowAction, ETriggerEvent::Completed, this, &AFreeFlyCameraPawn::HandleSlowCompleted);
        }

        if (AdjustSpeedAction)
        {
            EnhancedInput->BindAction(AdjustSpeedAction, ETriggerEvent::Triggered, this, &AFreeFlyCameraPawn::HandleAdjustSpeed);
        }
    }
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

void AFreeFlyCameraPawn::ApplyMappingContext()
{
    if (bMappingContextApplied || !DefaultMappingContext)
    {
        return;
    }

    if (!CachedPlayerController)
    {
        UpdatePlayerController();
    }

    if (!CachedPlayerController)
    {
        return;
    }

    if (ULocalPlayer* LocalPlayer = CachedPlayerController->GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
            bMappingContextApplied = true;
        }
    }
}

void AFreeFlyCameraPawn::HandleMove(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();
    MoveForward(MovementVector.Y);
    MoveRight(MovementVector.X);
}

void AFreeFlyCameraPawn::HandleMoveUp(const FInputActionValue& Value)
{
    MoveUp(Value.Get<float>());
}

void AFreeFlyCameraPawn::HandleLook(const FInputActionValue& Value)
{
    if (!bIsLooking)
    {
        return;
    }

    const FVector2D LookVector = Value.Get<FVector2D>();
    Turn(LookVector.X);
    LookUp(LookVector.Y);
}

void AFreeFlyCameraPawn::HandleAdjustSpeed(const FInputActionValue& Value)
{
    AdjustSpeed(Value.Get<float>());
}

void AFreeFlyCameraPawn::HandleLookStarted(const FInputActionValue& Value)
{
    if (Value.Get<bool>())
    {
        StartLook();
    }
}

void AFreeFlyCameraPawn::HandleLookCompleted(const FInputActionValue& /*Value*/)
{
    StopLook();
}

void AFreeFlyCameraPawn::HandleBoostStarted(const FInputActionValue& Value)
{
    if (Value.Get<bool>())
    {
        OnBoostPressed();
    }
}

void AFreeFlyCameraPawn::HandleBoostCompleted(const FInputActionValue& /*Value*/)
{
    OnBoostReleased();
}

void AFreeFlyCameraPawn::HandleSlowStarted(const FInputActionValue& Value)
{
    if (Value.Get<bool>())
    {
        OnSlowPressed();
    }
}

void AFreeFlyCameraPawn::HandleSlowCompleted(const FInputActionValue& /*Value*/)
{
    OnSlowReleased();
}
