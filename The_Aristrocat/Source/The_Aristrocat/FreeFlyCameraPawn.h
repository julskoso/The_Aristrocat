#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "FreeFlyCameraPawn.generated.h"

class UCameraComponent;
class UFloatingPawnMovement;
class USceneComponent;
class APlayerController;
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
    void ApplyMappingContext();
    void HandleMove(const FInputActionValue& Value);
    void HandleMoveUp(const FInputActionValue& Value);
    void HandleLook(const FInputActionValue& Value);
    void HandleAdjustSpeed(const FInputActionValue& Value);
    void HandleLookStarted(const FInputActionValue& Value);
    void HandleLookCompleted(const FInputActionValue& Value);
    void HandleBoostStarted(const FInputActionValue& Value);
    void HandleBoostCompleted(const FInputActionValue& Value);
    void HandleSlowStarted(const FInputActionValue& Value);
    void HandleSlowCompleted(const FInputActionValue& Value);

    void MoveForward(float Value);
    void MoveRight(float Value);
    void MoveUp(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void StartLook();
    void StopLook();
    void OnBoostPressed();
    void OnBoostReleased();
    void OnSlowPressed();
    void OnSlowReleased();
    void AdjustSpeed(float Value);

    float GetCurrentSpeed() const;
    void UpdatePlayerController();
    void ApplyRotation();

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FreeFly|Camera")
    float MouseSensitivity = 1.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> MoveVerticalAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> LookToggleAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> BoostAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> SlowAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeFly|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> AdjustSpeedAction;

private:
    float ForwardInput = 0.f;
    float RightInput = 0.f;
    float UpInput = 0.f;
    bool bBoostPressed = false;
    bool bSlowPressed = false;
    bool bIsLooking = false;
    float YawAngle = 0.f;
    float PitchAngle = 0.f;

    UPROPERTY()
    APlayerController* CachedPlayerController = nullptr;

    bool bMappingContextApplied = false;
};
