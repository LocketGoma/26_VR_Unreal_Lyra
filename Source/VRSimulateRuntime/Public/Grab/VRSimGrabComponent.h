#pragma once

#include "Components/ActorComponent.h"
#include "VRSimGrabComponent.generated.h"

class AActor;
class UVRSimInteractionComponent;

UCLASS(BlueprintType, Blueprintable, ClassGroup = "VR Simulate", meta = (BlueprintSpawnableComponent, ToolTip = "대상 Grab, Release 및 Manipulation의 공통 기반 컴포넌트입니다.", ShortToolTip = "Grab 및 Manipulation 컴포넌트입니다."))
class VRSIMULATERUNTIME_API UVRSimGrabComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRSimGrabComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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
};
