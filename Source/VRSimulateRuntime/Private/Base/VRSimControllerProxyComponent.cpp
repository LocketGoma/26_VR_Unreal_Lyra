#include "Base/VRSimControllerProxyComponent.h"

#include "DrawDebugHelpers.h"

UVRSimControllerProxyComponent::UVRSimControllerProxyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UVRSimControllerProxyComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(bDebugInteraction);
}

void UVRSimControllerProxyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	DrawDebugVisualization();
}

FVector UVRSimControllerProxyComponent::GetWorldPosition() const
{
	return GetComponentLocation();
}

FRotator UVRSimControllerProxyComponent::GetWorldRotation() const
{
	return GetComponentRotation();
}

FVector UVRSimControllerProxyComponent::GetForwardDirection() const
{
	return GetForwardVector();
}

FVector UVRSimControllerProxyComponent::GetRayOrigin() const
{
	return GetComponentTransform().TransformPosition(RayOriginOffset);
}

FVector UVRSimControllerProxyComponent::GetNearInteractionOrigin() const
{
	return GetComponentTransform().TransformPosition(NearInteractionOriginOffset);
}

FTransform UVRSimControllerProxyComponent::GetGrabAttachTransform() const
{
	return GrabAttachOffset * GetComponentTransform();
}

void UVRSimControllerProxyComponent::SetDebugEnabled(bool bEnabled)
{
	bDebugInteraction = bEnabled;
	SetComponentTickEnabled(bDebugInteraction);
}

bool UVRSimControllerProxyComponent::IsDebugEnabled() const
{
	return bDebugInteraction;
}

void UVRSimControllerProxyComponent::DrawDebugVisualization() const
{
	if (!bDebugInteraction || !GetWorld())
	{
		return;
	}

	const FVector Origin = GetWorldPosition();
	const FVector DirectionEnd = Origin + GetForwardDirection() * DebugDirectionLength;

	DrawDebugCoordinateSystem(GetWorld(), Origin, GetWorldRotation(), 20.0f, false, 0.0f, 0, 1.5f);
	DrawDebugDirectionalArrow(GetWorld(), Origin, DirectionEnd, 10.0f, FColor::Cyan, false, 0.0f, 0, 1.5f);
	DrawDebugSphere(GetWorld(), GetNearInteractionOrigin(), NearInteractionRadius, 16, FColor::Yellow, false, 0.0f, 0, 1.0f);
}
