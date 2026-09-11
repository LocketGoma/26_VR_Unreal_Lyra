#include "Feedback/VRSimOutlineFeedbackComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

UVRSimOutlineFeedbackComponent::UVRSimOutlineFeedbackComponent()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Engine/EngineDebugMaterials/DebugMeshMaterial.DebugMeshMaterial"));
	CubeMesh = Cube.Object;
	EdgeMaterial = Material.Object;
}

void UVRSimOutlineFeedbackComponent::SetTargetMesh(UStaticMeshComponent* Mesh)
{
	if (TargetMesh == Mesh) return;
	DestroyEdges();
	TargetMesh = Mesh;
	if (HasBegunPlay()) RefreshVisuals();
}

void UVRSimOutlineFeedbackComponent::CreateEdges()
{
	if (!Edges.IsEmpty() || !IsValid(TargetMesh) || !TargetMesh->GetStaticMesh() || !CubeMesh) return;
	if (EdgeMaterial) EdgeMID = UMaterialInstanceDynamic::Create(EdgeMaterial, this);
	const FBox Bounds = TargetMesh->GetStaticMesh()->GetBoundingBox();
	const FVector Center = Bounds.GetCenter();
	const FVector Extent = Bounds.GetExtent() + FVector(.75);
	const double Thickness = FMath::Max(.1f, EdgeThickness);
	for (int32 Axis = 0; Axis < 3; ++Axis)
	{
		const int32 SideA = (Axis + 1) % 3;
		const int32 SideB = (Axis + 2) % 3;
		for (int32 SignA : { -1, 1 })
		{
			for (int32 SignB : { -1, 1 })
			{
				FVector Position = Center;
				Position[SideA] += Extent[SideA] * SignA;
				Position[SideB] += Extent[SideB] * SignB;
				FVector Size(Thickness);
				Size[Axis] = Extent[Axis] * 2 + Thickness;
				auto* Edge = NewObject<UStaticMeshComponent>(GetOwner());
				GetOwner()->AddInstanceComponent(Edge);
				Edge->SetupAttachment(TargetMesh);
				Edge->SetMobility(EComponentMobility::Movable);
				Edge->SetStaticMesh(CubeMesh);
				Edge->SetRelativeLocation(Position);
				Edge->SetRelativeScale3D(Size / 100.0);
				Edge->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Edge->SetGenerateOverlapEvents(false);
				Edge->SetCanEverAffectNavigation(false);
				Edge->SetCastShadow(false);
				Edge->bAffectDistanceFieldLighting = false;
				Edge->bAffectDynamicIndirectLighting = false;
				Edge->SetVisibility(false);
				if (EdgeMID) Edge->SetMaterial(0, EdgeMID);
				Edge->RegisterComponent();
				Edges.Add(Edge);
			}
		}
	}
}

void UVRSimOutlineFeedbackComponent::RefreshVisuals()
{
	const bool bVisible = bFeedbackEnabled && (bSelected || bHeld);
	if (bVisible) CreateEdges();
	if (EdgeMID)
	{
		const FLinearColor Color = bHeld ? HeldColor : SelectedColor;
		EdgeMID->SetVectorParameterValue(TEXT("Color"), FLinearColor(Color.R * EmissiveIntensity, Color.G * EmissiveIntensity, Color.B * EmissiveIntensity, 1.f));
	}
	for (UStaticMeshComponent* Edge : Edges) if (IsValid(Edge)) Edge->SetVisibility(bVisible);
}

void UVRSimOutlineFeedbackComponent::SetFeedbackEnabled(bool bEnabled)
{
	Super::SetFeedbackEnabled(bEnabled);
	RefreshVisuals();
}

void UVRSimOutlineFeedbackComponent::PlaySelectionFeedback_Implementation(bool bInSelected, UObject* Target)
{
	bSelected = bInSelected;
	RefreshVisuals();
}

void UVRSimOutlineFeedbackComponent::SetHeldFeedback(bool bInHeld)
{
	bHeld = bInHeld;
	RefreshVisuals();
}

void UVRSimOutlineFeedbackComponent::ClearFeedback()
{
	bSelected = bHeld = false;
	RefreshVisuals();
}

void UVRSimOutlineFeedbackComponent::DestroyEdges()
{
	for (UStaticMeshComponent* Edge : Edges)
	{
		if (!IsValid(Edge)) continue;
		if (GetOwner()) GetOwner()->RemoveInstanceComponent(Edge);
		Edge->DestroyComponent();
	}
	Edges.Empty();
	EdgeMID = nullptr;
}

void UVRSimOutlineFeedbackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearFeedback();
	DestroyEdges();
	Super::EndPlay(EndPlayReason);
}

void UVRSimOutlineFeedbackComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	DestroyEdges();
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}
