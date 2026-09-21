#pragma once

#include "Camera/LyraCameraMode.h"
#include "ParcelCameraMode.generated.h"

// Concrete first-person mode using Lyra's existing pivot, rotation and view calculation.
UCLASS(Blueprintable)
class VRSIMULATERUNTIME_API UParcelCameraMode : public ULyraCameraMode
{
	GENERATED_BODY()
public:
	UParcelCameraMode();
};
