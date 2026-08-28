#pragma once

#include "Components/ActorComponent.h"
#include "VRSimToolAdapterComponent.generated.h"

class UVRSimControllerProxyComponent;

UCLASS(BlueprintType, Blueprintable, ClassGroup = "VR Simulate", meta = (BlueprintSpawnableComponent, ToolTip = "컨트롤러 프록시의 방향을 Tool 또는 Shooter Action에 연결하는 어댑터 컴포넌트입니다.", ShortToolTip = "Tool Action 어댑터 컴포넌트입니다."))
class VRSIMULATERUNTIME_API UVRSimToolAdapterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRSimToolAdapterComponent();

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Tool")
	void SetControllerProxy(UVRSimControllerProxyComponent* InControllerProxy);

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Tool")
	UVRSimControllerProxyComponent* GetControllerProxy() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Tool")
	FVector GetActionOrigin() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Tool")
	FVector GetActionDirection() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Simulate|Tool")
	bool ExecutePrimaryAction();
	virtual bool ExecutePrimaryAction_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Simulate|Tool")
	bool ExecuteSecondaryAction();
	virtual bool ExecuteSecondaryAction_Implementation();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Tool")
	TObjectPtr<UVRSimControllerProxyComponent> ControllerProxy;
};
