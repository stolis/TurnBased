// Fill out your copyright notice in the Description page of Project Settings.
#include "CityGenerator.h"

static const double earthDiameterMeters = 6371.0 * 2 * 1000;

// Sets default values
ACityGenerator::ACityGenerator()
{
 	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	ACityGenerator::AddStreetSpline();
	ACityGenerator::AddStreetSplineMesh();
}

void ACityGenerator::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	Super::PostEditChangeProperty(e);

	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ACityGenerator, GenerateStreets)) {
		if (this->GenerateStreets) {
			LoadMapXML();
		}
		else {
			Street->ClearSplinePoints(true);
		}
	}
}

/*void ACityGenerator::OnConstruction(const FTransform& Transform) 
{
	ACityGenerator::AddStreetSpline();
}*/

void ACityGenerator::AddStreetSpline()
{
	Street = CreateDefaultSubobject<USplineComponent>(FName(TEXT("StreetSpline")));
	Street->SetupAttachment(Root);
	Street->ClearSplinePoints(true);
	
}

void ACityGenerator::AddStreetSplineMesh()
{
	StreetMesh = CreateDefaultSubobject<USplineMeshComponent>(FName(TEXT("StreetSplineMesh")));
	StreetMesh->SetupAttachment(Root);
}

void ACityGenerator::LoadMapXML()
{
	pugi::xml_document doc;
	FString contentDir = FPaths::ProjectContentDir();
	FString mapFile = contentDir + FString(TEXT("/Data/OSM/Athens_Omonoia_Sample.osm"));
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

		streetDistance = CoordinateTools::CoordinatesToMeters(minLat, minLon, maxLat, maxLon);
		streetAngle = CoordinateTools::CoordinatesToAngle(latitude1, longitude1, latitude2, longitude2);
		streetName = ACityGenerator::GetAddress(way);
		ACityGenerator::GetCoordNodes(root, way, minLat, minLon);
	}
}

std::vector<std::pair<double, double>> ACityGenerator::GetCoordNodes(pugi::xml_node root, pugi::xml_node way, double refLat, double refLon)
{
	std::vector<std::pair<double, double>> coords;
	pugi::xpath_node_set nodesRefs = way.select_nodes("//way[@id='10741722']/nd");
	//pugi::xpath_node_set nodesRefs = way.select_nodes("//way/nd");
	double lat, lon, x, y;
	int32 index = 0;
	for (pugi::xpath_node nd : nodesRefs) {
		auto nodeFound = root.find_child_by_attribute("id", nd.node().attribute("ref").as_string());
		FilePath += nodeFound.attribute("id").as_string();
		FilePath += ",";
		lat = nodeFound.attribute("lat").as_double();
		lon = nodeFound.attribute("lon").as_double();
		CoordinateTools::GeoDeticOffsetInv(refLat, refLon, lat, lon, x, y);
		Street->AddSplineWorldPoint(FVector(x, y, 0));
		coords.push_back(std::pair<double, double>(x, y));
		index++;
	}
	return coords;
}

FString ACityGenerator::GetAddress(pugi::xml_node way) 
{
	pugi::xml_node tag = way.find_child_by_attribute("k", "int_name");
	return tag.attribute("v").as_string();
}
















