#pragma once

#include "Components/ActorComponent.h"
#include "VRSimFeedbackComponent.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup = "VR Simulate", meta = (BlueprintSpawnableComponent, ToolTip = "선택, 상호작용 및 목적지 상태의 피드백을 제공하는 컴포넌트입니다.", ShortToolTip = "상호작용 피드백 컴포넌트입니다."))
class VRSIMULATERUNTIME_API UVRSimFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRSimFeedbackComponent();

	UFUNCTION(BlueprintCallable, Category = "VR Simulate|Feedback")
	virtual void SetFeedbackEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "VR Simulate|Feedback")
	bool IsFeedbackEnabled() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Simulate|Feedback")
	void PlaySelectionFeedback(bool bSelected, UObject* Target);
	virtual void PlaySelectionFeedback_Implementation(bool bSelected, UObject* Target);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Simulate|Feedback")
	void PlayInteractionFeedback(bool bSucceeded, UObject* Target);
	virtual void PlayInteractionFeedback_Implementation(bool bSucceeded, UObject* Target);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Simulate|Feedback")
	void PlayDestinationFeedback(bool bValid, const FVector& Destination);
	virtual void PlayDestinationFeedback_Implementation(bool bValid, const FVector& Destination);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Simulate|Feedback")
	bool bFeedbackEnabled = true;
};
