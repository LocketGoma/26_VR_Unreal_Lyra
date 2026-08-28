#include "Tools/VRSimToolAdapterComponent.h"

#include "Base/VRSimControllerProxyComponent.h"

UVRSimToolAdapterComponent::UVRSimToolAdapterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVRSimToolAdapterComponent::SetControllerProxy(UVRSimControllerProxyComponent* InControllerProxy)
{
	ControllerProxy = InControllerProxy;
}

UVRSimControllerProxyComponent* UVRSimToolAdapterComponent::GetControllerProxy() const
{
	return ControllerProxy;
}

FVector UVRSimToolAdapterComponent::GetActionOrigin() const
{
	return ControllerProxy ? ControllerProxy->GetRayOrigin() : FVector::ZeroVector;
}

FVector UVRSimToolAdapterComponent::GetActionDirection() const
{
	return ControllerProxy ? ControllerProxy->GetForwardDirection() : FVector::ForwardVector;
}

bool UVRSimToolAdapterComponent::ExecutePrimaryAction_Implementation()
{
	return false;
}

bool UVRSimToolAdapterComponent::ExecuteSecondaryAction_Implementation()
{
	return false;
}
