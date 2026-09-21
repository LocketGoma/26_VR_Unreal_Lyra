#pragma once

#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "ParcelSortingGameMode.generated.h"

class AParcelSortingArena;

UCLASS(Blueprintable)
class VRSIMULATERUNTIME_API AParcelSortingGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AParcelSortingGameMode();
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	// Authoritative arena class for Play; matching placed instances retain their settings.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Parcel", meta=(AllowAbstract="false"))
	TSubclassOf<AParcelSortingArena> ArenaClass;
	// Explicit level-instance priority for maps with independently configured arenas.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Parcel")
	bool bUsePlacedArena = false;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category="Parcel")
	TObjectPtr<AParcelSortingArena> ActiveArena;
private:
	void InitializeArena();
};

UCLASS()
class VRSIMULATERUNTIME_API AParcelSortingHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;
	virtual void NotifyHitBoxClick(FName BoxName) override;
};
