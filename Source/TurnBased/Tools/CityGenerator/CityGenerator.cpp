// Fill out your copyright notice in the Description page of Project Settings.
#include "CityGenerator.h"

using namespace pugi;
using namespace std;

static const double earthDiameterMeters = 6371.0 * 2 * 1000;

// Sets default values
ACityGenerator::ACityGenerator()
{
 	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	ACityGenerator::AddStreetSplineComponent();
	ACityGenerator::AddStreetSplineMeshComponent();
	ACityGenerator::AddWallSplineMeshComponent();
}

void ACityGenerator::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	Super::PostEditChangeProperty(e);

	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ACityGenerator, GenerateStreets)) {
		if (this->GenerateStreets) {
			ActiveElement = Enum_k::highway;
			LoadMapXML();
		}
		else {
			ClearStreetSplineMeshComponents();
			Spline->ClearSplinePoints(true);
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ACityGenerator, GenerateRealEstate)) {
		if (this->GenerateRealEstate) {
			ActiveElement = Enum_k::building;
			LoadMapXML();
		}
		else {
			ClearLanduseSplineMeshComponents();
			Spline->ClearSplinePoints(true);
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ACityGenerator, ShowAddress)) {
		ToggleStreetAddressComponents();
	}
}

void ACityGenerator::AddStreetSplineComponent()
{
	Spline = CreateDefaultSubobject<USplineComponent>(FName(TEXT("StreetSpline")));
	Spline->SetupAttachment(Root);
	Spline->ClearSplinePoints(true);
	Spline->SetClosedLoop(false);
}

void ACityGenerator::AddStreetSplineMeshComponent()
{
	StreetMesh = CreateDefaultSubobject<USplineMeshComponent>(FName(TEXT("StreetSplineMesh")));
	StreetMesh->SetupAttachment(Root);
}

void ACityGenerator::AddWallSplineMeshComponent()
{
	WallMesh = CreateDefaultSubobject<USplineMeshComponent>(FName(TEXT("WallSplineMesh")));
	WallMesh->SetupAttachment(Root);
}

void ACityGenerator::GenerateStreetSplineMeshComponents(int32 index, FString address)
{
	USplineMeshComponent* streetMesh;
	UTextRenderComponent* streetAddress;

	streetMesh = NewObject<USplineMeshComponent>();
	streetMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	streetMesh->SetMobility(EComponentMobility::Static);
	streetMesh->SetupAttachment(Root);
	streetMesh->RegisterComponentWithWorld(GetWorld());
	HighwayMeshComponents.Add(streetMesh);
	FVector ptLocStart, ptTanStart, ptLocEnd, ptTanEnd;
	Spline->GetLocalLocationAndTangentAtSplinePoint(index - 1, ptLocStart, ptTanStart);
	Spline->GetLocalLocationAndTangentAtSplinePoint(index, ptLocEnd, ptTanEnd);
	UStaticMesh *sm_Street = StreetMesh->GetStaticMesh(); 
	streetMesh->SetStaticMesh(sm_Street);
	streetMesh->SetMaterial(0, StreetMesh->GetMaterial(0));
	streetMesh->SetStartAndEnd(ptLocStart, ptTanStart, ptLocEnd, ptTanEnd);
	
	streetAddress = NewObject<UTextRenderComponent>();
	streetAddress->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	streetAddress->SetMobility(EComponentMobility::Static);
	streetAddress->SetupAttachment(Root);
	streetAddress->RegisterComponentWithWorld(GetWorld());
	streetAddress->SetText(FText::FromString(address));
	streetAddress->SetWorldLocation(ptLocStart);
	streetAddress->SetWorldRotation(FQuat(0, -90, 90, 1));
	streetAddress->SetWorldScale3D(FVector(10, 10, 10));
	HighwayAddressComponents.Add(streetAddress);
	
}

void ACityGenerator::GenerateLanduseSplineMeshComponents(int32 index) {
	USplineMeshComponent* landuseMesh;

	landuseMesh = NewObject<USplineMeshComponent>();
	landuseMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	landuseMesh->SetMobility(EComponentMobility::Static);
	landuseMesh->SetupAttachment(Root);
	landuseMesh->RegisterComponentWithWorld(GetWorld());
	landuseMesh->SetRelativeScale3D(FVector(1,1,1));
	LanduseMeshComponents.Add(landuseMesh);
	FVector ptLocStart, ptTanStart, ptLocEnd, ptTanEnd;
	Spline->GetLocalLocationAndTangentAtSplinePoint(index - 1, ptLocStart, ptTanStart);
	Spline->GetLocalLocationAndTangentAtSplinePoint(index, ptLocEnd, ptTanEnd);
	UStaticMesh *sm_LandusePart = WallMesh->GetStaticMesh();
	landuseMesh->SetStaticMesh(sm_LandusePart);
	landuseMesh->SetMaterial(0, StreetMesh->GetMaterial(0));
	landuseMesh->SetStartAndEnd(ptLocStart, ptTanStart, ptLocEnd, ptTanEnd);
}

void ACityGenerator::ToggleStreetAddressComponents() 
{
	for (UTextRenderComponent *s : HighwayAddressComponents) {
		s->ToggleVisibility();
	}
}

void ACityGenerator::ClearStreetSplineMeshComponents()
{
	for (USplineMeshComponent *s : HighwayMeshComponents) {
		if (s != nullptr)
			s->DestroyComponent();
	}
	for (UTextRenderComponent *s : HighwayAddressComponents) {
		if (s != nullptr)
			s->DestroyComponent();
	}
	HighwayMeshComponents.Empty();
	HighwayAddressComponents.Empty();
}

void ACityGenerator::ClearLanduseSplineMeshComponents()
{
	for (USplineMeshComponent *s : LanduseMeshComponents) {
		if (s != nullptr)
			s->DestroyComponent();
	}
	LanduseMeshComponents.Empty();
}

void ACityGenerator::LoadMapXML()
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetVerb(TEXT("GET"));
	Request->SetURL("https://api.openstreetmap.org/api/0.6/map?bbox=23.72603,37.98262,23.73050,37.98569");
	Request->SetHeader("Content-Type", "application/xml");
	Request->SetContentAsString(ReceivedData);
	Request->OnProcessRequestComplete().BindUObject(this, &ACityGenerator::CompletedHTTPRequest);
	
	if (!Request->ProcessRequest())
	{
		auto fail = "asdfasfd";
		// HTTP request failed
	}

	pugi::xml_document doc;
	FString contentDir = FPaths::ProjectContentDir();
	FString mapFile = contentDir + FString(TEXT("/Data/OSM/Athens_Omonoia_Sample5.osm"));
	const char * getFrom = TCHAR_TO_ANSI(*mapFile);
	
	if (doc.load_file(getFrom)) {
		pugi::xml_node root = doc.child("osm");
		pugi::xml_node bounds = root.child("bounds");
		pugi::xml_node way = root.child("way");

		minLon = bounds.attribute("minlon").as_double();
		minLat = bounds.attribute("minlat").as_double();
		maxLon = bounds.attribute("maxlon").as_double();
		maxLat = bounds.attribute("maxlat").as_double();

		const auto latitude1 = minLat, longitude1 = minLon,
			latitude2 = maxLat, longitude2 = maxLon;

		GetCoordNodes(root, way, minLat, minLon);
	}
}

vector<pair<double, double>> ACityGenerator::GetCoordNodes(xml_node root, xml_node way, double refLat, double refLon)
{
	//ToDo: Use Omonoia square for reference coords or pass the boundary start
	refLat = 37.98414;
	refLon = 23.72807;
	double lat, lon, x, y;
	int32 streetIndex = 0, pointIndex = 0;
	vector<pair<double, double>> coords;

	xpath_node_set nodesTags = way.select_nodes(TagValues[(int)ActiveElement].c_str());
	
	
	for (xpath_node nodeTag : nodesTags) {
		auto wayNode = nodeTag.parent();
		const char_t *id = wayNode.attribute("id").as_string();
		xpath_variable_set vars;
		vars.add("id", xpath_type_string);
		vars.set("id", id);
		xml_node addressTag = wayNode.find_child_by_attribute("k", "int_name");
		FString address = addressTag.attribute("v").as_string();
		xpath_query query_way_nd("//way[@id=string($id)]/nd", &vars);
		xpath_node_set nodesNd = way.select_nodes(query_way_nd);
		pointIndex = 0;
		for (xpath_node nodeNd : nodesNd) {
			auto nodeFound = root.find_child_by_attribute("id", nodeNd.node().attribute("ref").as_string());
			FilePath += nodeFound.attribute("id").as_string();
			FilePath += ",";
			lat = nodeFound.attribute("lat").as_double();
			lon = nodeFound.attribute("lon").as_double();
			CoordinateTools::GeoDeticOffsetInv(refLat, refLon, lat, lon, x, y);
			float z = FindLandscapeZ(x*1.5, -y*1.5);
			Spline->AddSplineWorldPoint(FVector(x*1.5, -y*1.5, z));
			if (ActiveElement == Enum_k::building)
				Spline->SetSplinePointType(pointIndex, ESplinePointType::Linear, true);
			if (pointIndex > 0) {
				switch (ActiveElement) {
				case Enum_k::highway: {
					Spline->SetSplinePointType(pointIndex, ESplinePointType::Curve, true);
					GenerateStreetSplineMeshComponents(pointIndex, address); 
					break;
				}
				case Enum_k::building: {
					Spline->SetSplinePointType(pointIndex, ESplinePointType::Linear, true);
					GenerateLanduseSplineMeshComponents(pointIndex);
					break;
				}
				default:break;
				}
				
			}
			coords.push_back(pair<double, double>(x, y));
			pointIndex++;
		}
		Spline->ClearSplinePoints(false);
		streetIndex++;
	}
	return coords;
}

float ACityGenerator::FindLandscapeZ(float x, float y) {
	FVector rayStart = FVector(x, y, 1000);
	FVector rayEnd = FVector(x, y, -1000);

	UWorld* world = GetWorld();
	if (world == nullptr)
		return 0;

	TActorIterator<ALandscape> landscapeIterator(world);
	ALandscape *landscape = *landscapeIterator;

	FCollisionQueryParams collisionParams(FName(TEXT("StreetClusterPlacementTrace")), true, this);
	collisionParams.bReturnPhysicalMaterial = true;

	FHitResult hit(ForceInit);
	if (landscape->ActorLineTraceSingle(hit, rayStart, rayEnd, ECC_Visibility, collisionParams)) {
		return hit.ImpactPoint.Z;
	}
	else return 0;
}

void ACityGenerator::CompletedHTTPRequest(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful) {
	if (!response.IsValid())
	{
		// no valid response
	}
	else if (EHttpResponseCodes::IsOk(response->GetResponseCode()))
	{
		// valid response
		FString msg = response->GetContentAsString();
		FString data = ReceivedData;
	}
	else
	{
		// HTTP request error
	}
	FString msg = response->GetContentAsString();

}











