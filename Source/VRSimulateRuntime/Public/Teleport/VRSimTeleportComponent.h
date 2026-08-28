#pragma once

#include "Components/ActorComponent.h"
#include "VRSimTeleportComponent.generated.h"

class AActor;
class UVRSimControllerProxyComponent;

UCLASS(BlueprintType, Blueprintable, ClassGroup = "VR Simulate", meta = (BlueprintSpawnableComponent, ToolTip = "Teleport 목적지 계산, 검증 및 실행의 공통 기반 컴포넌트입니다.", ShortToolTip = "Teleport 컴포넌트입니다."))
class VRSIMULATERUNTIME_API UVRSimTeleportComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRSimTeleportComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Teleport")
	void SetControllerProxy(UVRSimControllerProxyComponent* InControllerProxy);

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Teleport")
	UVRSimControllerProxyComponent* GetControllerProxy() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Teleport")
	FVector GetTeleportOrigin() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Teleport")
	FVector GetTeleportDirection() const;

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Teleport")
	void SetTeleportDestination(const FTransform& Destination, bool bValid);

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Teleport")
	void ClearTeleportDestination();

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Teleport")
	FTransform GetTeleportDestination() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Teleport")
	bool HasValidTeleportDestination() const;

	UFUNCTION(BlueprintNativeEvent, Category = "VR Simulate|Teleport")
	void UpdateDestination(float DeltaTime);
	virtual void UpdateDestination_Implementation(float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Simulate|Teleport")
	bool TryTeleport(AActor* TargetActor);
	virtual bool TryTeleport_Implementation(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Debug")
	void SetDebugEnabled(bool bEnabled);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Teleport")
	TObjectPtr<UVRSimControllerProxyComponent> ControllerProxy;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "VR Simulate|Teleport")
	FTransform TeleportDestination;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "VR Simulate|Teleport")
	bool bHasDestination = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "VR Simulate|Teleport")
	bool bDestinationValid = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Debug")
	bool bDebugInteraction = false;

private:
	void DrawTeleportDebug() const;
};
