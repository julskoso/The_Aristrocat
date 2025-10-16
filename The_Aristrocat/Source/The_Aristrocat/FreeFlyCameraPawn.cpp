#include "FreeFlyCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/EngineTypes.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
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
    InitializeMappingContext();

    if (CachedPlayerController)
    {
        CachedPlayerController->bShowMouseCursor = true;

        FInputModeGameAndUI InputMode;
        InputMode.SetHideCursorDuringCapture(false);
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        CachedPlayerController->SetInputMode(InputMode);
    }
}

void AFreeFlyCameraPawn::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    UpdatePlayerController();
    InitializeMappingContext();
}

void AFreeFlyCameraPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    const float CurrentSpeed = GetCurrentSpeed();

    FVector Direction = (GetActorForwardVector() * MoveInput.Y) +
        (GetActorRightVector() * MoveInput.X) +
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

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFreeFlyCameraPawn::HandleMove);
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AFreeFlyCameraPawn::HandleMoveCompleted);
        }

        if (UpDownAction)
        {
            EnhancedInputComponent->BindAction(UpDownAction, ETriggerEvent::Triggered, this, &AFreeFlyCameraPawn::HandleVerticalMove);
            EnhancedInputComponent->BindAction(UpDownAction, ETriggerEvent::Completed, this, &AFreeFlyCameraPawn::HandleVerticalMoveCompleted);
        }

        if (LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFreeFlyCameraPawn::HandleLook);
        }

        if (RMBAction)
        {
            EnhancedInputComponent->BindAction(RMBAction, ETriggerEvent::Started, this, &AFreeFlyCameraPawn::StartLook);
            EnhancedInputComponent->BindAction(RMBAction, ETriggerEvent::Completed, this, &AFreeFlyCameraPawn::StopLook);
            EnhancedInputComponent->BindAction(RMBAction, ETriggerEvent::Canceled, this, &AFreeFlyCameraPawn::StopLook);
        }

        if (SprintAction)
        {
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AFreeFlyCameraPawn::StartSprint);
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AFreeFlyCameraPawn::StopSprint);
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AFreeFlyCameraPawn::StopSprint);
        }

        if (SlowAction)
        {
            EnhancedInputComponent->BindAction(SlowAction, ETriggerEvent::Started, this, &AFreeFlyCameraPawn::StartSlow);
            EnhancedInputComponent->BindAction(SlowAction, ETriggerEvent::Completed, this, &AFreeFlyCameraPawn::StopSlow);
            EnhancedInputComponent->BindAction(SlowAction, ETriggerEvent::Canceled, this, &AFreeFlyCameraPawn::StopSlow);
        }

        if (SpeedStepAction)
        {
            EnhancedInputComponent->BindAction(SpeedStepAction, ETriggerEvent::Triggered, this, &AFreeFlyCameraPawn::HandleSpeedStep);
        }
    }
}

void AFreeFlyCameraPawn::HandleMove(const FInputActionValue& Value)
{
    MoveInput = Value.Get<FVector2D>();
}

void AFreeFlyCameraPawn::HandleMoveCompleted(const FInputActionValue& Value)
{
    (void)Value;
    MoveInput = FVector2D::ZeroVector;
}

void AFreeFlyCameraPawn::HandleVerticalMove(const FInputActionValue& Value)
{
    UpInput = Value.Get<float>();
}

void AFreeFlyCameraPawn::HandleVerticalMoveCompleted(const FInputActionValue& Value)
{
    (void)Value;
    UpInput = 0.f;
}

void AFreeFlyCameraPawn::HandleLook(const FInputActionValue& Value)
{
    if (!bIsLooking)
    {
        return;
    }

    const FVector2D LookAxis = Value.Get<FVector2D>();
    YawAngle += LookAxis.X * MouseSensitivity;
    PitchAngle = FMath::Clamp(PitchAngle + LookAxis.Y * MouseSensitivity, -85.f, 85.f);
}

void AFreeFlyCameraPawn::StartLook(const FInputActionValue& Value)
{
    (void)Value;
    bIsLooking = true;
    UpdatePlayerController();

    if (CachedPlayerController)
    {
        CachedPlayerController->bShowMouseCursor = false;

        FInputModeGameOnly InputMode;
        CachedPlayerController->SetInputMode(InputMode);
        CachedPlayerController->SetIgnoreLookInput(false);
        CachedPlayerController->SetIgnoreMoveInput(false);
    }
}

void AFreeFlyCameraPawn::StopLook(const FInputActionValue& Value)
{
    (void)Value;
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

void AFreeFlyCameraPawn::StartSprint(const FInputActionValue& Value)
{
    (void)Value;
    bSprint = true;
}

void AFreeFlyCameraPawn::StopSprint(const FInputActionValue& Value)
{
    (void)Value;
    bSprint = false;
}

void AFreeFlyCameraPawn::StartSlow(const FInputActionValue& Value)
{
    (void)Value;
    bSlow = true;
}

void AFreeFlyCameraPawn::StopSlow(const FInputActionValue& Value)
{
    (void)Value;
    bSlow = false;
}

void AFreeFlyCameraPawn::HandleSpeedStep(const FInputActionValue& Value)
{
    const float Step = Value.Get<float>() * 200.f;
    if (!FMath::IsNearlyZero(Step))
    {
        BaseSpeed = FMath::Clamp(BaseSpeed + Step, MinBaseSpeed, MaxBaseSpeed);
    }
}

float AFreeFlyCameraPawn::GetCurrentSpeed() const
{
    float Speed = BaseSpeed;

    if (bSprint)
    {
        Speed *= SprintMultiplier;
    }

    if (bSlow)
    {
        Speed *= SlowMultiplier;
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

void AFreeFlyCameraPawn::InitializeMappingContext()
{
    if (!FreeFlyMappingContext)
    {
        return;
    }

    UpdatePlayerController();

    if (!CachedPlayerController)
    {
        return;
    }

    if (ULocalPlayer* LocalPlayer = CachedPlayerController->GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
        {
            if (!CachedInputSubsystem.IsValid() || CachedInputSubsystem.Get() != Subsystem || !bMappingContextInitialized)
            {
                Subsystem->AddMappingContext(FreeFlyMappingContext, 0);
                CachedInputSubsystem = Subsystem;
                bMappingContextInitialized = true;
            }
        }
    }
}
