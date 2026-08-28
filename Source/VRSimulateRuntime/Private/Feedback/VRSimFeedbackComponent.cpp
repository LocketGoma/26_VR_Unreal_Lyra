#include "Feedback/VRSimFeedbackComponent.h"

UVRSimFeedbackComponent::UVRSimFeedbackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVRSimFeedbackComponent::SetFeedbackEnabled(bool bEnabled)
{
	bFeedbackEnabled = bEnabled;
}

bool UVRSimFeedbackComponent::IsFeedbackEnabled() const
{
	return bFeedbackEnabled;
}

void UVRSimFeedbackComponent::PlaySelectionFeedback_Implementation(bool bSelected, UObject* Target)
{
}

void UVRSimFeedbackComponent::PlayInteractionFeedback_Implementation(bool bSucceeded, UObject* Target)
{
}

void UVRSimFeedbackComponent::PlayDestinationFeedback_Implementation(bool bValid, const FVector& Destination)
{
}
