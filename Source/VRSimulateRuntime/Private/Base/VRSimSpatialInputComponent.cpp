#include "Base/VRSimSpatialInputComponent.h"

#include "Base/VRSimControllerProxyComponent.h"

UVRSimSpatialInputComponent::UVRSimSpatialInputComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVRSimSpatialInputComponent::BeginPlay()
{
	Super::BeginPlay();
	InitialRelativeTransform = ControllerProxy ? ControllerProxy->GetRelativeTransform() : FTransform::Identity;
}

void UVRSimSpatialInputComponent::SetControllerProxy(UVRSimControllerProxyComponent* InControllerProxy)
{
	ControllerProxy = InControllerProxy;
	InitialRelativeTransform = ControllerProxy ? ControllerProxy->GetRelativeTransform() : FTransform::Identity;
}

UVRSimControllerProxyComponent* UVRSimSpatialInputComponent::GetControllerProxy() const
{
	return ControllerProxy;
}

void UVRSimSpatialInputComponent::AddTranslationInput(FVector Input, float DeltaTime)
{
	if (!bSpatialInputEnabled || !ControllerProxy)
	{
		return;
	}

	ControllerProxy->AddLocalOffset(Input * TranslationSpeed * DeltaTime);
}

void UVRSimSpatialInputComponent::AddRotationInput(FRotator Input, float DeltaTime)
{
	if (!bSpatialInputEnabled || !ControllerProxy)
	{
		return;
	}

	const FRotator DeltaRotation(
		Input.Pitch * RotationSpeed * DeltaTime,
		Input.Yaw * RotationSpeed * DeltaTime,
		Input.Roll * RotationSpeed * DeltaTime);

	ControllerProxy->AddLocalRotation(DeltaRotation);
}

void UVRSimSpatialInputComponent::ResetProxyPose()
{
	if (ControllerProxy)
	{
		ControllerProxy->SetRelativeTransform(InitialRelativeTransform);
	}
}

void UVRSimSpatialInputComponent::SetSpatialInputEnabled(bool bEnabled)
{
	bSpatialInputEnabled = bEnabled;
}

bool UVRSimSpatialInputComponent::IsSpatialInputEnabled() const
{
	return bSpatialInputEnabled;
}
