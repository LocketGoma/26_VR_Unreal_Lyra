#include "Base/VRSimInputRouterComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "EnhancedActionKeyMapping.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

namespace
{
	constexpr int32 CommandCount = 5;
	bool KeysOverlap(const FKey& A, const FKey& B)
	{
		if (A.GetVirtualKey() == B.GetVirtualKey()) return true;
		if (A == EKeys::Mouse2D && (B == EKeys::MouseX || B == EKeys::MouseY)) return true;
		return A == EKeys::MouseWheelAxis && (B == EKeys::MouseScrollUp || B == EKeys::MouseScrollDown);
	}
	bool IsButton(EVRSimInputCommand Command)
	{
		return Command == EVRSimInputCommand::Interact || Command == EVRSimInputCommand::Reset;
	}
	EInputActionValueType ExpectedType(EVRSimInputCommand Command)
	{
		if (IsButton(Command)) return EInputActionValueType::Boolean;
		return Command == EVRSimInputCommand::AdjustDistance ? EInputActionValueType::Axis1D : EInputActionValueType::Axis2D;
	}
	TArray<FKey> FallbackKeys()
	{
		return { EKeys::W, EKeys::S, EKeys::A, EKeys::D, EKeys::E, EKeys::LeftMouseButton,
			EKeys::R, EKeys::MouseScrollUp, EKeys::MouseScrollDown };
	}
}

UVRSimInputRouterComponent::UVRSimInputRouterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	for (int32 Index = 0; Index < CommandCount; ++Index) Actions.Add(static_cast<EVRSimInputCommand>(Index), nullptr);
}

APlayerController* UVRSimInputRouterComponent::GetPlayerController() const
{
	if (const auto* Pawn = Cast<APawn>(GetOwner())) return Cast<APlayerController>(Pawn->GetController());
	return Cast<APlayerController>(GetOwner());
}

UEnhancedInputLocalPlayerSubsystem* UVRSimInputRouterComponent::GetInputSubsystem() const
{
	const APlayerController* PC = GetPlayerController();
	const ULocalPlayer* Player = PC ? PC->GetLocalPlayer() : nullptr;
	return Player ? Player->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
}

void UVRSimInputRouterComponent::SetupInput(UInputComponent* Input)
{
	ClearInput();
	SourceInput = Input;
	EnhancedInput = Cast<UEnhancedInputComponent>(Input);
	if (EnhancedInput.IsValid())
	{
		for (const auto& Entry : Actions)
		{
			if (static_cast<int32>(Entry.Key) >= CommandCount) continue;
			UInputAction* Action = Entry.Value;
			if (!Action) continue;
			if (Action->ValueType != ExpectedType(Entry.Key))
			{
				UE_LOG(LogTemp, Warning, TEXT("VRSim input: incompatible value type for %s; binding omitted."), *GetNameSafe(Action));
				continue;
			}
			BoundActions.Add(Entry.Key, Action);
			BindingHandles.Add(EnhancedInput->BindAction(Action, ETriggerEvent::Triggered, this, &ThisClass::HandleEnhancedInput, Entry.Key).GetHandle());
			if (IsButton(Entry.Key))
			{
				BindingHandles.Add(EnhancedInput->BindAction(Action, ETriggerEvent::Completed, this, &ThisClass::HandleEnhancedRelease, Entry.Key).GetHandle());
				BindingHandles.Add(EnhancedInput->BindAction(Action, ETriggerEvent::Canceled, this, &ThisClass::HandleEnhancedRelease, Entry.Key).GetHandle());
			}
		}
		if (auto* Subsystem = GetInputSubsystem(); Subsystem && MappingContext && !Subsystem->HasMappingContext(MappingContext))
		{
			Subsystem->AddMappingContext(MappingContext, MappingPriority);
			ContextSubsystem = Subsystem;
			OwnedContext = MappingContext;
		}
	}
	ResetInputState();
}

void UVRSimInputRouterComponent::ClearInput()
{
	if (EnhancedInput.IsValid())
		for (uint32 Handle : BindingHandles) EnhancedInput->RemoveBindingByHandle(Handle);
	BindingHandles.Empty();
	BoundActions.Empty();
	if (ContextSubsystem.IsValid() && OwnedContext) ContextSubsystem->RemoveMappingContext(OwnedContext);
	OwnedContext = nullptr;
	ContextSubsystem.Reset();
	EnhancedInput.Reset();
	SourceInput.Reset();
	PreviousMappedKeys.Empty();
	BlockedUntilRelease.Empty();
	for (int32 Index = 0; Index < CommandCount; ++Index)
	{
		bPreviousEnhanced[Index] = false;
		bButtonDown[Index] = false;
		LastCommandFrame[Index] = MAX_uint64;
	}
}

bool UVRSimInputRouterComponent::HasEnhancedBinding(EVRSimInputCommand Command) const
{
	const UInputAction* Action = BoundActions.FindRef(Command);
	const auto* Subsystem = GetInputSubsystem();
	return EnhancedInput.IsValid() && Action && Subsystem && !Subsystem->QueryKeysMappedToAction(Action).IsEmpty();
}

bool UVRSimInputRouterComponent::IsActionKeyDown(EVRSimInputCommand Command) const
{
	const APlayerController* PC = GetPlayerController();
	const auto* Subsystem = GetInputSubsystem();
	if (PC && Subsystem)
		for (const FKey& Key : Subsystem->QueryKeysMappedToAction(BoundActions.FindRef(Command)))
			if (PC->IsInputKeyDown(Key)) return true;
	return false;
}

void UVRSimInputRouterComponent::BlockHeldFallbackKeys()
{
	if (const APlayerController* PC = GetPlayerController())
		for (const FKey& Key : FallbackKeys()) if (PC->IsInputKeyDown(Key)) BlockedUntilRelease.Add(Key);
}

void UVRSimInputRouterComponent::ResetInputState()
{
	BlockHeldFallbackKeys();
	for (int32 Index = 0; Index < CommandCount; ++Index)
	{
		const auto Command = static_cast<EVRSimInputCommand>(Index);
		bButtonDown[Index] = IsButton(Command) && IsActionKeyDown(Command);
		LastCommandFrame[Index] = GFrameCounter;
	}
}

void UVRSimInputRouterComponent::SetGameplayInputEnabled(bool bEnabled)
{
	if (bGameplayInputEnabled == bEnabled) return;
	bGameplayInputEnabled = bEnabled;
	ResetInputState();
}

bool UVRSimInputRouterComponent::IsFallbackKeyAvailable(const FKey& Key, bool bIgnoreHeldBlock) const
{
	if (!bIgnoreHeldBlock && BlockedUntilRelease.Contains(Key)) return false;
	const APlayerController* PC = GetPlayerController();
	const auto* PlayerInput = PC ? Cast<UEnhancedPlayerInput>(PC->PlayerInput) : nullptr;
	if (PlayerInput)
		for (const FEnhancedActionKeyMapping& Mapping : PlayerInput->GetEnhancedActionMappingsView())
			if (Mapping.Action && KeysOverlap(Mapping.Key, Key)) return false;
	return true;
}

void UVRSimInputRouterComponent::Dispatch(EVRSimInputCommand Command, const FInputActionValue& Value)
{
	if (!bGameplayInputEnabled && Command != EVRSimInputCommand::Reset) return;
	const int32 Index = static_cast<int32>(Command);
	if (Index < 0 || Index >= CommandCount || LastCommandFrame[Index] == GFrameCounter) return;
	LastCommandFrame[Index] = GFrameCounter;
	OnCommand.Broadcast(Command, Value);
}

void UVRSimInputRouterComponent::HandleEnhancedInput(const FInputActionValue& Value, EVRSimInputCommand Command)
{
	if (!HasEnhancedBinding(Command)) return;
	const int32 Index = static_cast<int32>(Command);
	if (IsButton(Command))
	{
		if (bButtonDown[Index] || !Value.Get<bool>()) return;
		bButtonDown[Index] = true;
	}
	Dispatch(Command, Value);
}

void UVRSimInputRouterComponent::HandleEnhancedRelease(const FInputActionValue& Value, EVRSimInputCommand Command)
{
	const int32 Index = static_cast<int32>(Command);
	if (Index < CommandCount) bButtonDown[Index] = false;
}

FString UVRSimInputRouterComponent::GetBindingLabel(EVRSimInputCommand Command) const
{
	TArray<FString> Labels;
	if (HasEnhancedBinding(Command))
	{
		for (const FKey& Key : GetInputSubsystem()->QueryKeysMappedToAction(BoundActions.FindRef(Command)))
			Labels.AddUnique(Key.GetDisplayName().ToString());
	}
	else if (bEnableBuiltInFallback)
	{
		TArray<FKey> Keys;
		switch (Command)
		{
		case EVRSimInputCommand::Move: Keys = { EKeys::W, EKeys::A, EKeys::S, EKeys::D }; break;
		case EVRSimInputCommand::Look: Keys = { EKeys::MouseX, EKeys::MouseY }; break;
		case EVRSimInputCommand::Interact: Keys = { EKeys::E, EKeys::LeftMouseButton }; break;
		case EVRSimInputCommand::AdjustDistance: Keys = { EKeys::MouseScrollUp, EKeys::MouseScrollDown }; break;
		case EVRSimInputCommand::Reset: Keys = { EKeys::R }; break;
		default: break;
		}
		for (const FKey& Key : Keys)
			if (IsFallbackKeyAvailable(Key, true)) Labels.Add(Key.GetDisplayName().ToString());
	}
	return Labels.IsEmpty() ? TEXT("Unbound") : FString::Join(Labels, TEXT("/"));
}

void UVRSimInputRouterComponent::PollFallbackInput()
{
	APlayerController* PC = GetPlayerController();
	if (!SourceInput.IsValid() || !PC || !PC->IsLocalController()) return;
	for (auto It = BlockedUntilRelease.CreateIterator(); It; ++It)
		if (!PC->IsInputKeyDown(*It)) It.RemoveCurrent();

	TSet<FKey> MappedKeys;
	if (const auto* PlayerInput = Cast<UEnhancedPlayerInput>(PC->PlayerInput))
		for (const FEnhancedActionKeyMapping& Mapping : PlayerInput->GetEnhancedActionMappingsView())
			if (Mapping.Action) MappedKeys.Add(Mapping.Key.GetVirtualKey());
	bool bRouteChanged = MappedKeys.Num() != PreviousMappedKeys.Num();
	for (const FKey& Key : MappedKeys) bRouteChanged |= !PreviousMappedKeys.Contains(Key);
	for (int32 Index = 0; Index < CommandCount; ++Index)
	{
		const bool bEnhanced = HasEnhancedBinding(static_cast<EVRSimInputCommand>(Index));
		bRouteChanged |= bEnhanced != bPreviousEnhanced[Index];
		if (!bEnhanced) bButtonDown[Index] = false;
		bPreviousEnhanced[Index] = bEnhanced;
	}
	if (bRouteChanged) BlockHeldFallbackKeys();
	PreviousMappedKeys = MoveTemp(MappedKeys);
	if (!bEnableBuiltInFallback) return;

	auto Down = [&](const FKey& Key) { return IsFallbackKeyAvailable(Key) && PC->IsInputKeyDown(Key) ? 1.f : 0.f; };
	auto Pressed = [&](const FKey& Key) { return IsFallbackKeyAvailable(Key) && PC->WasInputKeyJustPressed(Key); };
	if (!HasEnhancedBinding(EVRSimInputCommand::Move))
		Dispatch(EVRSimInputCommand::Move, FInputActionValue(FVector2D(Down(EKeys::D) - Down(EKeys::A), Down(EKeys::W) - Down(EKeys::S))));
	if (!HasEnhancedBinding(EVRSimInputCommand::Look))
	{
		float X = 0, Y = 0;
		PC->GetInputMouseDelta(X, Y);
		Dispatch(EVRSimInputCommand::Look, FInputActionValue(FVector2D(IsFallbackKeyAvailable(EKeys::MouseX) ? X : 0.f, IsFallbackKeyAvailable(EKeys::MouseY) ? -Y : 0.f)));
	}
	if (!HasEnhancedBinding(EVRSimInputCommand::Interact) && (Pressed(EKeys::E) || Pressed(EKeys::LeftMouseButton)))
		Dispatch(EVRSimInputCommand::Interact, FInputActionValue(true));
	if (!HasEnhancedBinding(EVRSimInputCommand::AdjustDistance))
	{
		const float Steps = (Pressed(EKeys::MouseScrollUp) ? 1.f : 0.f) - (Pressed(EKeys::MouseScrollDown) ? 1.f : 0.f);
		if (Steps != 0) Dispatch(EVRSimInputCommand::AdjustDistance, FInputActionValue(Steps));
	}
	if (!HasEnhancedBinding(EVRSimInputCommand::Reset) && Pressed(EKeys::R))
		Dispatch(EVRSimInputCommand::Reset, FInputActionValue(true));
}

void UVRSimInputRouterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearInput();
	Super::EndPlay(EndPlayReason);
}
