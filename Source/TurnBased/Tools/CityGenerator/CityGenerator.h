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
#include "Runtime/Landscape/Classes/LandscapeSplinesComponent.h"
#include "Runtime/Landscape/Classes/LandscapeSplineSegment.h"
#include "Runtime/Landscape/Classes/LandscapeSplineControlPoint.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "Runtime/Landscape/Classes/Landscape.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Runtime/Json/Public/Json.h"
#include "Runtime/JsonUtilities/Public/JsonUtilities.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Editor/UnrealEd/Classes/Editor/EditorEngine.h"
#include "Editor/UnrealEd/Public/FileHelpers.h"
#include "Editor/UnrealEd/Public/UnrealEd.h"
#include "Editor/UnrealEd/Public/EditorLevelUtils.h"
#include "CityGenerator.generated.h"

USTRUCT()
struct FMapChunk {
	GENERATED_BODY()
		UPROPERTY(EditAnywhere, Category = Inertia_CityGen, meta = (EditCondition = "MapIsLoaded")) bool GenerateStreets = false;
		UPROPERTY(EditAnywhere, Category = Inertia_CityGen, meta = (EditCondition = "MapIsLoaded")) bool GenerateBuildings = false;
		UPROPERTY() bool MapIsLoaded = false;
		UPROPERTY(VisibleAnywhere) double left;
		UPROPERTY(VisibleAnywhere) double bottom;
		UPROPERTY(VisibleAnywhere) double right;
		UPROPERTY(VisibleAnywhere) double top;

		UPROPERTY(VisibleAnywhere, Category = Inertia_CityGen, Meta = (AllowPrivateAccess = true))
			TArray<USplineComponent*> highwaySplineComponents;
		UPROPERTY(VisibleAnywhere, Category = Inertia_CityGen, Meta = (AllowPrivateAccess = true))
			TArray<USplineMeshComponent*> highwayMeshComponents;
		UPROPERTY(VisibleAnywhere, Category = Inertia_CityGen, Meta = (AllowPrivateAccess = true))
			TArray<UTextRenderComponent*> highwayAddressComponents;

		UPROPERTY(VisibleAnywhere, Category = Inertia_CityGen, Meta = (AllowPrivateAccess = true))
			TArray<USplineComponent*> buildingSplineComponents;
		UPROPERTY(VisibleAnywhere, Category = Inertia_CityGen, Meta = (AllowPrivateAccess = true))
			TArray<USplineMeshComponent*> buildingMeshComponents;

		UPROPERTY() FString mapChunkString;
		FString GetStringCoords() {
			return FString::SanitizeFloat(left) + "," + FString::SanitizeFloat(bottom) + "," + FString::SanitizeFloat(right) + "," + FString::SanitizeFloat(top);
		}
		FString GetFileName() {
			return FString::SanitizeFloat(left) + "_" + FString::SanitizeFloat(bottom) + "_" + FString::SanitizeFloat(right) + "_" + FString::SanitizeFloat(top) + ".OSM";
		}
		ULevel* MapLevel;
	FMapChunk() {}
	~FMapChunk() { EditorLevelUtils::RemoveLevelFromWorld(MapLevel); }
};

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
	std::string TagValues[3] = { "//way/tag[@k='highway' and (@v='tertiary' or @v='secondary' or @v='secondary_link' or @v='primary' or @v='crossing' or @v='residential' or @v='pedestrian')]",
							     "//way/tag[@k='landuse']",
								 "//way/tag[@k='building']"
	};

	UPROPERTY(EditAnywhere, Category = Inertia_CityGen, Meta = (AllowPrivateAccess = true))
		TArray<FMapChunk> MapChunks;

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

	UPROPERTY(EditAnywhere, Category = CityGenerator, Meta = (MakeEditWidget = true, BlueprintProtected))
		bool DownloadMapChunk = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = CityGenerator, Meta = (AllowPrivateAccess = true))
		TArray<USplineMeshComponent*> HighwayMeshComponents;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = CityGenerator, Meta = (AllowPrivateAccess = true))
		TArray<UTextRenderComponent*> HighwayAddressComponents;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = CityGenerator, Meta = (AllowPrivateAccess = true))
		TArray<USplineMeshComponent*> LanduseMeshComponents;

	UPROPERTY(EditAnywhere)
		UHierarchicalInstancedStaticMeshComponent* StreetNetwork;

	FMapChunk* CurrentMapChunk;

	const char * DownloadedMapChunkString;

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

	virtual void GenerateStreetSplineMeshComponents(int32, FString, USplineComponent*);

	virtual void GenerateBuildingSplineMeshComponents(int32, USplineComponent*);

	virtual float FindLandscapeZ(float, float);

	virtual void ClearStreetSplineMeshComponents();

	virtual void ClearBuildingSplineMeshComponents();

	virtual void LoadMapXML();

	virtual std::vector<std::pair<double, double>> GetCoordNodes(pugi::xml_node, pugi::xml_node);

	virtual void ToggleStreetAddressComponents();

	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& e) override;

	//virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
	//virtual void PreEditChange(UProperty* PropertyThatWillChange) override;

	void RequestMapChunk(FString);

	void MapChunkResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void PinCoordsToMapChunk(FMapChunk&, int);

private:

	//300x300.02 meter square around Omonoia
	const double leftB = 23.72624;
	const double bottomB = 37.98260;
	const double rightB = 23.72977;
	const double topB = 37.98536;
	const double horizontalDistance = rightB - leftB;
	const double verticalDistance = topB - bottomB;

	//ToDo: Use Omonoia square for reference coords or pass the boundary start
	const double refLat = 37.98414;
	const double refLon = 23.72807;

	const int rowSize = 2;
	
	const FString osmFilePath = "C:/Users/zero_/Documents/GitHub/TurnBased/Content/Data/OSM/";

	FHttpModule * Http;

	FString ApiBaseUrl = "https://api.openstreetmap.org/api/0.6/map?bbox=";

	FString AuthorizationHeader = TEXT("Authorization");
	
	void SetAuthorizationHash(FString Hash, TSharedRef<IHttpRequest>& Request);

	TSharedRef<IHttpRequest> RequestWithBoxCoords(FString Subroute);
	
	void SetRequestHeaders(TSharedRef<IHttpRequest>& Request);

	TSharedRef<IHttpRequest> GetRequest();
	
	void Send(TSharedRef<IHttpRequest>& Request);

	bool ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful);

	void AssignReponseToCurrentMapChunk(FHttpResponsePtr Response);

	void SaveToOSMFile(FString, FString);


};



