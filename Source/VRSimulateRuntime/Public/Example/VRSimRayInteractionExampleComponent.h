#pragma once

#include "Interaction/VRSimRayInteractionComponent.h"
#include "VRSimRayInteractionExampleComponent.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup = "VR Simulate", meta = (BlueprintSpawnableComponent, ToolTip = "Ray Interaction을 구현한 C++ 예제 컴포넌트입니다.", ShortToolTip = "Ray Interaction 예제 컴포넌트입니다."))
class VRSIMULATERUNTIME_API UVRSimRayInteractionExampleComponent : public UVRSimRayInteractionComponent
{
	GENERATED_BODY()

public:
	virtual void UpdateDetection_Implementation(float DeltaTime) override;
};
