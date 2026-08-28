#include "Grab/VRSimGrabComponent.h"

#include "Base/VRSimControllerProxyComponent.h"
#include "GameFramework/Actor.h"
#include "Interaction/VRSimInteractionComponent.h"

UVRSimGrabComponent::UVRSimGrabComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVRSimGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsGrabbing())
	{
		UpdateManipulation(DeltaTime);
	}
}

void UVRSimGrabComponent::SetInteractionComponent(UVRSimInteractionComponent* InInteractionComponent)
{
	InteractionComponent = InInteractionComponent;
}

UVRSimInteractionComponent* UVRSimGrabComponent::GetInteractionComponent() const
{
	return InteractionComponent;
}

AActor* UVRSimGrabComponent::GetCandidateTarget() const
{
	return InteractionComponent ? InteractionComponent->GetSelectedTarget() : nullptr;
}

bool UVRSimGrabComponent::SetGrabbedTarget(AActor* Target)
{
	if (!IsValid(Target))
	{
		return false;
	}

	GrabbedTarget = Target;
	return true;
}

void UVRSimGrabComponent::ClearGrabbedTarget()
{
	GrabbedTarget = nullptr;
}

AActor* UVRSimGrabComponent::GetGrabbedTarget() const
{
	return GrabbedTarget;
}

bool UVRSimGrabComponent::IsGrabbing() const
{
	return IsValid(GrabbedTarget);
}

FTransform UVRSimGrabComponent::GetGrabAttachTransform() const
{
	const UVRSimControllerProxyComponent* Proxy = InteractionComponent
		? InteractionComponent->GetControllerProxy()
		: nullptr;
	return Proxy ? Proxy->GetGrabAttachTransform() : FTransform::Identity;
}

bool UVRSimGrabComponent::TryGrab_Implementation()
{
	return false;
}

bool UVRSimGrabComponent::TryRelease_Implementation()
{
	return false;
}

void UVRSimGrabComponent::UpdateManipulation_Implementation(float DeltaTime)
{
}
