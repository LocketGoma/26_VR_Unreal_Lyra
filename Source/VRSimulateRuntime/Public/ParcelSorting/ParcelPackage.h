#pragma once

#include "GameFramework/Actor.h"
#include "ParcelSorting/ParcelSortingTypes.h"
#include "Interaction/VRSimInteractable.h"
#include "Grab/VRSimGrabbable.h"
#include "ParcelPackage.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;
class UVRSimOutlineFeedbackComponent;

UCLASS(Blueprintable)
class VRSIMULATERUNTIME_API AParcelPackage : public AActor, public IVRSimInteractable, public IVRSimGrabbable
{
	GENERATED_BODY()
public:
	AParcelPackage();
	void Initialize(EParcelDestination InDestination, float Scale);
	void SealForDispatch(AActor* Cart);
	bool IsHeld() const;
	virtual bool CanInteract_Implementation(UObject* InteractionSource) const override;
	virtual void Interact_Implementation(UObject* InteractionSource) override;
	virtual void OnSelected_Implementation(UObject* InteractionSource) override;
	virtual void OnDeselected_Implementation(UObject* InteractionSource) override;
	virtual bool CanGrab_Implementation(UVRSimGrabComponent* Source) const override;
	virtual UPrimitiveComponent* GetGrabPrimitive_Implementation() const override;
	virtual void OnGrabbed_Implementation(UVRSimGrabComponent* Source) override;
	virtual void OnReleased_Implementation(UVRSimGrabComponent* Source) override;
	bool IsProcessed() const { return bProcessed; }
	bool WasOnConveyor() const { return bOnConveyor; }
	void SetOnConveyor(bool bValue) { bOnConveyor = bValue; }
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Parcel") TObjectPtr<UStaticMeshComponent> Box;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Parcel") TObjectPtr<UVRSimOutlineFeedbackComponent> Feedback;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Parcel") EParcelDestination Destination = EParcelDestination::A;
private:
	UPROPERTY() TArray<TObjectPtr<UTextRenderComponent>> Labels;
	UPROPERTY(Transient) TWeakObjectPtr<UVRSimGrabComponent> Grabber;
	bool bProcessed = false;
	bool bOnConveyor = false;
};
