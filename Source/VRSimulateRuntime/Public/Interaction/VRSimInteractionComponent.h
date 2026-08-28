#pragma once

#include "Components/ActorComponent.h"
#include "VRSimInteractionComponent.generated.h"

class AActor;
class UVRSimControllerProxyComponent;

UCLASS(BlueprintType, Blueprintable, ClassGroup = "VR Simulate", meta = (BlueprintSpawnableComponent, ToolTip = "대상 감지, 선택 및 상호작용의 공통 기반 컴포넌트입니다.", ShortToolTip = "공통 Interaction 컴포넌트입니다."))
class VRSIMULATERUNTIME_API UVRSimInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRSimInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Interaction")
	void SetControllerProxy(UVRSimControllerProxyComponent* InControllerProxy);

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Interaction")
	UVRSimControllerProxyComponent* GetControllerProxy() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Interaction")
	FVector GetInteractionOrigin() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Interaction")
	FVector GetInteractionDirection() const;

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Interaction")
	bool SetSelectedTarget(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Interaction")
	void ClearSelectedTarget();

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Interaction")
	AActor* GetSelectedTarget() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Interaction")
	bool IsInteractableTarget(AActor* Target) const;

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Interaction")
	bool TryInteract();

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Debug")
	void SetDebugEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Debug")
	bool IsDebugEnabled() const;

	UFUNCTION(BlueprintNativeEvent, Category = "VR Simulate|Interaction")
	void UpdateDetection(float DeltaTime);
	virtual void UpdateDetection_Implementation(float DeltaTime);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Interaction")
	TObjectPtr<UVRSimControllerProxyComponent> ControllerProxy;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "VR Simulate|Interaction")
	TObjectPtr<AActor> SelectedTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Debug")
	bool bDebugInteraction = false;
};
