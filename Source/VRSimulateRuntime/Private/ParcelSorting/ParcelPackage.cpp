#include "ParcelSorting/ParcelPackage.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Feedback/VRSimOutlineFeedbackComponent.h"
#include "Grab/VRSimGrabComponent.h"
#include "Interaction/VRSimInteractionComponent.h"
#include "ParcelSorting/ParcelSortingArena.h"
#include "UObject/ConstructorHelpers.h"

AParcelPackage::AParcelPackage()
{
	Box = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Box"));
	SetRootComponent(Box);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	Box->SetStaticMesh(Cube.Object);
	Box->SetMobility(EComponentMobility::Movable);
	Box->SetCollisionProfileName(TEXT("PhysicsActor"));
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Box->SetGenerateOverlapEvents(true);
	Box->SetSimulatePhysics(true);
	Box->SetLinearDamping(.3f);
	Box->SetAngularDamping(3.f);
	Box->BodyInstance.bUseCCD = true;
	Feedback = CreateDefaultSubobject<UVRSimOutlineFeedbackComponent>(TEXT("Feedback"));
	Feedback->SetTargetMesh(Box);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		auto* Label = CreateDefaultSubobject<UTextRenderComponent>(*FString::Printf(TEXT("Label%d"), Index));
		Label->SetupAttachment(Box);
		const FRotator Rotation(0, Index * 90.f, 0);
		Label->SetRelativeLocation(Rotation.Vector() * 50.5f);
		Label->SetRelativeRotation(Rotation);
		Label->SetHorizontalAlignment(EHTA_Center);
		Label->SetVerticalAlignment(EVRTA_TextCenter);
		Label->SetWorldSize(72.f);
		Label->SetText(FText::FromString(TEXT("A")));
		Labels.Add(Label);
	}
}

void AParcelPackage::Initialize(EParcelDestination InDestination, float Scale)
{
	Destination = InDestination;
	SetActorScale3D(FVector(Scale));
	for (UTextRenderComponent* Label : Labels)
	{
		Label->SetText(FText::FromString(ParcelSorting::Letter(Destination)));
		Label->SetTextRenderColor(ParcelSorting::Color(Destination));
	}
}

bool AParcelPackage::IsHeld() const
{
	return Grabber.IsValid() && Grabber->GetGrabbedTarget() == this;
}

bool AParcelPackage::CanInteract_Implementation(UObject* InteractionSource) const
{
	const AParcelSortingArena* Arena = AParcelSortingArena::Find(GetWorld());
	return Arena && Arena->IsRoundActive() && !bProcessed && !IsHeld();
}

void AParcelPackage::Interact_Implementation(UObject* InteractionSource)
{
	const auto* Interaction = Cast<UVRSimInteractionComponent>(InteractionSource);
	if (!Interaction || !Interaction->GetOwner() || !CanInteract_Implementation(InteractionSource)) return;
	if (auto* Grab = Interaction->GetOwner()->FindComponentByClass<UVRSimGrabComponent>())
	{
		if (Grab->GetInteractionComponent() == Interaction && Interaction->GetSelectedTarget() == this) Grab->TryGrab();
	}
}

void AParcelPackage::OnSelected_Implementation(UObject* InteractionSource)
{
	Feedback->PlaySelectionFeedback(true, this);
}

void AParcelPackage::OnDeselected_Implementation(UObject* InteractionSource)
{
	Feedback->PlaySelectionFeedback(false, this);
}

bool AParcelPackage::CanGrab_Implementation(UVRSimGrabComponent* Source) const
{
	return IsValid(Source) && CanInteract_Implementation(Source);
}

UPrimitiveComponent* AParcelPackage::GetGrabPrimitive_Implementation() const { return Box; }

void AParcelPackage::OnGrabbed_Implementation(UVRSimGrabComponent* Source)
{
	Grabber = Source;
	bOnConveyor = false;
	Feedback->SetHeldFeedback(true);
}

void AParcelPackage::OnReleased_Implementation(UVRSimGrabComponent* Source)
{
	if (Grabber.Get() != Source) return;
	Grabber.Reset();
	Feedback->SetHeldFeedback(false);
}

void AParcelPackage::SealForDispatch(AActor* Cart)
{
	if (bProcessed || IsHeld() || !IsValid(Cart)) return;
	bProcessed = true;
	bOnConveyor = false;
	Feedback->ClearFeedback();
	Box->SetSimulatePhysics(false);
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachToActor(Cart, FAttachmentTransformRules::KeepWorldTransform);
}
