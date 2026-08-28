#include "Teleport/VRSimPortalBase.h"

#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"

AVRSimPortalBase::AVRSimPortalBase()
{
	DestinationPoint = CreateDefaultSubobject<USceneComponent>(TEXT("DestinationPoint"));
	DestinationPoint->SetupAttachment(RootComponent);
}

bool AVRSimPortalBase::CanInteract_Implementation(UObject* InteractionSource) const
{
	return bPortalEnabled && Super::CanInteract_Implementation(InteractionSource);
}

void AVRSimPortalBase::Interact_Implementation(UObject* InteractionSource)
{
	Super::Interact_Implementation(InteractionSource);

	AActor* Traveler = Cast<AActor>(InteractionSource);
	if (!Traveler)
	{
		const UActorComponent* SourceComponent = Cast<UActorComponent>(InteractionSource);
		Traveler = SourceComponent ? SourceComponent->GetOwner() : nullptr;
	}

	if (bPortalEnabled && Traveler)
	{
		TryActivatePortal(Traveler);
	}
}

void AVRSimPortalBase::SetPortalEnabled(bool bEnabled)
{
	bPortalEnabled = bEnabled;
}

bool AVRSimPortalBase::IsPortalEnabled() const
{
	return bPortalEnabled;
}

FTransform AVRSimPortalBase::GetDestinationTransform() const
{
	return DestinationPoint ? DestinationPoint->GetComponentTransform() : FTransform::Identity;
}

bool AVRSimPortalBase::TryActivatePortal_Implementation(AActor* Traveler)
{
	return false;
}
