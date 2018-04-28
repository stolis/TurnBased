#include "../XMLParser/pugixml.hpp"
#include "Helpers/CoordinateTools.h"
#include <vector>
#include "Core.h"
#include "Runtime/Engine/Classes/Components/SplineComponent.h"
#include "Runtime/Engine/Classes/Components/SplineMeshComponent.h"
#include "Runtime/Engine/Classes/Components/HierarchicalInstancedStaticMeshComponent.h"
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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components, Meta = (AllowPrivateAccess = true))
		USplineComponent* Street;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Rendering, Meta = (AllowPrivateAccess = true))
		USplineMeshComponent* StreetMesh;

	UPROPERTY(EditAnywhere)
		UHierarchicalInstancedStaticMeshComponent* StreetNetwork;

	UPROPERTY(EditAnywhere, Category = Actions, Meta = (MakeEditWidget = true, BlueprintProtected))
		bool GenerateStreets;

	UPROPERTY(EditAnywhere, Category = GeographicProperties, Meta = (MakeEditWidget = true))
		double minLat;

	UPROPERTY(EditAnywhere, Category = GeographicProperties, Meta = (MakeEditWidget = true))
		double minLon;

	UPROPERTY(EditAnywhere, Category = GeographicProperties, Meta = (MakeEditWidget = true))
		double maxLon;

	UPROPERTY(EditAnywhere, Category = GeographicProperties, Meta = (MakeEditWidget = true))
		double maxLat;

	UPROPERTY(EditAnywhere, Category = GeographicProperties, Meta = (MakeEditWidget = true))
		float streetDistance;

	UPROPERTY(EditAnywhere, Category = GeographicProperties, Meta = (MakeEditWidget = true))
		float streetAngle;

	UPROPERTY(EditAnywhere, Category = GeographicProperties, Meta = (MakeEditWidget = true))
		FString streetName;

	UPROPERTY(EditAnywhere, Category = GeographicProperties, Meta = (MakeEditWidget = true))
		FString FilePath;

	virtual void AddStreetSpline();

	virtual void AddStreetSplineMesh();

	virtual void LoadMapXML();

	virtual std::vector<std::pair<double, double>> GetCoordNodes(pugi::xml_node, pugi::xml_node, double, double);

	virtual FString GetAddress(pugi::xml_node);

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
	
};

