#include "ParcelSorting/ParcelSortingArena.h"
#include "ParcelSorting/ParcelCart.h"
#include "ParcelSorting/ParcelPackage.h"
#include "ParcelSorting/ParcelSortingCharacter.h"
#include "Feedback/VRSimOutlineFeedbackComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

AParcelSortingArena::AParcelSortingArena()
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	CubeMesh = Cube.Object;
}

AParcelSortingArena* AParcelSortingArena::Find(const UWorld* World)
{
	if (!World) return nullptr;
	TActorIterator<AParcelSortingArena> It(World);
	return It ? *It : nullptr;
}

void AParcelSortingArena::BeginPlay()
{
	Super::BeginPlay();
	BuildWorkstation();
	RestartRound();
}

// Runtime workstation geometry, cart placement and lighting.
void AParcelSortingArena::BuildWorkstation()
{
	auto Part = [&](FVector Position, FVector Size, bool bCollision = true)
	{
		auto* Mesh = NewObject<UStaticMeshComponent>(this);
		AddInstanceComponent(Mesh);
		Mesh->SetupAttachment(RootComponent);
		Mesh->SetStaticMesh(CubeMesh);
		Mesh->SetRelativeLocation(Position);
		Mesh->SetRelativeScale3D(Size / 100.f);
		Mesh->SetCollisionProfileName(TEXT("BlockAll"));
		if (!bCollision) Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->RegisterComponent();
	};
	Part(FVector(450, 0, -15), FVector(1900, 2100, 30));
	Part(FVector(235, 0, 70), FVector(135, 1500, 24));
	Part(FVector(235, 0, 40), FVector(95, 1450, 65));
	// End stops on the work area, with space to walk around both ends of the belt.
	Part(FVector(-490, 0, 120), FVector(20, 2100, 240));
	Part(FVector(1390, 0, 120), FVector(20, 2100, 240));
	Part(FVector(450, -1040, 120), FVector(1900, 20, 240));
	Part(FVector(450, 1040, 120), FVector(1900, 20, 240));
	for (int32 Index = 0; Index < 20; ++Index)
		Part(FVector(235, -710 + Index * 74, 83), FVector(132, 3, 2), false);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		auto* Cart = GetWorld()->SpawnActor<AParcelCart>(GetActorLocation() + FVector(650, -450 + 300 * Index, 0), FRotator::ZeroRotator);
		if (Cart) { Cart->Initialize(static_cast<EParcelDestination>(Index), this); Carts.Add(Cart); }
	}
	auto* Sign = NewObject<UTextRenderComponent>(this);
	AddInstanceComponent(Sign);
	Sign->SetupAttachment(RootComponent);
	Sign->SetRelativeLocation(FVector(950, 0, 370));
	Sign->SetRelativeRotation(FRotator(0, 180, 0));
	Sign->SetHorizontalAlignment(EHTA_Center);
	Sign->SetWorldSize(42);
	Sign->SetText(FText::FromString(TEXT("PARCEL SORTING\nLOAD  /  CHECK  /  DISPATCH")));
	Sign->SetTextRenderColor(FColor(120, 210, 255));
	Sign->RegisterComponent();
	FActorSpawnParameters LightParams;
	LightParams.Owner = this;
	if (auto* Sun = GetWorld()->SpawnActor<ADirectionalLight>(GetActorLocation() + FVector(0, 0, 600), FRotator(-55, -35, 0), LightParams))
	{
		Sun->SetMobility(EComponentMobility::Movable);
		Sun->GetLightComponent()->SetIntensity(4.f);
	}
	for (int32 Side : { -1, 1 })
	{
		if (auto* Light = GetWorld()->SpawnActor<APointLight>(GetActorLocation() + FVector(450, Side * 550, 400), FRotator::ZeroRotator, LightParams))
		{
			Light->PointLightComponent->SetMobility(EComponentMobility::Movable);
			Light->PointLightComponent->SetIntensity(12000);
			Light->PointLightComponent->SetAttenuationRadius(1800);
		}
	}
}

bool AParcelSortingArena::IsRoundActive() const
{
	return bRunning && GetWorld() && GetWorld()->GetTimeSeconds() < RoundEndTime;
}

float AParcelSortingArena::GetRemainingTime() const
{
	return bRunning ? FMath::Max(0.f, static_cast<float>(RoundEndTime - GetWorld()->GetTimeSeconds())) : 0.f;
}

void AParcelSortingArena::RecordDispatch(int32 Correct, int32 Wrong)
{
	if (IsRoundActive()) Stats.RecordDispatch(Correct, Wrong, Settings);
}

void AParcelSortingArena::SpawnPackage()
{
	if (!IsRoundActive()) return;
	const float MinScale = FMath::Clamp(Settings.PackageMinScale, .2f, .8f);
	const float Scale = FMath::FRandRange(MinScale, FMath::Clamp(Settings.PackageMaxScale, MinScale, .8f));
	// Parcel scale applied before physics registration and spawn collision checks.
	const FTransform Transform(FRotator::ZeroRotator, GetActorLocation() + FVector(235, 690, 90 + 50 * Scale), FVector(Scale));
	auto* Package = GetWorld()->SpawnActorDeferred<AParcelPackage>(AParcelPackage::StaticClass(), Transform, this, nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);
	if (Package)
	{
		Package->Initialize(static_cast<EParcelDestination>(FMath::RandRange(0, 3)), Scale);
		Package->FinishSpawning(Transform);
		if (IsValid(Package)) Packages.Add(Package);
	}
}

void AParcelSortingArena::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bRunning) return;
	if (!IsRoundActive()) { EndRound(); return; }
	SpawnCountdown -= DeltaSeconds;
	if (SpawnCountdown <= 0)
	{
		SpawnPackage();
		SpawnCountdown = FMath::Max(.1f, Settings.PackageSpawnInterval);
	}
	for (int32 Index = Packages.Num() - 1; Index >= 0; --Index)
	{
		AParcelPackage* Package = Packages[Index];
		if (!IsValid(Package)) { Packages.RemoveAtSwap(Index); continue; }
		if (Package->IsHeld() || Package->IsProcessed()) continue;
		const FVector Position = Package->GetActorLocation() - GetActorLocation();
		const float Bottom = Package->Box->Bounds.GetBox().Min.Z - GetActorLocation().Z;
		const bool bOnBelt = FMath::Abs(Position.X - 235) < 68 && Bottom >= 75 && Bottom < 100;
		// Previous belt contact retained for end-zone detection across long physics steps.
		if ((bOnBelt || Package->WasOnConveyor()) && FMath::Abs(Position.X - 235) < 68 && Position.Y < -720)
		{
			++Stats.MissedPackages;
			Package->Destroy();
			Packages.RemoveAtSwap(Index);
		}
		else if (bOnBelt && Position.Y <= 760 && Position.Y >= -760)
		{
			const FVector Velocity = Package->Box->GetPhysicsLinearVelocity();
			Package->Box->SetPhysicsLinearVelocity(FVector(0, -FMath::Max(1.f, Settings.ConveyorSpeed), Velocity.Z));
		}
		else if (Position.Z < -1000)
		{
			// Out-of-world cleanup without a Missed increment.
			Package->Destroy();
			Packages.RemoveAtSwap(Index);
		}
		if (IsValid(Package)) Package->SetOnConveyor(bOnBelt);
	}
}

void AParcelSortingArena::EndRound()
{
	if (!bRunning) return;
	bRunning = false;
	for (TActorIterator<AParcelSortingCharacter> It(GetWorld()); It; ++It)
	{
		It->ReleaseHeldPackage();
		It->ConsumeMovementInputVector();
		It->GetCharacterMovement()->StopMovementImmediately();
		It->GetCharacterMovement()->DisableMovement();
		if (auto* PC = Cast<APlayerController>(It->GetController()))
		{
			PC->bShowMouseCursor = true;
			PC->bEnableClickEvents = true;
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			PC->SetInputMode(InputMode);
		}
	}
	for (TActorIterator<AParcelPackage> It(GetWorld()); It; ++It)
	{
		It->Feedback->ClearFeedback();
		It->Box->SetSimulatePhysics(false);
	}
	OnRoundEnded.Broadcast();
}

void AParcelSortingArena::RestartRound()
{
	bRunning = false;
	for (TActorIterator<AParcelSortingCharacter> It(GetWorld()); It; ++It)
	{
		It->ResetForRound(GetActorLocation() + FVector(0, 0, 100));
	}
	for (TActorIterator<AParcelPackage> It(GetWorld()); It; ++It) It->Destroy();
	Packages.Empty();
	for (AParcelCart* Cart : Carts) if (IsValid(Cart)) Cart->ResetCart();
	Stats = FParcelSortingStats();
	RoundEndTime = GetWorld()->GetTimeSeconds() + FMath::Max(1.f, Settings.GameDuration);
	SpawnCountdown = FMath::Max(.1f, Settings.PackageSpawnInterval);
	bRunning = true;
	OnRoundRestarted.Broadcast();
}
