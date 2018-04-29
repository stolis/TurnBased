#include "../XMLParser/pugixml.hpp"
#include "Helpers/CoordinateTools.h"
#include <vector>
#include "Core.h"
#include "Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Components/TextRenderComponent.h"
#include "Runtime/Engine/Classes/Components/SplineComponent.h"
#include "Runtime/Engine/Classes/Components/SplineMeshComponent.h"
#include "Runtime/Engine/Classes/Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "GameFramework/Actor.h"
#include "CityGenerator.generated.h"

UCLASS(Blueprintable)
class TURNBASED_API ACityGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	ACityGenerator();
	//void OnConstruction(const FTransform&);

public:
	
	UPROPERTY(VisibleAnywhere)
		USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = CityGenerator, Meta = (AllowPrivateAccess = true))
		USplineComponent* Street;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = CityGenerator, Meta = (AllowPrivateAccess = true))
		USplineMeshComponent* StreetMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = CityGenerator, Meta = (AllowPrivateAccess = true))
		TArray<USplineMeshComponent*> StreetMeshComponents;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = CityGenerator, Meta = (AllowPrivateAccess = true))
		TArray<UTextRenderComponent*> StreetAddressComponents;

	UPROPERTY(EditAnywhere)
		UHierarchicalInstancedStaticMeshComponent* StreetNetwork;

	UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true, BlueprintProtected))
		bool GenerateStreets = false;

	UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
		double minLat;

	UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
		double minLon;

	UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
		double maxLon;

	UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
		double maxLat;

	UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
		float streetDistance;

	UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
		float streetAngle;

	UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
		FString streetName;

	UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
		FString FilePath;

	virtual void AddStreetSplineComponent();

	virtual void AddStreetSplineMeshComponent();

	virtual void GenerateStreetSplineMeshComponents(int32, FString);

	virtual void ClearStreetSplineMeshComponents();

	virtual void LoadMapXML();

	virtual std::vector<std::pair<double, double>> GetCoordNodes(pugi::xml_node, pugi::xml_node, double, double);

	virtual FString GetAddress(pugi::xml_node);

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
};

