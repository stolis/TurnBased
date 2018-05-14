// Fill out your copyright notice in the Description page of Project Settings.
#include "CityGenerator.h"
//#include "Libraries/OpenStreetMap/OpenStreetMap_API.h"

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
	//CurrentMapChunk.mapChunkCoords = "23.72624,37.98260,23.72977,37.98536";
	Http = &FHttpModule::Get();
}

void ACityGenerator::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& e)
{
	const FString& fieldCat = e.Property->GetMetaData(TEXT("Category"));

	if (fieldCat == "Inertia_CityGen") {
		int32 index = e.GetArrayIndex(TEXT("MapChunks")); //checks skipped
		UE_LOG(LogTemp, Warning, TEXT("MapChunks index is: %d"), index);

		if (MapChunks.IsValidIndex(index)) {
			FMapChunk& fMapChunk = MapChunks[index];

			if (e.ChangeType == EPropertyChangeType::ArrayAdd) {
				PinCoordsToMapChunk(fMapChunk, index);
				CurrentMapChunk = &fMapChunk;
				RequestMapChunk(CurrentMapChunk->GetFileName());
				UWorld* editorWorld = GEditor->GetEditorWorldContext().World();
				if (editorWorld)
				{
					FString preppedName = L"/Game/Maps/SubLevels/Level_Block_" + FString::FromInt(index);
					if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*(preppedName))) {
						ULevelStreaming* levelStreaming = EditorLevelUtils::AddLevelToWorld(editorWorld, *preppedName, ULevelStreamingKismet::StaticClass());
						levelStreaming->bShouldBeLoaded = true;
						CurrentMapChunk->MapLevel = levelStreaming->GetLoadedLevel();
						CurrentMapChunk->Root = NewObject<USceneComponent>(CurrentMapChunk->MapLevel->GetWorld());
						CurrentMapChunk->Root->DetachFromParent(true);
						CurrentMapChunk->Root->RegisterComponentWithWorld(CurrentMapChunk->MapLevel->GetWorld());
						//levelStreaming->BroadcastLevelLoadedStatus(editorWorld, FName(*preppedName), true);
					}
					else {
						ULevelStreaming* levelStreaming = EditorLevelUtils::CreateNewStreamingLevelForWorld(*editorWorld, ULevelStreamingKismet::StaticClass(), preppedName);
						levelStreaming->bShouldBeLoaded = true;
						if (levelStreaming->GetLoadedLevel())
						{
							FEditorFileUtils::SaveLevel(levelStreaming->GetLoadedLevel());
							CurrentMapChunk->MapLevel = levelStreaming->GetLoadedLevel();
							CurrentMapChunk->Root = NewObject<USceneComponent>(CurrentMapChunk->MapLevel->GetWorld());
							CurrentMapChunk->Root->DetachFromParent(true);
							CurrentMapChunk->Root->RegisterComponentWithWorld(CurrentMapChunk->MapLevel->GetWorld());
							//levelStreaming->BroadcastLevelLoadedStatus(editorWorld, FName(*preppedName), true);
						}
					}

				}
			}
			else if (e.ChangeType == EPropertyChangeType::ArrayRemove) {

			}
			else if (e.ChangeType == EPropertyChangeType::ArrayClear) {

			}
			else if (e.ChangeType == EPropertyChangeType::ValueSet) {
				FString propertyName = (e.Property != NULL) ? e.Property->GetNameCPP() : "";
				if (propertyName == "GenerateStreets") {
					ActiveElement = Enum_k::highway;
					if (fMapChunk.GenerateStreets) {
						CurrentMapChunk = &fMapChunk;
						LoadMapXML();
						UE_LOG(LogTemp, Warning, TEXT("GenereateStreets is: true"), "");
					}
					else {
						CurrentMapChunk = &fMapChunk;
						ClearStreetSplineMeshComponents();
						UE_LOG(LogTemp, Warning, TEXT("GenereateStreets is false"), "");
					}
				}
				if (propertyName == "GenerateBuildings") {
					ActiveElement = Enum_k::building;
					if (fMapChunk.GenerateBuildings) {
						CurrentMapChunk = &fMapChunk;
						LoadMapXML();
						UE_LOG(LogTemp, Warning, TEXT("GenereateBuildings is: true"), "");
					}
					else {
						CurrentMapChunk = &fMapChunk;
						ClearBuildingSplineMeshComponents();
						UE_LOG(LogTemp, Warning, TEXT("GenereateBuildings is false"), "");
					}
				}
			}
			Super::PostEditChangeChainProperty(e);
		}
	}
}

/*void ACityGenerator::PreEditChange(UProperty* PropertyThatWillChange)
		{

			FName dd = PropertyThatWillChange->GetFName();
			FMapChunk* toBeDeletedChunk = Cast<FMapChunk>(&PropertyThatWillChange);
			EditorLevelUtils::RemoveLevelFromWorld(toBeDeletedChunk->MapLevel);
			Super::PreEditChange(PropertyThatWillChange);
		}
		/*void ACityGenerator::PostEditChangeProperty(struct FPropertyChangedEvent& e)
		{
			Super::PostEditChangeProperty(e);

			FName memberPropertyName = (e.Property != NULL) ? e.MemberProperty->GetFName() : NAME_None;
			if (memberPropertyName == GET_MEMBER_NAME_CHECKED(ACityGenerator, GenerateStreets)) {
				if (this->GenerateStreets) {
					ActiveElement = Enum_k::highway;
					LoadMapXML();
				}
				else {
					ClearStreetSplineMeshComponents();
					Spline->ClearSplinePoints(true);
				}
			}
			else if (memberPropertyName == GET_MEMBER_NAME_CHECKED(ACityGenerator, GenerateRealEstate)) {
				if (this->GenerateRealEstate) {
					ActiveElement = Enum_k::building;
					LoadMapXML();
				}
				else {
					ClearBuildingSplineMeshComponents();
					Spline->ClearSplinePoints(true);
				}
			}
			else if (memberPropertyName == GET_MEMBER_NAME_CHECKED(ACityGenerator, ShowAddress)) {
				ToggleStreetAddressComponents();
			}
		}*/

void ACityGenerator::PinCoordsToMapChunk(FMapChunk& fMapChunk, int index) {
	double moveToBottomDelta = 0;
	double moveToRightDelta = 0;

	moveToBottomDelta = verticalDistance * (index / 2);
    moveToRightDelta = horizontalDistance * (index % rowSize);

	fMapChunk.left = leftB + moveToRightDelta;
	fMapChunk.right = fMapChunk.left + horizontalDistance;
	fMapChunk.top = topB + moveToBottomDelta;
	fMapChunk.bottom = fMapChunk.top - verticalDistance;
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

void ACityGenerator::GenerateStreetSplineMeshComponents(int32 index, FString address, USplineComponent* spline)
{
	USplineMeshComponent* streetMesh;
	UTextRenderComponent* streetAddress;

	streetMesh = NewObject<USplineMeshComponent>();
	streetMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	streetMesh->SetMobility(EComponentMobility::Static);
	//CurrentMapChunk->Root->Rename(CurrentMapChunk->MapLevel->StaticPackage(), CurrentMapChunk->MapLevel->GetWorld());
	streetMesh->SetupAttachment(CurrentMapChunk->Root);
	streetMesh->RegisterComponentWithWorld(CurrentMapChunk->MapLevel->GetWorld());
	CurrentMapChunk->highwayMeshComponents.Add(streetMesh);
	FVector ptLocStart, ptTanStart, ptLocEnd, ptTanEnd;
	spline->GetLocalLocationAndTangentAtSplinePoint(index - 1, ptLocStart, ptTanStart);
	spline->GetLocalLocationAndTangentAtSplinePoint(index, ptLocEnd, ptTanEnd);
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
	//HighwayAddressComponents.Add(streetAddress);
	CurrentMapChunk->highwayAddressComponents.Add(streetAddress);
	
}

void ACityGenerator::GenerateBuildingSplineMeshComponents(int32 index, USplineComponent* spline) {
	USplineMeshComponent* buildingMesh;

	buildingMesh = NewObject<USplineMeshComponent>();
	buildingMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	buildingMesh->SetMobility(EComponentMobility::Static);
	buildingMesh->SetupAttachment(Root);
	buildingMesh->RegisterComponentWithWorld(GetWorld());
	buildingMesh->SetRelativeScale3D(FVector(1,1,1));
	CurrentMapChunk->buildingMeshComponents.Add(buildingMesh);
	FVector ptLocStart, ptTanStart, ptLocEnd, ptTanEnd;
	spline->GetLocalLocationAndTangentAtSplinePoint(index - 1, ptLocStart, ptTanStart);
	spline->GetLocalLocationAndTangentAtSplinePoint(index, ptLocEnd, ptTanEnd);
	UStaticMesh *sm_LandusePart = WallMesh->GetStaticMesh();
	buildingMesh->SetStaticMesh(sm_LandusePart);
	buildingMesh->SetMaterial(0, StreetMesh->GetMaterial(0));
	buildingMesh->SetStartAndEnd(ptLocStart, ptTanStart, ptLocEnd, ptTanEnd);
}

void ACityGenerator::ToggleStreetAddressComponents() 
{
	for (UTextRenderComponent *s : HighwayAddressComponents) {
		s->ToggleVisibility();
	}
}

void ACityGenerator::ClearStreetSplineMeshComponents()
{
	for (USplineComponent *s : CurrentMapChunk->highwaySplineComponents) {
		if (s != nullptr)
			s->DestroyComponent();
	}
	for (USplineMeshComponent *s : CurrentMapChunk->highwayMeshComponents) {
		if (s != nullptr)
			s->DestroyComponent();
	}
	for (UTextRenderComponent *s : CurrentMapChunk->highwayAddressComponents) {
		if (s != nullptr)
			s->DestroyComponent();
	}
	CurrentMapChunk->highwaySplineComponents.Empty();
	CurrentMapChunk->highwayMeshComponents.Empty();
	CurrentMapChunk->highwayAddressComponents.Empty();
}

void ACityGenerator::ClearBuildingSplineMeshComponents()
{
	for (USplineComponent *s : CurrentMapChunk->buildingSplineComponents) {
		if (s != nullptr)
			s->DestroyComponent();
	}
	for (USplineMeshComponent *s : CurrentMapChunk->buildingMeshComponents) {
		if (s != nullptr)
			s->DestroyComponent();
	}
	CurrentMapChunk->buildingSplineComponents.Empty();
	CurrentMapChunk->buildingMeshComponents.Empty();
}

void ACityGenerator::LoadMapXML()
{
	xml_document doc;
	FString filePathName = osmFilePath + CurrentMapChunk->GetFileName();
	FString fileContent;
	
	string getFrom = string(TCHAR_TO_ANSI(*CurrentMapChunk->mapChunkString));
	if (doc.load_string(getFrom.c_str())) {
		xml_node root = doc.child("osm");
		xml_node way = root.child("way");
		GetCoordNodes(root, way);
	}
	
}

vector<pair<double, double>> ACityGenerator::GetCoordNodes(xml_node root, xml_node way)
{
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
		USplineComponent* spline = NewObject<USplineComponent>();
		spline->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		spline->SetClosedLoop(false);
		spline->SetupAttachment(Root);
		spline->RegisterComponentWithWorld(GetWorld());
		spline->ClearSplinePoints();
		pointIndex = 0;
		for (xpath_node nodeNd : nodesNd) {
			auto nodeFound = root.find_child_by_attribute("id", nodeNd.node().attribute("ref").as_string());
			FilePath += nodeFound.attribute("id").as_string();
			FilePath += ",";
			lat = nodeFound.attribute("lat").as_double();
			lon = nodeFound.attribute("lon").as_double();
			CoordinateTools::GeoDeticOffsetInv(refLat, refLon, lat, lon, x, y);
			float z = FindLandscapeZ(x*1.5, -y*1.5);
			spline->AddSplineWorldPoint(FVector(x*1.5, -y*1.5, z));
			if (ActiveElement == Enum_k::building)
				spline->SetSplinePointType(pointIndex, ESplinePointType::Linear, true);
			if (pointIndex > 0) {
				switch (ActiveElement) {
				case Enum_k::highway: {
					spline->SetSplinePointType(pointIndex, ESplinePointType::Curve, true);
					GenerateStreetSplineMeshComponents(pointIndex, address, spline); 
					break;
				}
				case Enum_k::building: {
					spline->SetSplinePointType(pointIndex, ESplinePointType::Linear, true);
					GenerateBuildingSplineMeshComponents(pointIndex, spline);
					break;
				}
				default:break;
				}
				
			}
			coords.push_back(pair<double, double>(x, y));
			pointIndex++;
		}
		CurrentMapChunk->highwaySplineComponents.Add(spline);
		//Spline->ClearSplinePoints(false);
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

#pragma region OpenStreetAPI

TSharedRef<IHttpRequest> ACityGenerator::RequestWithBoxCoords(FString box_coords) {
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetURL(ApiBaseUrl + box_coords);
	SetRequestHeaders(Request);
	return Request;
}

void ACityGenerator::SetRequestHeaders(TSharedRef<IHttpRequest>& Request) {
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accepts"), TEXT("application/json"));
}

TSharedRef<IHttpRequest> ACityGenerator::GetRequest() {
	TSharedRef<IHttpRequest> Request = RequestWithBoxCoords(CurrentMapChunk->GetStringCoords());
	Request->SetVerb("GET");
	return Request;
}

void ACityGenerator::Send(TSharedRef<IHttpRequest>& Request) {
	Request->ProcessRequest();
}

bool ACityGenerator::ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful) {
	if (!bWasSuccessful || !Response.IsValid()) return false;
	if (EHttpResponseCodes::IsOk(Response->GetResponseCode())) return true;
	else {
		UE_LOG(LogTemp, Warning, TEXT("Http Response returned error code: %d"), Response->GetResponseCode());
		return false;
	}
}

void ACityGenerator::SetAuthorizationHash(FString Hash, TSharedRef<IHttpRequest>& Request) {
	Request->SetHeader(AuthorizationHeader, Hash);
}

void ACityGenerator::AssignReponseToCurrentMapChunk(FHttpResponsePtr Response) {
	FString JsonString = Response->GetContentAsString();
	CurrentMapChunk->mapChunkString = JsonString;
	UE_LOG(LogTemp, Warning, TEXT("Map is Loaded from Web!!!!"), "");
	CurrentMapChunk->MapIsLoaded = true;
	SaveToOSMFile(JsonString, CurrentMapChunk->GetFileName());
}

void ACityGenerator::RequestMapChunk(FString filename) {
	FString fileContent;
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*(osmFilePath + filename))) {
		FFileHelper::LoadFileToString(fileContent, *(osmFilePath + filename));
		CurrentMapChunk->mapChunkString = fileContent;
		UE_LOG(LogTemp, Warning, TEXT("Map is Loaded from File!!!!"), "");
		CurrentMapChunk->MapIsLoaded = true;
	}
	else {
		TSharedRef<IHttpRequest> Request = GetRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &ACityGenerator::MapChunkResponse);
		Send(Request);
	}
}

void ACityGenerator::MapChunkResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
	if (!ResponseIsValid(Response, bWasSuccessful)) return;
	AssignReponseToCurrentMapChunk(Response);
	//UE_LOG(LogTemp, Warning, TEXT("MapChunk is: %s"), string(TCHAR_TO_ANSI(*CurrentMapChunk->mapChunkString)).c_str());
}

void ACityGenerator::SaveToOSMFile(FString content, FString fileName) {
	FString SaveDirectory = FString("C:/Users/zero_/Documents/GitHub/TurnBased/Content/Data/OSM");
	
	bool AllowOverwriting = false;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
	{
		FString AbsoluteFilePath = SaveDirectory + "/" + fileName;
		FFileHelper::SaveStringToFile(content, *AbsoluteFilePath);
	}
}

#pragma endregion










