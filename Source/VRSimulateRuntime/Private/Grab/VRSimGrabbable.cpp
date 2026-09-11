#include "Grab/VRSimGrabbable.h"

bool IVRSimGrabbable::CanGrab_Implementation(UVRSimGrabComponent* Source) const { return false; }
UPrimitiveComponent* IVRSimGrabbable::GetGrabPrimitive_Implementation() const { return nullptr; }
void IVRSimGrabbable::OnGrabbed_Implementation(UVRSimGrabComponent* Source) {}
void IVRSimGrabbable::OnReleased_Implementation(UVRSimGrabComponent* Source) {}
