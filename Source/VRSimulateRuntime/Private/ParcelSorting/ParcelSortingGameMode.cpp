#include "ParcelSorting/ParcelSortingGameMode.h"
#include "ParcelSorting/ParcelSortingArena.h"
#include "ParcelSorting/ParcelSortingCharacter.h"
#include "ParcelSorting/ParcelCart.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"

AParcelSortingGameMode::AParcelSortingGameMode()
{
	DefaultPawnClass = AParcelSortingCharacter::StaticClass();
	HUDClass = AParcelSortingHUD::StaticClass();
}

void AParcelSortingGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	TActorIterator<APlayerStart> PlayerStartIt(GetWorld());
	if (!PlayerStartIt) GetWorld()->SpawnActor<APlayerStart>(FVector(0, 0, 100), FRotator::ZeroRotator);
}

void AParcelSortingGameMode::StartPlay()
{
	if (!AParcelSortingArena::Find(GetWorld())) GetWorld()->SpawnActor<AParcelSortingArena>();
	Super::StartPlay();
}

void AParcelSortingHUD::DrawHUD()
{
	Super::DrawHUD();
	auto* Arena = AParcelSortingArena::Find(GetWorld());
	if (!Arena || !Canvas) return;
	const float Scale = FMath::Clamp(Canvas->SizeY / 900.f, .65f, 1.6f);
	auto Text = [&](const FString& Value, float X, float Y, float FontScale = 1.f, FLinearColor Color = FLinearColor::White)
	{
		DrawText(Value, Color, X, Y, GEngine->GetMediumFont(), FontScale * Scale);
	};
	const FParcelSortingStats& Stats = Arena->Stats;
	if (Arena->IsRoundActive())
	{
		DrawRect(FLinearColor(.015f, .025f, .045f, .9f), 18, 18, 440 * Scale, 110 * Scale);
		Text(TEXT("PARCEL SORTING"), 34, 28, 1.3f, FLinearColor(.3f, .8f, 1));
		const int32 Seconds = FMath::CeilToInt(Arena->GetRemainingTime());
		Text(FString::Printf(TEXT("%02d:%02d    SCORE  %d    SENT  %d"), Seconds / 60, Seconds % 60, Stats.Score, Stats.SentCarts), 34, 66 * Scale, 1.2f);
		Text(TEXT("Score is recorded only when a cart is dispatched."), 34, 101 * Scale, .8f);
		for (int32 Index = 0; Index < Arena->Carts.Num(); ++Index)
		{
			const AParcelCart* Cart = Arena->Carts[Index];
			if (!IsValid(Cart)) continue;
			const FString Status = Cart->State == EParcelCartState::Available ? FString::Printf(TEXT("%d ready"), Cart->GetLoad().Num())
				: StaticEnum<EParcelCartState>()->GetNameStringByValue(static_cast<int64>(Cart->State));
			Text(FString::Printf(TEXT("%s   %s"), *ParcelSorting::Letter(Cart->Destination), *Status), Canvas->SizeX - 225 * Scale, (28 + Index * 32) * Scale, 1.f, FLinearColor(ParcelSorting::Color(Cart->Destination)));
		}
		DrawRect(FLinearColor::White, Canvas->SizeX / 2.f - 2, Canvas->SizeY / 2.f - 2, 4, 4);
		DrawRect(FLinearColor(0, 0, 0, .8f), 18, Canvas->SizeY - 88 * Scale, Canvas->SizeX - 36, 72 * Scale);
		if (const auto* Character = Cast<AParcelSortingCharacter>(GetOwningPawn()))
			Text(Character->GetInteractionPrompt(), 34, Canvas->SizeY - 80 * Scale, 1.1f);
		if (const auto* Character = Cast<AParcelSortingCharacter>(GetOwningPawn()))
			Text(Character->GetControlHints(), 34, Canvas->SizeY - 43 * Scale, .85f);
	}
	else
	{
		DrawRect(FLinearColor(0, 0, .015f, .88f), 0, 0, Canvas->SizeX, Canvas->SizeY);
		const float Left = Canvas->SizeX / 2.f - 230 * Scale;
		const float Top = Canvas->SizeY / 2.f - 220 * Scale;
		Text(TEXT("SHIFT COMPLETE"), Left, Top, 2, FLinearColor(.3f, .8f, 1));
		const TArray<FString> Rows = {
			FString::Printf(TEXT("Score                         %d"), Stats.Score),
			FString::Printf(TEXT("Sent carts                    %d"), Stats.SentCarts),
			FString::Printf(TEXT("Loaded packages               %d"), Stats.LoadedPackages),
			FString::Printf(TEXT("Wrong packages                %d"), Stats.WrongPackages),
			FString::Printf(TEXT("Missed packages               %d"), Stats.MissedPackages) };
		for (int32 Index = 0; Index < Rows.Num(); ++Index) Text(Rows[Index], Left, Top + (80 + Index * 44) * Scale, 1.3f);
		Text(TEXT("Undispatched parcels are not included."), Left, Top + 315 * Scale, .9f);
		const FVector2D ButtonPosition(Left, Top + 365 * Scale);
		const FVector2D ButtonSize(460 * Scale, 60 * Scale);
		DrawRect(FLinearColor(.025f, .35f, .6f, 1), ButtonPosition.X, ButtonPosition.Y, ButtonSize.X, ButtonSize.Y);
		const auto* Character = Cast<AParcelSortingCharacter>(GetOwningPawn());
		Text(Character ? Character->GetRestartLabel() : TEXT("RESTART"), Left + 100 * Scale, ButtonPosition.Y + 16 * Scale, 1.3f);
		AddHitBox(ButtonPosition, ButtonSize, TEXT("Restart"), true);
	}
}

void AParcelSortingHUD::NotifyHitBoxClick(FName BoxName)
{
	Super::NotifyHitBoxClick(BoxName);
	if (BoxName == TEXT("Restart"))
		if (auto* Arena = AParcelSortingArena::Find(GetWorld()); Arena && !Arena->IsRoundActive()) Arena->RestartRound();
}
