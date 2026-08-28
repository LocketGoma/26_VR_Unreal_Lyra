#include "Interaction/VRSimInteractionComponent.h"

#include "Base/VRSimControllerProxyComponent.h"
#include "GameFramework/Actor.h"
#include "Interaction/VRSimInteractable.h"

UVRSimInteractionComponent::UVRSimInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVRSimInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateDetection(DeltaTime);
}

void UVRSimInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearSelectedTarget();
	Super::EndPlay(EndPlayReason);
}

void UVRSimInteractionComponent::SetControllerProxy(UVRSimControllerProxyComponent* InControllerProxy)
{
	ControllerProxy = InControllerProxy;
}

UVRSimControllerProxyComponent* UVRSimInteractionComponent::GetControllerProxy() const
{
	return ControllerProxy;
}

FVector UVRSimInteractionComponent::GetInteractionOrigin() const
{
	return ControllerProxy ? ControllerProxy->GetRayOrigin() : FVector::ZeroVector;
}

FVector UVRSimInteractionComponent::GetInteractionDirection() const
{
	return ControllerProxy ? ControllerProxy->GetForwardDirection() : FVector::ForwardVector;
}

bool UVRSimInteractionComponent::SetSelectedTarget(AActor* Target)
{
	if (Target == SelectedTarget)
	{
		return IsValid(Target);
	}

	ClearSelectedTarget();

	if (!IsInteractableTarget(Target)
		|| !IVRSimInteractable::Execute_CanInteract(Target, this))
	{
		return false;
	}

	SelectedTarget = Target;
	IVRSimInteractable::Execute_OnSelected(SelectedTarget, this);
	return true;
}

void UVRSimInteractionComponent::ClearSelectedTarget()
{
	if (IsValid(SelectedTarget))
	{
		IVRSimInteractable::Execute_OnDeselected(SelectedTarget, this);
	}

	SelectedTarget = nullptr;
}

AActor* UVRSimInteractionComponent::GetSelectedTarget() const
{
	return SelectedTarget;
}

bool UVRSimInteractionComponent::IsInteractableTarget(AActor* Target) const
{
	return IsValid(Target) && Target->GetClass()->ImplementsInterface(UVRSimInteractable::StaticClass());
}

bool UVRSimInteractionComponent::TryInteract()
{
	if (!IsInteractableTarget(SelectedTarget)
		|| !IVRSimInteractable::Execute_CanInteract(SelectedTarget, this))
	{
		return false;
	}

	IVRSimInteractable::Execute_Interact(SelectedTarget, this);
	return true;
}

void UVRSimInteractionComponent::SetDebugEnabled(bool bEnabled)
{
	bDebugInteraction = bEnabled;
}

bool UVRSimInteractionComponent::IsDebugEnabled() const
{
	return bDebugInteraction;
}

void UVRSimInteractionComponent::UpdateDetection_Implementation(float DeltaTime)
{
}
