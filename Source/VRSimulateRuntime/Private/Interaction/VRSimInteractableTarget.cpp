#include "Interaction/VRSimInteractableTarget.h"

#include "Components/StaticMeshComponent.h"

AVRSimInteractableTarget::AVRSimInteractableTarget()
{
	PrimaryActorTick.bCanEverTick = false;

	TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetMesh"));
	SetRootComponent(TargetMesh);
	TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TargetMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

bool AVRSimInteractableTarget::CanInteract_Implementation(UObject* InteractionSource) const
{
	return bInteractionEnabled;
}

void AVRSimInteractableTarget::OnSelected_Implementation(UObject* InteractionSource)
{
	bSelected = true;
	ReceiveSelectionChanged(true, InteractionSource);
}

void AVRSimInteractableTarget::OnDeselected_Implementation(UObject* InteractionSource)
{
	bSelected = false;
	ReceiveSelectionChanged(false, InteractionSource);
}

void AVRSimInteractableTarget::Interact_Implementation(UObject* InteractionSource)
{
	ReceiveInteraction(InteractionSource);
}

void AVRSimInteractableTarget::SetInteractionEnabled(bool bEnabled)
{
	bInteractionEnabled = bEnabled;
}

bool AVRSimInteractableTarget::IsSelected() const
{
	return bSelected;
}
