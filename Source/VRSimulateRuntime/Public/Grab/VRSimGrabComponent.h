#pragma once

#include "Components/ActorComponent.h"
#include "VRSimGrabComponent.generated.h"

class AActor;
class UVRSimInteractionComponent;
class UPrimitiveComponent;

UCLASS(BlueprintType, Blueprintable, ClassGroup = "VR Simulate", meta = (BlueprintSpawnableComponent, ToolTip = "대상 Grab, Release 및 Manipulation의 공통 기반 컴포넌트입니다.", ShortToolTip = "Grab 및 Manipulation 컴포넌트입니다."))
class VRSIMULATERUNTIME_API UVRSimGrabComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRSimGrabComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Grab")
	void SetInteractionComponent(UVRSimInteractionComponent* InInteractionComponent);

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Grab")
	UVRSimInteractionComponent* GetInteractionComponent() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Grab")
	AActor* GetCandidateTarget() const;

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Grab")
	bool SetGrabbedTarget(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Grab")
	void ClearGrabbedTarget();

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Grab")
	AActor* GetGrabbedTarget() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Grab")
	bool IsGrabbing() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Grab")
	FTransform GetGrabAttachTransform() const;

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Grab")
	void AdjustHoldDistance(float Steps);
	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Grab")
	void ResetHoldDistance();
	// Distances in centimeters; DistanceStep is centimeters per input unit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR Simulate|Grab", meta=(ClampMin="0"))
	float DefaultHoldDistance = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR Simulate|Grab", meta=(ClampMin="0"))
	float MinHoldDistance = 90.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR Simulate|Grab", meta=(ClampMin="0"))
	float MaxHoldDistance = 330.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR Simulate|Grab", meta=(ClampMin="0"))
	float DistanceStep = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR Simulate|Grab", meta=(ClampMin="0"))
	float FollowSpeed = 14.f;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Simulate|Grab")
	bool TryGrab();
	virtual bool TryGrab_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Simulate|Grab")
	bool TryRelease();
	virtual bool TryRelease_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "VR Simulate|Grab")
	void UpdateManipulation(float DeltaTime);
	virtual void UpdateManipulation_Implementation(float DeltaTime);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Grab")
	TObjectPtr<UVRSimInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "VR Simulate|Grab")
	TObjectPtr<AActor> GrabbedTarget;

private:
	UFUNCTION() void HandleTargetDestroyed(AActor* Target);
	UPROPERTY(Transient) TWeakObjectPtr<UPrimitiveComponent> GrabbedPrimitive;
	float HoldDistance = 200.f;
	bool bRestoreSimulation = false;
};
