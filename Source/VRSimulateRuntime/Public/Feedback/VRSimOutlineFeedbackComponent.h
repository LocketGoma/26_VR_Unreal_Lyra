#pragma once

#include "Feedback/VRSimFeedbackComponent.h"
#include "VRSimOutlineFeedbackComponent.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

// Local bounding-box edge feedback for selection and held state.
UCLASS(Blueprintable, ClassGroup="VR Simulate", meta=(BlueprintSpawnableComponent))
class VRSIMULATERUNTIME_API UVRSimOutlineFeedbackComponent : public UVRSimFeedbackComponent
{
	GENERATED_BODY()
public:
	UVRSimOutlineFeedbackComponent();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	virtual void SetFeedbackEnabled(bool bEnabled) override;
	virtual void PlaySelectionFeedback_Implementation(bool bSelected, UObject* Target) override;
	UFUNCTION(BlueprintCallable, Category="VR Simulate|Feedback") void SetTargetMesh(UStaticMeshComponent* Mesh);
	UFUNCTION(BlueprintCallable, Category="VR Simulate|Feedback") void SetHeldFeedback(bool bHeld);
	UFUNCTION(BlueprintCallable, Category="VR Simulate|Feedback") void ClearFeedback();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR Simulate|Feedback") FLinearColor SelectedColor = FLinearColor(.05f, .75f, 1.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR Simulate|Feedback") FLinearColor HeldColor = FLinearColor(1.f, .55f, .06f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR Simulate|Feedback", meta=(ClampMin="0")) float EmissiveIntensity = 3.f;
	// Local centimeters; scaled by the target mesh transform.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR Simulate|Feedback", meta=(ClampMin="0.1")) float EdgeThickness = 3.5f;
private:
	void CreateEdges();
	void RefreshVisuals();
	void DestroyEdges();
	UPROPERTY() TObjectPtr<UStaticMeshComponent> TargetMesh;
	UPROPERTY() TObjectPtr<UStaticMesh> CubeMesh;
	UPROPERTY() TObjectPtr<UMaterialInterface> EdgeMaterial;
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> EdgeMID;
	UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> Edges;
	bool bSelected = false;
	bool bHeld = false;
};
