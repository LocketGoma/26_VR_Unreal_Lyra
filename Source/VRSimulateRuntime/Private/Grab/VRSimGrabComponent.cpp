#include "Grab/VRSimGrabComponent.h"

#include "Base/VRSimControllerProxyComponent.h"
#include "GameFramework/Actor.h"
#include "Interaction/VRSimInteractionComponent.h"
#include "Grab/VRSimGrabbable.h"
#include "Components/PrimitiveComponent.h"

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
	else if (GrabbedTarget) ClearGrabbedTarget();
}

void UVRSimGrabComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearGrabbedTarget();
	Super::EndPlay(EndPlayReason);
}

void UVRSimGrabComponent::SetInteractionComponent(UVRSimInteractionComponent* InInteractionComponent)
{
	if (InteractionComponent == InInteractionComponent) return;
	ClearGrabbedTarget();
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
	if (IsGrabbing() || !IsValid(Target) || !Target->Implements<UVRSimGrabbable>()
		|| !IVRSimGrabbable::Execute_CanGrab(Target, this))
	{
		return false;
	}

	UPrimitiveComponent* Body = IVRSimGrabbable::Execute_GetGrabPrimitive(Target);
	if (!IsValid(Body) || Body != Target->GetRootComponent() || Body->Mobility != EComponentMobility::Movable) return false;
	GrabbedTarget = Target;
	GrabbedPrimitive = Body;
	bRestoreSimulation = Body->IsSimulatingPhysics();
	Body->SetSimulatePhysics(false);
	ResetHoldDistance();
	if (InteractionComponent)
	{
		const FHitResult Hit = InteractionComponent->GetLastHitResult();
		if (Hit.GetActor() == Target)
			HoldDistance = FMath::Clamp(static_cast<float>(Hit.Distance), MinHoldDistance, FMath::Max(MinHoldDistance, MaxHoldDistance));
	}
	Target->OnDestroyed.AddDynamic(this, &ThisClass::HandleTargetDestroyed);
	IVRSimGrabbable::Execute_OnGrabbed(Target, this);
	// Held feedback takes priority over the deselection notification.
	if (InteractionComponent) InteractionComponent->SetSelectionSuppressed(true);
	return true;
}

void UVRSimGrabComponent::ClearGrabbedTarget()
{
	AActor* Target = GrabbedTarget;
	UPrimitiveComponent* Body = GrabbedPrimitive.Get();
	GrabbedTarget = nullptr;
	GrabbedPrimitive.Reset();
	if (IsValid(Target))
	{
		Target->OnDestroyed.RemoveDynamic(this, &ThisClass::HandleTargetDestroyed);
		if (IsValid(Body))
		{
			Body->SetSimulatePhysics(bRestoreSimulation);
			if (bRestoreSimulation)
			{
				Body->SetPhysicsLinearVelocity(FVector::ZeroVector);
				Body->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			}
		}
		if (Target->Implements<UVRSimGrabbable>()) IVRSimGrabbable::Execute_OnReleased(Target, this);
	}
	bRestoreSimulation = false;
	if (InteractionComponent) InteractionComponent->SetSelectionSuppressed(false);
}

void UVRSimGrabComponent::HandleTargetDestroyed(AActor* Target)
{
	ClearGrabbedTarget();
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
	FTransform Result = Proxy ? Proxy->GetGrabAttachTransform() : FTransform::Identity;
	if (Proxy) Result.AddToTranslation(Proxy->GetForwardDirection() * HoldDistance);
	return Result;
}

bool UVRSimGrabComponent::TryGrab_Implementation()
{
	return SetGrabbedTarget(GetCandidateTarget());
}

bool UVRSimGrabComponent::TryRelease_Implementation()
{
	const bool bHadTarget = IsGrabbing();
	ClearGrabbedTarget();
	return bHadTarget;
}

void UVRSimGrabComponent::UpdateManipulation_Implementation(float DeltaTime)
{
	if (!IsGrabbing() || !InteractionComponent || !InteractionComponent->GetControllerProxy()) return;
	if (!GrabbedPrimitive.IsValid()) { ClearGrabbedTarget(); return; }
	const FVector Location = FMath::VInterpTo(GrabbedTarget->GetActorLocation(), GetGrabAttachTransform().GetLocation(), DeltaTime, FollowSpeed);
	GrabbedTarget->SetActorLocation(Location, true);
}

void UVRSimGrabComponent::AdjustHoldDistance(float Steps)
{
	HoldDistance = FMath::Clamp(HoldDistance + Steps * DistanceStep, MinHoldDistance, FMath::Max(MinHoldDistance, MaxHoldDistance));
}

void UVRSimGrabComponent::ResetHoldDistance()
{
	HoldDistance = FMath::Clamp(DefaultHoldDistance, MinHoldDistance, FMath::Max(MinHoldDistance, MaxHoldDistance));
}
