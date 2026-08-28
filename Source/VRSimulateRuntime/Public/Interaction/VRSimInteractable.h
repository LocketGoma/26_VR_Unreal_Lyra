#pragma once

#include "UObject/Interface.h"
#include "VRSimInteractable.generated.h"

UINTERFACE(BlueprintType, meta = (ToolTip = "선택 및 상호작용이 가능한 대상의 공통 인터페이스입니다.", ShortToolTip = "상호작용 대상 인터페이스입니다."))
class VRSIMULATERUNTIME_API UVRSimInteractable : public UInterface
{
	GENERATED_BODY()
};

class VRSIMULATERUNTIME_API IVRSimInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "VR Simulate|Interaction")
	bool CanInteract(UObject* InteractionSource) const;
	virtual bool CanInteract_Implementation(UObject* InteractionSource) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "VR Simulate|Interaction")
	void OnSelected(UObject* InteractionSource);
	virtual void OnSelected_Implementation(UObject* InteractionSource);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "VR Simulate|Interaction")
	void OnDeselected(UObject* InteractionSource);
	virtual void OnDeselected_Implementation(UObject* InteractionSource);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "VR Simulate|Interaction")
	void Interact(UObject* InteractionSource);
	virtual void Interact_Implementation(UObject* InteractionSource);
};
