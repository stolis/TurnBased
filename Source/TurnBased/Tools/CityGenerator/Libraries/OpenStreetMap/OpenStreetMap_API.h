#pragma once
#include <iostream>

class TURNBASED_API OpenStreetMap_API
{

private:
	std::string OpenStreetURL = "https://api.openstreetmap.org/";

public:
	OpenStreetMap_API();
	~OpenStreetMap_API();
	static std::string DownloadMapChunkByBBox(double, double, double, double);
};

