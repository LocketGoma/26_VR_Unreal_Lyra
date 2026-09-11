#include "ParcelSorting/ParcelSortingCharacter.h"
#include "ParcelSorting/ParcelSortingArena.h"
#include "ParcelSorting/ParcelPackage.h"
#include "ParcelSorting/ParcelCart.h"
#include "Base/VRSimControllerProxyComponent.h"
#include "Interaction/VRSimRayInteractionComponent.h"
#include "Grab/VRSimGrabComponent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Camera/CameraTypes.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

AParcelSortingCharacter::AParcelSortingCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	ParcelAbilitySystem = CreateDefaultSubobject<ULyraAbilitySystemComponent>(TEXT("ParcelAbilitySystem"));
	ParcelAbilitySystem->SetIsReplicated(false);
	ControllerProxy = CreateDefaultSubobject<UVRSimControllerProxyComponent>(TEXT("ControllerProxy"));
	ControllerProxy->SetupAttachment(GetRootComponent());
	Interaction = CreateDefaultSubobject<UVRSimRayInteractionComponent>(TEXT("Interaction"));
	Interaction->SetControllerProxy(ControllerProxy);
	Grab = CreateDefaultSubobject<UVRSimGrabComponent>(TEXT("Grab"));
	Grab->SetInteractionComponent(Interaction);
	InputRouter = CreateDefaultSubobject<UVRSimInputRouterComponent>(TEXT("InputRouter"));
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetMesh()->SetVisibility(false);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->MaxWalkSpeed = 430.f;
	GetCharacterMovement()->bEnablePhysicsInteraction = false;
	BaseEyeHeight = 70.f;
}

UAbilitySystemComponent* AParcelSortingCharacter::GetAbilitySystemComponent() const { return ParcelAbilitySystem; }

void AParcelSortingCharacter::BeginPlay()
{
	Super::BeginPlay();
	// Explicit sample update order: view pose, detection, then held-object movement.
	Interaction->SetComponentTickEnabled(false);
	Grab->SetComponentTickEnabled(false);
	Interaction->SetTraceDistance(InteractionDistance);
	Grab->DefaultHoldDistance = HoldDistance;
	Grab->MaxHoldDistance = FMath::Max(Grab->MinHoldDistance, InteractionDistance - 20.f);
	Grab->ResetHoldDistance();
	InputRouter->OnCommand.AddUObject(this, &ThisClass::HandleInputCommand);
	ParcelAbilitySystem->InitAbilityActorInfo(this, this);
	InitializeGameplayTags();
	if (HasAuthority()) ParcelAbilitySystem->GiveAbility(FGameplayAbilitySpec(UParcelInteractionAbility::StaticClass(), 1));
	UpdateInteractionContext();
}

void AParcelSortingCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReleaseHeldPackage();
	InputRouter->OnCommand.RemoveAll(this);
	InputRouter->ClearInput();
	ParcelAbilitySystem->CancelAllAbilities();
	ParcelAbilitySystem->ClearActorInfo();
	Super::EndPlay(EndPlayReason);
}

void AParcelSortingCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	ParcelAbilitySystem->InitAbilityActorInfo(this, this);
	if (auto* PC = Cast<APlayerController>(NewController)) PC->SetInputMode(FInputModeGameOnly());
}

void AParcelSortingCharacter::UnPossessed()
{
	ReleaseHeldPackage();
	InputRouter->ClearInput();
	Super::UnPossessed();
}

bool AParcelSortingCharacter::CanPlay() const
{
	const auto* Arena = AParcelSortingArena::Find(GetWorld());
	return Arena && Arena->IsRoundActive() && IsLocallyControlled();
}

void AParcelSortingCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	OutResult.Location = GetPawnViewLocation();
	OutResult.Rotation = GetViewRotation();
	OutResult.FOV = 90.f;
}

void AParcelSortingCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	InputRouter->SetupInput(PlayerInputComponent);
}

void AParcelSortingCharacter::HandleInputCommand(EVRSimInputCommand Command, const FInputActionValue& Value)
{
	if (Command == EVRSimInputCommand::Reset)
	{
		if (auto* Arena = AParcelSortingArena::Find(GetWorld()); Arena && !Arena->IsRoundActive())
			Arena->RestartRound();
		return;
	}
	if (!CanPlay()) return;
	switch (Command)
	{
	case EVRSimInputCommand::Move:
	{
		const FVector2D Axis = Value.Get<FVector2D>();
		AddMovementInput(GetActorRightVector(), static_cast<float>(Axis.X));
		AddMovementInput(GetActorForwardVector(), static_cast<float>(Axis.Y));
		break;
	}
	case EVRSimInputCommand::Look:
	{
		const FVector2D Axis = Value.Get<FVector2D>();
		AddControllerYawInput(static_cast<float>(Axis.X));
		AddControllerPitchInput(static_cast<float>(Axis.Y));
		break;
	}
	case EVRSimInputCommand::Interact:
		ParcelAbilitySystem->TryActivateAbilityByClass(UParcelInteractionAbility::StaticClass());
		break;
	case EVRSimInputCommand::AdjustDistance:
		Grab->AdjustHoldDistance(Value.Get<float>());
		break;
	default:
		break;
	}
}

void AParcelSortingCharacter::UpdateInteractionContext()
{
	ControllerProxy->SetWorldLocationAndRotation(GetPawnViewLocation(), GetViewRotation());
	Interaction->SetInteractionEnabled(CanPlay());
}

void AParcelSortingCharacter::PerformInteraction()
{
	if (!CanPlay()) return;
	UpdateInteractionContext();
	if (Grab->IsGrabbing()) Grab->TryRelease();
	else Interaction->TryInteract();
}

void AParcelSortingCharacter::ReleaseHeldPackage()
{
	Interaction->SetInteractionEnabled(false);
	Grab->ClearGrabbedTarget();
}

void AParcelSortingCharacter::ResetForRound(const FVector& Location)
{
	ReleaseHeldPackage();
	ParcelAbilitySystem->CancelAllAbilities();
	ParcelAbilitySystem->ClearAbilityInput();
	ConsumeMovementInputVector();
	GetCharacterMovement()->StopMovementImmediately();
	SetActorLocationAndRotation(Location, FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	Grab->ResetHoldDistance();
	InputRouter->ResetInputState();
	if (auto* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetControlRotation(FRotator::ZeroRotator);
		PC->bShowMouseCursor = false;
		PC->bEnableClickEvents = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void AParcelSortingCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	InputRouter->SetGameplayInputEnabled(CanPlay());
	InputRouter->PollFallbackInput();
	UpdateInteractionContext();
	if (!CanPlay()) { ReleaseHeldPackage(); return; }
	Interaction->UpdateDetection(DeltaSeconds);
	Grab->UpdateManipulation(DeltaSeconds);
}

FString AParcelSortingCharacter::GetInteractionPrompt() const
{
	if (Grab->IsGrabbing()) return TEXT("Interact: release    |    Adjust distance: change reach");
	const FHitResult Hit = Interaction->GetLastHitResult();
	if (const auto* Package = Cast<AParcelPackage>(Interaction->GetSelectedTarget()))
		return FString::Printf(TEXT("Interact: grab parcel %s"), *ParcelSorting::Letter(Package->Destination));
	if (const auto* Cart = Cast<AParcelCart>(Hit.GetActor()); Cart && Cart->IsDepartureButton(Hit.GetComponent()))
		return Cart->State == EParcelCartState::Available
			? FString::Printf(TEXT("Interact: dispatch %s  (%d parcels)"), *ParcelSorting::Letter(Cart->Destination), Cart->GetLoad().Num())
			: TEXT("Cart unavailable");
	return TEXT("Aim at a parcel or the small button beside a cart");
}

FString AParcelSortingCharacter::GetControlHints() const
{
	return FString::Printf(TEXT("%s: move    %s: look    %s: interact    %s: reach"),
		*InputRouter->GetBindingLabel(EVRSimInputCommand::Move),
		*InputRouter->GetBindingLabel(EVRSimInputCommand::Look),
		*InputRouter->GetBindingLabel(EVRSimInputCommand::Interact),
		*InputRouter->GetBindingLabel(EVRSimInputCommand::AdjustDistance));
}

FString AParcelSortingCharacter::GetRestartLabel() const
{
	return FString::Printf(TEXT("RESTART  [%s]"), *InputRouter->GetBindingLabel(EVRSimInputCommand::Reset));
}

UParcelInteractionAbility::UParcelInteractionAbility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

bool UParcelInteractionAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const auto* Arena = ActorInfo && ActorInfo->AvatarActor.IsValid() ? AParcelSortingArena::Find(ActorInfo->AvatarActor->GetWorld()) : nullptr;
	return Arena && Arena->IsRoundActive() && Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UParcelInteractionAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo))
		if (auto* Character = Cast<AParcelSortingCharacter>(ActorInfo->AvatarActor.Get())) Character->PerformInteraction();
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
