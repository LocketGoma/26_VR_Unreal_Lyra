#pragma once

#include "GameFramework/Actor.h"
#include "ParcelSorting/ParcelSortingTypes.h"
#include "Interaction/VRSimInteractable.h"
#include "ParcelCart.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class AParcelPackage;
class AParcelSortingArena;

UCLASS(Blueprintable)
class VRSIMULATERUNTIME_API AParcelCart : public AActor, public IVRSimInteractable
{
	GENERATED_BODY()
public:
	AParcelCart();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void Initialize(EParcelDestination InDestination, AParcelSortingArena* InArena);
	UFUNCTION(BlueprintCallable, Category="Parcel") bool RequestDeparture();
	UFUNCTION(BlueprintCallable, Category="Parcel") void ResetCart();
	TArray<AParcelPackage*> GetLoad() const;
	bool IsDepartureButton(const UPrimitiveComponent* Component) const;
	virtual bool CanInteract_Implementation(UObject* InteractionSource) const override;
	virtual void Interact_Implementation(UObject* InteractionSource) override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parcel") EParcelDestination Destination = EParcelDestination::A;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Parcel") EParcelCartState State = EParcelCartState::Available;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Parcel") TObjectPtr<UBoxComponent> LoadingVolume;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="0.1")) float DoorCloseDuration = .8f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="0.1")) float TravelDuration = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="200")) float TravelDistance = 1000.f;
private:
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Door;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Button;
	UPROPERTY() TObjectPtr<UTextRenderComponent> Label;
	UPROPERTY() TObjectPtr<AParcelSortingArena> Arena;
	UPROPERTY() TArray<TObjectPtr<AParcelPackage>> DispatchedPackages;
	FTransform HomeTransform = FTransform::Identity;
	float StateTime = 0;
};
