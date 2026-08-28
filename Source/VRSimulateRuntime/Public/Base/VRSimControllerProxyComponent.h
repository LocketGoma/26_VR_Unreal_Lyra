#pragma once

#include "Components/SceneComponent.h"
#include "VRSimControllerProxyComponent.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup = "VR Simulate", meta = (BlueprintSpawnableComponent, ToolTip = "VR 컨트롤러의 공간 Pose를 나타내는 프록시 컴포넌트입니다.", ShortToolTip = "VR 컨트롤러 프록시 컴포넌트입니다."))
class VRSIMULATERUNTIME_API UVRSimControllerProxyComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UVRSimControllerProxyComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Pose")
	FVector GetWorldPosition() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Pose")
	FRotator GetWorldRotation() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Pose")
	FVector GetForwardDirection() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Pose")
	FVector GetRayOrigin() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Pose")
	FVector GetNearInteractionOrigin() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Pose")
	FTransform GetGrabAttachTransform() const;

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Debug")
	void SetDebugEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Debug")
	bool IsDebugEnabled() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Pose")
	FVector RayOriginOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Pose")
	FVector NearInteractionOriginOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Pose")
	FTransform GrabAttachOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Debug")
	bool bDebugInteraction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Debug", meta = (ClampMin = "0.0"))
	float DebugDirectionLength = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Debug", meta = (ClampMin = "0.0"))
	float NearInteractionRadius = 20.0f;

private:
	void DrawDebugVisualization() const;
};
