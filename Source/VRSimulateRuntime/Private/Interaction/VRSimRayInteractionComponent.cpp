#include "Interaction/VRSimRayInteractionComponent.h"

#include "DrawDebugHelpers.h"

FVector UVRSimRayInteractionComponent::GetRayStart() const
{
	return GetInteractionOrigin();
}

FVector UVRSimRayInteractionComponent::GetRayEnd() const
{
	return GetRayStart() + GetInteractionDirection().GetSafeNormal() * TraceDistance;
}

void UVRSimRayInteractionComponent::DrawRayDebug(const FVector& Start, const FVector& End, const FHitResult& HitResult) const
{
	if (!bDebugInteraction || !GetWorld())
	{
		return;
	}

	const bool bHit = HitResult.bBlockingHit;
	const FVector TraceEnd = bHit ? HitResult.ImpactPoint : End;
	DrawDebugLine(GetWorld(), Start, TraceEnd, bHit ? FColor::Green : FColor::Red, false, 0.0f, 0, 1.5f);

	if (bHit)
	{
		DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Yellow, false, 0.0f);
		DrawDebugDirectionalArrow(
			GetWorld(),
			HitResult.ImpactPoint,
			HitResult.ImpactPoint + HitResult.ImpactNormal * 30.0f,
			8.0f,
			FColor::Blue,
			false,
			0.0f,
			0,
			1.0f);
	}
}
