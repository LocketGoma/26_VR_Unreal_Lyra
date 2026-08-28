#pragma once

#include "Engine/EngineTypes.h"
#include "Interaction/VRSimInteractionComponent.h"
#include "VRSimRayInteractionComponent.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup = "VR Simulate", meta = (BlueprintSpawnableComponent, ToolTip = "Ray 기반 상호작용의 공통 데이터와 Debug 기능을 제공하는 컴포넌트입니다.", ShortToolTip = "Ray Interaction 컴포넌트입니다."))
class VRSIMULATERUNTIME_API UVRSimRayInteractionComponent : public UVRSimInteractionComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "VR Simulate|Ray")
	FVector GetRayStart() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Ray")
	FVector GetRayEnd() const;

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Debug")
	void DrawRayDebug(const FVector& Start, const FVector& End, const FHitResult& HitResult) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Ray", meta = (ClampMin = "0.0"))
	float TraceDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Ray")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
};
