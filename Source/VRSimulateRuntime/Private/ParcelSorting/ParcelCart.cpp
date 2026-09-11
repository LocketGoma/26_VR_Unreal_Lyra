#include "ParcelSorting/ParcelCart.h"
#include "ParcelSorting/ParcelPackage.h"
#include "ParcelSorting/ParcelSortingArena.h"
#include "Interaction/VRSimInteractionComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AParcelCart::AParcelCart()
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	auto Part = [&](const TCHAR* Name, FVector Position, FVector Size)
	{
		auto* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Mesh->SetupAttachment(RootComponent);
		Mesh->SetStaticMesh(Cube.Object);
		Mesh->SetRelativeLocation(Position);
		Mesh->SetRelativeScale3D(Size / 100.f);
		Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		return Mesh;
	};
	Part(TEXT("Floor"), FVector(0, 0, 20), FVector(180, 190, 14));
	Part(TEXT("LeftWall"), FVector(0, -100, 95), FVector(180, 10, 160));
	Part(TEXT("RightWall"), FVector(0, 100, 95), FVector(180, 10, 160));
	Part(TEXT("BackWall"), FVector(95, 0, 95), FVector(10, 210, 160));
	Door = Part(TEXT("DepartureDoor"), FVector(-95, 0, 95), FVector(10, 210, 160));
	Door->SetVisibility(false);
	Door->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Button = Part(TEXT("DispatchButton"), FVector(-112, 115, 105), FVector(18, 28, 28));
	auto* ButtonLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ButtonLabel"));
	ButtonLabel->SetupAttachment(RootComponent);
	ButtonLabel->SetRelativeLocation(FVector(-122, 115, 128));
	ButtonLabel->SetRelativeRotation(FRotator(0, 180, 0));
	ButtonLabel->SetHorizontalAlignment(EHTA_Center);
	ButtonLabel->SetWorldSize(14);
	ButtonLabel->SetText(FText::FromString(TEXT("SEND")));
	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DestinationLabel"));
	Label->SetupAttachment(RootComponent);
	Label->SetRelativeLocation(FVector(88, 0, 220));
	Label->SetRelativeRotation(FRotator(0, 180, 0));
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetWorldSize(68);
	LoadingVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("LoadingVolume"));
	LoadingVolume->SetupAttachment(RootComponent);
	LoadingVolume->SetRelativeLocation(FVector(0, 0, 109));
	LoadingVolume->SetBoxExtent(FVector(89, 94, 82));
	LoadingVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LoadingVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	LoadingVolume->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	LoadingVolume->SetGenerateOverlapEvents(true);
}

void AParcelCart::BeginPlay()
{
	Super::BeginPlay();
	HomeTransform = GetActorTransform();
}

void AParcelCart::Initialize(EParcelDestination InDestination, AParcelSortingArena* InArena)
{
	Destination = InDestination;
	Arena = InArena;
	HomeTransform = GetActorTransform();
	Label->SetText(FText::FromString(ParcelSorting::Letter(Destination)));
	Label->SetTextRenderColor(ParcelSorting::Color(Destination));
}

TArray<AParcelPackage*> AParcelCart::GetLoad() const
{
	TArray<AParcelPackage*> Result;
	TArray<AActor*> Overlaps;
	// Current overlaps for the departure snapshot.
	LoadingVolume->UpdateOverlaps();
	LoadingVolume->GetOverlappingActors(Overlaps, AParcelPackage::StaticClass());
	for (AActor* Actor : Overlaps)
	{
		auto* Package = Cast<AParcelPackage>(Actor);
		if (!IsValid(Package) || Package->IsHeld() || Package->IsProcessed()) continue;
		// Full mesh containment with a one-centimeter contact tolerance.
		if (!Package->Box->GetStaticMesh()) continue;
		const FBox Bounds = Package->Box->GetStaticMesh()->GetBoundingBox();
		const FTransform Transform = LoadingVolume->GetComponentTransform();
		const FVector Extent = LoadingVolume->GetUnscaledBoxExtent();
		bool bInside = true;
		for (int32 Corner = 0; Corner < 8; ++Corner)
		{
			const FVector Point((Corner & 1) ? Bounds.Max.X : Bounds.Min.X,
				(Corner & 2) ? Bounds.Max.Y : Bounds.Min.Y, (Corner & 4) ? Bounds.Max.Z : Bounds.Min.Z);
			const FVector Local = Transform.InverseTransformPosition(Package->Box->GetComponentTransform().TransformPosition(Point)).GetAbs();
			bInside &= Local.X <= Extent.X + 1.f && Local.Y <= Extent.Y + 1.f && Local.Z <= Extent.Z + 1.f;
		}
		if (bInside) Result.Add(Package);
	}
	return Result;
}

bool AParcelCart::IsDepartureButton(const UPrimitiveComponent* Component) const { return Component == Button; }

bool AParcelCart::CanInteract_Implementation(UObject* InteractionSource) const
{
	const auto* Interaction = Cast<UVRSimInteractionComponent>(InteractionSource);
	if (!Interaction || !IsValid(Arena) || !Arena->IsRoundActive() || State != EParcelCartState::Available) return false;
	const FHitResult Hit = Interaction->GetLastHitResult();
	return Hit.GetActor() == this && IsDepartureButton(Hit.GetComponent());
}

void AParcelCart::Interact_Implementation(UObject* InteractionSource)
{
	if (CanInteract_Implementation(InteractionSource)) RequestDeparture();
}

bool AParcelCart::RequestDeparture()
{
	if (!IsValid(Arena) || !Arena->IsRoundActive() || State != EParcelCartState::Available) return false;
	State = EParcelCartState::Scoring; // Dispatch lock before snapshot collection.
	const TArray<AParcelPackage*> Load = GetLoad();
	int32 Correct = 0;
	int32 Wrong = 0;
	for (AParcelPackage* Package : Load)
	{
		(Package->Destination == Destination ? Correct : Wrong)++;
		Package->SealForDispatch(this);
		DispatchedPackages.Add(Package);
	}
	Arena->RecordDispatch(Correct, Wrong);
	State = EParcelCartState::ClosingDoor;
	StateTime = 0;
	Door->SetRelativeLocation(FVector(-95, 0, 260));
	Door->SetVisibility(true);
	Door->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Button->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	return true;
}

void AParcelCart::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!IsValid(Arena) || !Arena->IsRoundActive()) return;
	StateTime += DeltaSeconds;
	const float TravelAlpha = FMath::Clamp(StateTime / FMath::Max(.1f, TravelDuration), 0.f, 1.f);
	switch (State)
	{
	case EParcelCartState::ClosingDoor:
	{
		const float Alpha = FMath::Clamp(StateTime / FMath::Max(.1f, DoorCloseDuration), 0.f, 1.f);
		Door->SetRelativeLocation(FVector(-95, 0, FMath::Lerp(260.f, 95.f, Alpha)));
		if (Alpha >= 1) { State = EParcelCartState::Departing; StateTime = 0; }
		break;
	}
	case EParcelCartState::Departing:
		SetActorLocation(HomeTransform.GetLocation() + HomeTransform.GetRotation().GetForwardVector() * TravelDistance * TravelAlpha);
		if (TravelAlpha >= 1)
		{
			for (AParcelPackage* Package : DispatchedPackages) if (IsValid(Package)) Package->Destroy();
			DispatchedPackages.Empty();
			SetActorHiddenInGame(true);
			SetActorEnableCollision(false);
			State = EParcelCartState::Away; StateTime = 0;
		}
		break;
	case EParcelCartState::Away:
		if (StateTime >= FMath::Max(0.f, Arena->Settings.CartReturnTime))
		{
			State = EParcelCartState::Returning; StateTime = 0;
			SetActorHiddenInGame(false);
			Door->SetVisibility(false);
			Door->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		break;
	case EParcelCartState::Returning:
		SetActorLocation(HomeTransform.GetLocation() + HomeTransform.GetRotation().GetForwardVector() * TravelDistance * (1.f - TravelAlpha));
		if (TravelAlpha >= 1) ResetCart();
		break;
	default: break;
	}
}

void AParcelCart::ResetCart()
{
	for (AParcelPackage* Package : DispatchedPackages) if (IsValid(Package)) Package->Destroy();
	DispatchedPackages.Empty();
	SetActorTransform(HomeTransform, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	Door->SetVisibility(false);
	Door->SetRelativeLocation(FVector(-95, 0, 95));
	Door->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Button->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	State = EParcelCartState::Available;
	StateTime = 0;
}
