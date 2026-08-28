#include "Interaction/VRSimNearInteractionComponent.h"

#include "Base/VRSimControllerProxyComponent.h"
#include "DrawDebugHelpers.h"

FVector UVRSimNearInteractionComponent::GetNearInteractionOrigin() const
{
	return ControllerProxy ? ControllerProxy->GetNearInteractionOrigin() : FVector::ZeroVector;
}

void UVRSimNearInteractionComponent::DrawNearInteractionDebug() const
{
	if (bDebugInteraction && GetWorld())
	{
		DrawDebugSphere(
			GetWorld(),
			GetNearInteractionOrigin(),
			InteractionRadius,
			16,
			FColor::Yellow,
			false,
			0.0f,
			0,
			1.0f);
	}
}
