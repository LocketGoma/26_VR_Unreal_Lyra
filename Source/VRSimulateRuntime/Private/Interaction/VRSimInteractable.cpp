#include "Interaction/VRSimInteractable.h"

bool IVRSimInteractable::CanInteract_Implementation(UObject* InteractionSource) const
{
	return true;
}

void IVRSimInteractable::OnSelected_Implementation(UObject* InteractionSource)
{
}

void IVRSimInteractable::OnDeselected_Implementation(UObject* InteractionSource)
{
}

void IVRSimInteractable::Interact_Implementation(UObject* InteractionSource)
{
}
