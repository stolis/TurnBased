// Fill out your copyright notice in the Description page of Project Settings.
#include "CityGenerator.h"

static const double earthDiameterMeters = 6371.0 * 2 * 1000;

// Sets default values
ACityGenerator::ACityGenerator()
{
 	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	ACityGenerator::AddStreetSplineComponent();
	ACityGenerator::AddStreetSplineMeshComponent();
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
			ACityGenerator::ClearStreetSplineMeshComponents();
			Street->ClearSplinePoints(true);
		}
	}
}

/*void ACityGenerator::OnConstruction(const FTransform& Transform) 
{
	ACityGenerator::AddStreetSpline();
}*/

void ACityGenerator::AddStreetSplineComponent()
{
	Street = CreateDefaultSubobject<USplineComponent>(FName(TEXT("StreetSpline")));
	Street->SetupAttachment(Root);
	Street->ClearSplinePoints(true);
	
}

void ACityGenerator::AddStreetSplineMeshComponent()
{
	StreetMesh = CreateDefaultSubobject<USplineMeshComponent>(FName(TEXT("StreetSplineMesh")));
	StreetMesh->SetupAttachment(Root);
}

void ACityGenerator::GenerateStreetSplineMeshComponents(int32 index, FString address)
{
	USplineMeshComponent* streetMesh;
	UTextRenderComponent* streetAddress;

	/*if (StreetMeshComponents.IsValidIndex(index)) {
		streetMesh = StreetMeshComponents[index];
		streetMesh->SetVisibility(true);
	}
	else {*/
		streetMesh = NewObject<USplineMeshComponent>();
		streetMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		streetMesh->SetMobility(EComponentMobility::Static);
		streetMesh->SetupAttachment(Root);
		streetMesh->RegisterComponentWithWorld(GetWorld());
		StreetMeshComponents.Add(streetMesh);
	//}

	FVector ptLocStart, ptTanStart, ptLocEnd, ptTanEnd;
	Street->GetLocalLocationAndTangentAtSplinePoint(index - 1, ptLocStart, ptTanStart);
	Street->GetLocalLocationAndTangentAtSplinePoint(index, ptLocEnd, ptTanEnd);
	UStaticMesh *sm_Street = StreetMesh->GetStaticMesh(); 
	streetMesh->SetStaticMesh(sm_Street);
	streetMesh->SetMaterial(0, StreetMesh->GetMaterial(0));
	streetMesh->SetStartAndEnd(ptLocStart, ptTanStart, ptLocEnd, ptTanEnd);
	
	streetAddress = NewObject<UTextRenderComponent>();
	streetAddress->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	streetAddress->SetMobility(EComponentMobility::Static);
	streetAddress->SetupAttachment(Root);
	streetAddress->RegisterComponentWithWorld(GetWorld());
	streetAddress->Text = FText::FromString(address);
	streetAddress->SetWorldLocation(ptLocStart);
	streetAddress->SetWorldRotation(FQuat(0, -90, 90, 1));
	streetAddress->SetWorldScale3D(FVector(10, 10, 10));
	StreetAddressComponents.Add(streetAddress);
	
}

void ACityGenerator::ClearStreetSplineMeshComponents()
{
	for (USplineMeshComponent *s : StreetMeshComponents) {
		s->DestroyComponent();
	}
	for (UTextRenderComponent *s : StreetAddressComponents) {
		s->DestroyComponent();
	}
	StreetMeshComponents.Empty();
	StreetAddressComponents.Empty();
	/*for (USplineMeshComponent* streetMeshComponent : StreetMeshComponents) {
		streetMeshComponent->SetVisibility(false);
	}*/
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
	pugi::xpath_node_set nodesTags = way.select_nodes("//way/tag[@k='highway']");
	double lat, lon, x, y;
	int32 streetIndex = 0, pointIndex = 0;
	
	for (pugi::xpath_node nodeTag : nodesTags) {
		auto wayNode = nodeTag.parent();
		const pugi::char_t *id = wayNode.attribute("id").as_string();
		pugi::xpath_variable_set vars;
		vars.add("id", pugi::xpath_type_string);
		vars.set("id", id);
		pugi::xml_node addressTag = wayNode.find_child_by_attribute("k", "int_name");
		FString address = addressTag.attribute("v").as_string();
		pugi::xpath_query query_way_nd("//way[@id=string($id)]/nd", &vars);
		pugi::xpath_node_set nodesNd = way.select_nodes(query_way_nd);
		pointIndex = 0;
		for (pugi::xpath_node nodeNd : nodesNd) {
			auto nodeFound = root.find_child_by_attribute("id", nodeNd.node().attribute("ref").as_string());
			FilePath += nodeFound.attribute("id").as_string();
			FilePath += ",";
			lat = nodeFound.attribute("lat").as_double();
			lon = nodeFound.attribute("lon").as_double();
			CoordinateTools::GeoDeticOffsetInv(refLat, refLon, lat, lon, x, y);
			Street->AddSplineWorldPoint(FVector(x, y, 0));
			if (pointIndex > 0) {
				ACityGenerator::GenerateStreetSplineMeshComponents(pointIndex, address);
			}
			coords.push_back(std::pair<double, double>(x, y));
			pointIndex++;
		}
		Street->ClearSplinePoints(false);
		streetIndex++;
		if (streetIndex == 1)
			return coords;
	}
	return coords;
}

FString ACityGenerator::GetAddress(pugi::xml_node way) 
{
	pugi::xml_node tag = way.find_child_by_attribute("k", "int_name");
	return tag.attribute("v").as_string();
}











