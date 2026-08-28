#include "Teleport/VRSimTeleportComponent.h"

#include "Base/VRSimControllerProxyComponent.h"
#include "DrawDebugHelpers.h"

UVRSimTeleportComponent::UVRSimTeleportComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVRSimTeleportComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateDestination(DeltaTime);
	DrawTeleportDebug();
}

void UVRSimTeleportComponent::SetControllerProxy(UVRSimControllerProxyComponent* InControllerProxy)
{
	ControllerProxy = InControllerProxy;
}

UVRSimControllerProxyComponent* UVRSimTeleportComponent::GetControllerProxy() const
{
	return ControllerProxy;
}

FVector UVRSimTeleportComponent::GetTeleportOrigin() const
{
	return ControllerProxy ? ControllerProxy->GetRayOrigin() : FVector::ZeroVector;
}

FVector UVRSimTeleportComponent::GetTeleportDirection() const
{
	return ControllerProxy ? ControllerProxy->GetForwardDirection() : FVector::ForwardVector;
}

void UVRSimTeleportComponent::SetTeleportDestination(const FTransform& Destination, bool bValid)
{
	TeleportDestination = Destination;
	bHasDestination = true;
	bDestinationValid = bValid;
}

void UVRSimTeleportComponent::ClearTeleportDestination()
{
	TeleportDestination = FTransform::Identity;
	bHasDestination = false;
	bDestinationValid = false;
}

FTransform UVRSimTeleportComponent::GetTeleportDestination() const
{
	return TeleportDestination;
}

bool UVRSimTeleportComponent::HasValidTeleportDestination() const
{
	return bHasDestination && bDestinationValid;
}

void UVRSimTeleportComponent::UpdateDestination_Implementation(float DeltaTime)
{
}

bool UVRSimTeleportComponent::TryTeleport_Implementation(AActor* TargetActor)
{
	return false;
}

void UVRSimTeleportComponent::SetDebugEnabled(bool bEnabled)
{
	bDebugInteraction = bEnabled;
}

void UVRSimTeleportComponent::DrawTeleportDebug() const
{
	if (bDebugInteraction && bHasDestination && GetWorld())
	{
		DrawDebugSphere(
			GetWorld(),
			TeleportDestination.GetLocation(),
			16.0f,
			16,
			bDestinationValid ? FColor::Green : FColor::Red,
			false,
			0.0f,
			0,
			1.5f);
	}
}
