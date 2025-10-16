#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "FreeFlyCameraPawn.generated.h"

class UCameraComponent;
class UFloatingPawnMovement;
class USceneComponent;
class APlayerController;
class UEnhancedInputComponent;
class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UInputMappingContext;

UCLASS(Blueprintable)
class THE_ARISTROCAT_API AFreeFlyCameraPawn : public APawn
{
    GENERATED_BODY()

public:
    AFreeFlyCameraPawn();

    virtual void Tick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
    void HandleMove(const FInputActionValue& Value);
    void HandleMoveCompleted(const FInputActionValue& Value);
    void HandleVerticalMove(const FInputActionValue& Value);
    void HandleVerticalMoveCompleted(const FInputActionValue& Value);
    void HandleLook(const FInputActionValue& Value);
    void StartLook(const FInputActionValue& Value);
    void StopLook(const FInputActionValue& Value);
    void StartSprint(const FInputActionValue& Value);
    void StopSprint(const FInputActionValue& Value);
    void StartSlow(const FInputActionValue& Value);
    void StopSlow(const FInputActionValue& Value);
    void HandleSpeedStep(const FInputActionValue& Value);

    float GetCurrentSpeed() const;
    void UpdatePlayerController();
    void ApplyRotation();
    void InitializeMappingContext();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UFloatingPawnMovement* Movement;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FreeFly|Movement", meta = (ClampMin = "200.0", ClampMax = "6000.0"))
    float BaseSpeed = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FreeFly|Movement")
    float SprintMultiplier = 3.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FreeFly|Movement")
    float SlowMultiplier = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FreeFly|Movement")
    float MinBaseSpeed = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FreeFly|Movement")
    float MaxBaseSpeed = 6000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FreeFly|Camera")
    float MouseSensitivity = 1.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input")
    TObjectPtr<UInputMappingContext> FreeFlyMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input")
    TObjectPtr<UInputAction> UpDownAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input")
    TObjectPtr<UInputAction> RMBAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input")
    TObjectPtr<UInputAction> SprintAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input")
    TObjectPtr<UInputAction> SlowAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input")
    TObjectPtr<UInputAction> SpeedStepAction;

private:
    FVector2D MoveInput = FVector2D::ZeroVector;
    float UpInput = 0.f;
    bool bSprint = false;
    bool bSlow = false;
    bool bIsLooking = false;
    float YawAngle = 0.f;
    float PitchAngle = 0.f;
    bool bMappingContextInitialized = false;

    TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> CachedInputSubsystem;

    UPROPERTY()
    APlayerController* CachedPlayerController = nullptr;
};
