#include "../XMLParser/pugixml.hpp"
#include "Helpers/CoordinateTools.h"
#include <vector>
#include "Core.h"
#include "Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Components/TextRenderComponent.h"
#include "Runtime/Engine/Classes/Components/SplineComponent.h"
#include "Runtime/Engine/Classes/Components/SplineMeshComponent.h"
#include "Runtime/Landscape/Classes/LandscapeSplinesComponent.h"
#include "Runtime/Landscape/Classes/LandscapeSplineSegment.h"
#include "Runtime/Landscape/Classes/LandscapeSplineControlPoint.h"
#include "Runtime/Engine/Classes/Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Landscape/Classes/Landscape.h"
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
	
	enum class Enum_k : uint8
	{
		highway,
		landuse,
		building,
		tunnel
	};

	Enum_k ActiveElement;
	std::string TagValues[3] = { "//way/tag[@k='highway' and (@v='tertiary' or @v='secondary' or @v='secondary_link' or @v='primary')]",
							     "//way/tag[@k='landuse']",
								 "//way/tag[@k='building']"
	};


	UPROPERTY(VisibleAnywhere)
		USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = CityGenerator, Meta = (AllowPrivateAccess = true))
		USplineComponent* Spline;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = CityGenerator, Meta = (AllowPrivateAccess = true))
		USplineMeshComponent* StreetMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = CityGenerator, Meta = (AllowPrivateAccess = true))
		USplineMeshComponent* WallMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = CityGenerator, Meta = (AllowPrivateAccess = true))
		ULandscapeSplinesComponent* LSplines;

	UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true, BlueprintProtected))
		bool GenerateStreets = false;

	UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true, BlueprintProtected))
		bool GenerateRealEstate = false;

	/*UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true, BlueprintProtected))
		bool GenerateBuildings = false;*/
	
	UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true, BlueprintProtected))
		bool ShowAddress = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = CityGenerator, Meta = (AllowPrivateAccess = true))
		TArray<USplineMeshComponent*> HighwayMeshComponents;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = CityGenerator, Meta = (AllowPrivateAccess = true))
		TArray<UTextRenderComponent*> HighwayAddressComponents;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = CityGenerator, Meta = (AllowPrivateAccess = true))
		TArray<USplineMeshComponent*> LanduseMeshComponents;

	UPROPERTY(EditAnywhere)
		UHierarchicalInstancedStaticMeshComponent* StreetNetwork;

#pragma region debugParms
	//UPROPERTY(VisibleAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
	double minLat;

	//UPROPERTY(VisibleAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
	double minLon;

	//UPROPERTY(VisibleAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
	double maxLon;

	//UPROPERTY(VisibleAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
	double maxLat;

	//UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
	float streetDistance;

	//UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
	float streetAngle;

	//UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
	FString streetName;

	//UPROPERTY(VisibleAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true))
	FString FilePath;
#pragma endregion  

	virtual void AddStreetSplineComponent();

	virtual void AddStreetSplineMeshComponent();

	virtual void AddWallSplineMeshComponent();

	virtual void GenerateStreetSplineMeshComponents(int32, FString);

	virtual void GenerateLanduseSplineMeshComponents(int32);

	virtual float FindLandscapeZ(float, float);

	virtual void ClearStreetSplineMeshComponents();

	virtual void ClearLanduseSplineMeshComponents();

	virtual void LoadMapXML();

	virtual std::vector<std::pair<double, double>> GetCoordNodes(pugi::xml_node, pugi::xml_node, double, double);

	virtual void ToggleStreetAddressComponents();

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
};

