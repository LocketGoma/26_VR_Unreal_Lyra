#pragma once

#include "Components/ActorComponent.h"
#include "VRSimSpatialInputComponent.generated.h"

class UVRSimControllerProxyComponent;

UCLASS(BlueprintType, Blueprintable, ClassGroup = "VR Simulate", meta = (BlueprintSpawnableComponent, ToolTip = "공간 입력으로 컨트롤러 프록시의 위치와 회전을 조작하는 컴포넌트입니다.", ShortToolTip = "공간 입력 컴포넌트입니다."))
class VRSIMULATERUNTIME_API UVRSimSpatialInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRSimSpatialInputComponent();
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Input")
	void SetControllerProxy(UVRSimControllerProxyComponent* InControllerProxy);

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Input")
	UVRSimControllerProxyComponent* GetControllerProxy() const;

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Input")
	void AddTranslationInput(FVector Input, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Input")
	void AddRotationInput(FRotator Input, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Input")
	void ResetProxyPose();

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Input")
	void SetSpatialInputEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Input")
	bool IsSpatialInputEnabled() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Input", meta = (ClampMin = "0.0"))
	float TranslationSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Input", meta = (ClampMin = "0.0"))
	float RotationSpeed = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Input")
	bool bSpatialInputEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Input")
	TObjectPtr<UVRSimControllerProxyComponent> ControllerProxy;

private:
	FTransform InitialRelativeTransform;
};
