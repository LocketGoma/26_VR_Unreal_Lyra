#pragma once

#include "GameFramework/Actor.h"
#include "Interaction/VRSimInteractable.h"
#include "VRSimInteractableTarget.generated.h"

class UStaticMeshComponent;

UCLASS(BlueprintType, Blueprintable, meta = (ToolTip = "선택 및 상호작용이 가능한 기본 대상 액터입니다.", ShortToolTip = "기본 상호작용 대상 액터입니다."))
class VRSIMULATERUNTIME_API AVRSimInteractableTarget : public AActor, public IVRSimInteractable
{
	GENERATED_BODY()

public:
	AVRSimInteractableTarget();

	virtual bool CanInteract_Implementation(UObject* InteractionSource) const override;
	virtual void OnSelected_Implementation(UObject* InteractionSource) override;
	virtual void OnDeselected_Implementation(UObject* InteractionSource) override;
	virtual void Interact_Implementation(UObject* InteractionSource) override;

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Interaction")
	void SetInteractionEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Interaction")
	bool IsSelected() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Simulate|Components")
	TObjectPtr<UStaticMeshComponent> TargetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Interaction")
	bool bInteractionEnabled = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "VR Simulate|Interaction")
	bool bSelected = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "VR Simulate|Interaction", meta = (DisplayName = "On Selection Changed"))
	void ReceiveSelectionChanged(bool bIsSelected, UObject* InteractionSource);

	UFUNCTION(BlueprintImplementableEvent, Category = "VR Simulate|Interaction", meta = (DisplayName = "On Interaction"))
	void ReceiveInteraction(UObject* InteractionSource);
};
