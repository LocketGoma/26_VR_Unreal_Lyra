#pragma once

#include "Interaction/VRSimInteractableTarget.h"
#include "VRSimPortalBase.generated.h"

class USceneComponent;

UCLASS(BlueprintType, Blueprintable, meta = (ToolTip = "상호작용으로 활성화되는 Portal의 공통 기반 액터입니다.", ShortToolTip = "Portal 기본 액터입니다."))
class VRSIMULATERUNTIME_API AVRSimPortalBase : public AVRSimInteractableTarget
{
	GENERATED_BODY()

public:
	AVRSimPortalBase();

	virtual bool CanInteract_Implementation(UObject* InteractionSource) const override;
	virtual void Interact_Implementation(UObject* InteractionSource) override;

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Portal")
	void SetPortalEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Portal")
	bool IsPortalEnabled() const;

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Portal")
	FTransform GetDestinationTransform() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Simulate|Portal")
	bool TryActivatePortal(AActor* Traveler);
	virtual bool TryActivatePortal_Implementation(AActor* Traveler);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Simulate|Components")
	TObjectPtr<USceneComponent> DestinationPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Portal")
	bool bPortalEnabled = true;
};
