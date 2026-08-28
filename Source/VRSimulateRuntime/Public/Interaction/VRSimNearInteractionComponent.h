#pragma once

#include "Interaction/VRSimInteractionComponent.h"
#include "VRSimNearInteractionComponent.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup = "VR Simulate", meta = (BlueprintSpawnableComponent, ToolTip = "근거리 Overlap 상호작용의 공통 기반 컴포넌트입니다.", ShortToolTip = "Near Interaction 컴포넌트입니다."))
class VRSIMULATERUNTIME_API UVRSimNearInteractionComponent : public UVRSimInteractionComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "VR Simulate|Near")
	FVector GetNearInteractionOrigin() const;

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Debug")
	void DrawNearInteractionDebug() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Near", meta = (ClampMin = "0.0"))
	float InteractionRadius = 20.0f;
};
