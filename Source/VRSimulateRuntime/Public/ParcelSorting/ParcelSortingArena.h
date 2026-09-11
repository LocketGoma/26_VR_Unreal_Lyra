#pragma once

#include "GameFramework/Actor.h"
#include "ParcelSorting/ParcelSortingTypes.h"
#include "ParcelSortingArena.generated.h"

class AParcelCart;
class AParcelPackage;
class UStaticMesh;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FParcelRoundEvent);

// Single-arena round management and runtime workstation geometry.
UCLASS(Blueprintable)
class VRSIMULATERUNTIME_API AParcelSortingArena : public AActor
{
	GENERATED_BODY()
public:
	AParcelSortingArena();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	UFUNCTION(BlueprintCallable, Category="Parcel") void RestartRound();
	UFUNCTION(BlueprintCallable, Category="Parcel") void EndRound();
	UFUNCTION(BlueprintPure, Category="Parcel") bool IsRoundActive() const;
	UFUNCTION(BlueprintPure, Category="Parcel") float GetRemainingTime() const;
	void RecordDispatch(int32 Correct, int32 Wrong);
	static AParcelSortingArena* Find(const UWorld* World);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel") FParcelSortingSettings Settings;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Parcel") FParcelSortingStats Stats;
	UPROPERTY(BlueprintAssignable, Category="Parcel") FParcelRoundEvent OnRoundEnded;
	UPROPERTY(BlueprintAssignable, Category="Parcel") FParcelRoundEvent OnRoundRestarted;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Parcel") TArray<TObjectPtr<AParcelCart>> Carts;
private:
	void BuildWorkstation();
	void SpawnPackage();
	UPROPERTY() TObjectPtr<UStaticMesh> CubeMesh;
	UPROPERTY() TArray<TObjectPtr<AParcelPackage>> Packages;
	double RoundEndTime = 0;
	float SpawnCountdown = 0;
	bool bRunning = false;
};
