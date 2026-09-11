#pragma once

#include "Character/LyraCharacter.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Base/VRSimInputRouterComponent.h"
#include "ParcelSortingCharacter.generated.h"

class ULyraAbilitySystemComponent;
class UVRSimControllerProxyComponent;
class UVRSimRayInteractionComponent;
class UVRSimGrabComponent;

// First-person movement, input forwarding and sample component composition.
UCLASS(Blueprintable)
class VRSIMULATERUNTIME_API AParcelSortingCharacter : public ALyraCharacter
{
	GENERATED_BODY()
public:
	AParcelSortingCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	void PerformInteraction();
	void ReleaseHeldPackage();
	void ResetForRound(const FVector& Location);
	FString GetInteractionPrompt() const;
	FString GetControlHints() const;
	FString GetRestartLabel() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction") TObjectPtr<UVRSimControllerProxyComponent> ControllerProxy;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction") TObjectPtr<UVRSimRayInteractionComponent> Interaction;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction") TObjectPtr<UVRSimGrabComponent> Grab;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Input") TObjectPtr<UVRSimInputRouterComponent> InputRouter;

	// Existing sample defaults, in centimeters. Applied to the composed components at BeginPlay.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction", meta=(ClampMin="100")) float InteractionDistance = 350.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction", meta=(ClampMin="80")) float HoldDistance = 200.f;
private:
	bool CanPlay() const;
	void UpdateInteractionContext();
	void HandleInputCommand(EVRSimInputCommand Command, const FInputActionValue& Value);
	UPROPERTY() TObjectPtr<ULyraAbilitySystemComponent> ParcelAbilitySystem;
};

// GAS entry point for a validated grab, release or cart interaction.
UCLASS()
class VRSIMULATERUNTIME_API UParcelInteractionAbility : public ULyraGameplayAbility
{
	GENERATED_BODY()
public:
	UParcelInteractionAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
