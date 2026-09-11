#pragma once

#include "UObject/Interface.h"
#include "VRSimGrabbable.generated.h"

class UPrimitiveComponent;
class UVRSimGrabComponent;

// Movable root-body contract for swept, single-owner grabbing.
UINTERFACE(BlueprintType)
class VRSIMULATERUNTIME_API UVRSimGrabbable : public UInterface
{
	GENERATED_BODY()
};

class VRSIMULATERUNTIME_API IVRSimGrabbable
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="VR Simulate|Grab")
	bool CanGrab(UVRSimGrabComponent* Source) const;
	virtual bool CanGrab_Implementation(UVRSimGrabComponent* Source) const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="VR Simulate|Grab")
	UPrimitiveComponent* GetGrabPrimitive() const;
	virtual UPrimitiveComponent* GetGrabPrimitive_Implementation() const;
	UFUNCTION(BlueprintNativeEvent, Category="VR Simulate|Grab")
	void OnGrabbed(UVRSimGrabComponent* Source);
	virtual void OnGrabbed_Implementation(UVRSimGrabComponent* Source);
	UFUNCTION(BlueprintNativeEvent, Category="VR Simulate|Grab")
	void OnReleased(UVRSimGrabComponent* Source);
	virtual void OnReleased_Implementation(UVRSimGrabComponent* Source);
};
