#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FreeFlyCameraPawn.generated.h"

class UCameraComponent;
class UFloatingPawnMovement;
class USceneComponent;
class APlayerController;

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
};
