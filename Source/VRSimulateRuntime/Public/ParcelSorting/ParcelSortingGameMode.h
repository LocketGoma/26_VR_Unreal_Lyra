#pragma once

#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "ParcelSortingGameMode.generated.h"

UCLASS()
class VRSIMULATERUNTIME_API AParcelSortingGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AParcelSortingGameMode();
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
};

UCLASS()
class VRSIMULATERUNTIME_API AParcelSortingHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;
	virtual void NotifyHitBoxClick(FName BoxName) override;
};
