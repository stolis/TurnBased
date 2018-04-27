// Fill out your copyright notice in the Description page of Project Settings.
#include "CityGenerator.h"


// Sets default values
ACityGenerator::ACityGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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
	const char * mapFile = "Content/Data/OSM/Athens_Omonoia_Sample.OSM";


	if (doc.load_file(mapFile)) {
		for (pugi::xml_node node = node.child("node"); node; node = node.next_sibling("node")) {
			XMLNodes++;
		}
	}

	//Content\Data\OSM\Athens_Omonoia_Sample.OSM
}

void ACityGenerator::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	Super::PostEditChangeProperty(e);

	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ACityGenerator, GenerateStreets)) {
		/* Because you are inside the class, you should see the value already changed */
		if (this->GenerateStreets) 
			LoadMapXML(); // This is how you access MyBool.
		
	}
}





