#include "Example/VRSimRayInteractionExampleComponent.h"

#include "Base/VRSimControllerProxyComponent.h"
#include "Engine/World.h"

void UVRSimRayInteractionExampleComponent::UpdateDetection_Implementation(float DeltaTime)
{
	if (!ControllerProxy || !GetWorld())
	{
		ClearSelectedTarget();
		return;
	}

	const FVector Start = GetInteractionOrigin();
	const FVector Direction = GetInteractionDirection().GetSafeNormal();
	const FVector End = Start + Direction * TraceDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VRSimRayInteraction), false, GetOwner());
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, TraceChannel, QueryParams);

	SetSelectedTarget(HitResult.GetActor());
	DrawRayDebug(Start, End, HitResult);
}
