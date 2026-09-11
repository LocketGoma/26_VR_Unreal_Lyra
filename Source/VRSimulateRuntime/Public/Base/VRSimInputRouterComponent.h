#pragma once

#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "VRSimInputRouterComponent.generated.h"

class UInputAction;
class UInputComponent;
class UInputMappingContext;
class UEnhancedInputComponent;
class UEnhancedInputLocalPlayerSubsystem;
class APlayerController;

UENUM(BlueprintType)
enum class EVRSimInputCommand : uint8 { Move, Look, Interact, AdjustDistance, Reset };

DECLARE_MULTICAST_DELEGATE_TwoParams(FVRSimInputCommandEvent, EVRSimInputCommand, const FInputActionValue&);

// Enhanced Input routing with per-command and per-key native fallback arbitration.
UCLASS(Blueprintable, ClassGroup="VR Simulate", meta=(BlueprintSpawnableComponent))
class VRSIMULATERUNTIME_API UVRSimInputRouterComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UVRSimInputRouterComponent();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UFUNCTION(BlueprintCallable, Category="VR Simulate|Input") void SetupInput(UInputComponent* Input);
	UFUNCTION(BlueprintCallable, Category="VR Simulate|Input") void ClearInput();
	void PollFallbackInput();
	void SetGameplayInputEnabled(bool bEnabled);
	void ResetInputState();
	UFUNCTION(BlueprintPure, Category="VR Simulate|Input") bool HasEnhancedBinding(EVRSimInputCommand Command) const;
	UFUNCTION(BlueprintPure, Category="VR Simulate|Input") FString GetBindingLabel(EVRSimInputCommand Command) const;
	FVRSimInputCommandEvent OnCommand;

	// Move/Look: Axis2D. AdjustDistance: Axis1D. Interact/Reset: Boolean.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="VR Simulate|Input")
	TMap<EVRSimInputCommand, TObjectPtr<UInputAction>> Actions;
	// Optional context. Existing contexts retain their external ownership and priority.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="VR Simulate|Input") TObjectPtr<UInputMappingContext> MappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="VR Simulate|Input") int32 MappingPriority = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR Simulate|Input") bool bEnableBuiltInFallback = true;
private:
	APlayerController* GetPlayerController() const;
	UEnhancedInputLocalPlayerSubsystem* GetInputSubsystem() const;
	void HandleEnhancedInput(const FInputActionValue& Value, EVRSimInputCommand Command);
	void HandleEnhancedRelease(const FInputActionValue& Value, EVRSimInputCommand Command);
	void Dispatch(EVRSimInputCommand Command, const FInputActionValue& Value);
	void BlockHeldFallbackKeys();
	bool IsFallbackKeyAvailable(const FKey& Key, bool bIgnoreHeldBlock = false) const;
	bool IsActionKeyDown(EVRSimInputCommand Command) const;
	UPROPERTY(Transient) TMap<EVRSimInputCommand, TObjectPtr<UInputAction>> BoundActions;
	UPROPERTY(Transient) TWeakObjectPtr<UInputComponent> SourceInput;
	UPROPERTY(Transient) TWeakObjectPtr<UEnhancedInputComponent> EnhancedInput;
	UPROPERTY(Transient) TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> ContextSubsystem;
	UPROPERTY(Transient) TObjectPtr<UInputMappingContext> OwnedContext;
	TArray<uint32> BindingHandles;
	TSet<FKey> BlockedUntilRelease;
	TSet<FKey> PreviousMappedKeys;
	bool bPreviousEnhanced[5] = {};
	bool bButtonDown[5] = {};
	uint64 LastCommandFrame[5] = { MAX_uint64, MAX_uint64, MAX_uint64, MAX_uint64, MAX_uint64 };
	bool bGameplayInputEnabled = true;
};
