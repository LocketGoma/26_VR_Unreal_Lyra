#pragma once

#include "CoreMinimal.h"
#include "ParcelSortingTypes.generated.h"

UENUM(BlueprintType)
enum class EParcelDestination : uint8 { A, B, C, D };

UENUM(BlueprintType)
enum class EParcelCartState : uint8 { Available, Scoring, ClosingDoor, Departing, Away, Returning };

USTRUCT(BlueprintType)
struct VRSIMULATERUNTIME_API FParcelSortingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Round", meta=(ClampMin="1"))
	float GameDuration = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conveyor", meta=(ClampMin="0.1"))
	float PackageSpawnInterval = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conveyor", meta=(ClampMin="1"))
	float ConveyorSpeed = 65.f;
	// Waiting time after the cart has left the work area.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cart", meta=(ClampMin="0"))
	float CartReturnTime = 30.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scoring")
	int32 CorrectScore = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scoring")
	int32 WrongScore = -5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Packages", meta=(ClampMin="0.2", ClampMax="0.8"))
	float PackageMinScale = .32f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Packages", meta=(ClampMin="0.2", ClampMax="0.8"))
	float PackageMaxScale = .55f;
};

USTRUCT(BlueprintType)
struct VRSIMULATERUNTIME_API FParcelSortingStats
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category="Result") int32 Score = 0;
	UPROPERTY(BlueprintReadOnly, Category="Result") int32 SentCarts = 0;
	UPROPERTY(BlueprintReadOnly, Category="Result") int32 LoadedPackages = 0;
	UPROPERTY(BlueprintReadOnly, Category="Result") int32 CorrectPackages = 0;
	UPROPERTY(BlueprintReadOnly, Category="Result") int32 WrongPackages = 0;
	UPROPERTY(BlueprintReadOnly, Category="Result") int32 MissedPackages = 0;

	void RecordDispatch(int32 Correct, int32 Wrong, const FParcelSortingSettings& Settings)
	{
		++SentCarts;
		CorrectPackages += Correct;
		WrongPackages += Wrong;
		LoadedPackages += Correct + Wrong;
		Score += Correct * Settings.CorrectScore + Wrong * Settings.WrongScore;
	}
};

namespace ParcelSorting
{
	inline FString Letter(EParcelDestination Destination) { return FString::Chr(TEXT('A') + static_cast<uint8>(Destination)); }
	inline FColor Color(EParcelDestination Destination)
	{
		static const FColor Colors[] = { FColor(80, 190, 255), FColor(255, 185, 55), FColor(90, 235, 160), FColor(235, 115, 210) };
		return Colors[FMath::Clamp(static_cast<int32>(Destination), 0, 3)];
	}
}
