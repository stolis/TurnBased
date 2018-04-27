// Fill out your copyright notice in the Description page of Project Settings.
#include "CityGenerator.h"

static const double earthDiameterMeters = 6371.0 * 2 * 1000;

// Sets default values
ACityGenerator::ACityGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>("Root");

}

// Called when the game starts or when spawned
void ACityGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACityGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

		minLon = atof(bounds.attribute("minlon").value());
		minLat = atof(bounds.attribute("minlat").value());
		maxLon = atof(bounds.attribute("minlon").value());
		maxLat = atof(bounds.attribute("minlat").value());

		//double r = CoordinateTools::CoordinatesToMeters(minLat, minLon, maxLat, maxLon);
		streetDistance = CoordinateTools::CoordinatesToMeters(minLat, minLon, maxLat, maxLon);
	}
}

void ACityGenerator::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	Super::PostEditChangeProperty(e);

	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ACityGenerator, GenerateStreets)) {
		if (this->GenerateStreets) 
			LoadMapXML(); 
		
	}
}












