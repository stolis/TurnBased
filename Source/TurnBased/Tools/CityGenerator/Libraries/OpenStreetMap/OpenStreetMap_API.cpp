#include "OpenStreetMap_API.h"
#include <sstream>
#include "../Curl/curl.h"
#include "../Curl/easy.h"


using namespace std;

OpenStreetMap_API::OpenStreetMap_API()
{
}


OpenStreetMap_API::~OpenStreetMap_API()
{
}

string OpenStreetMap_API::DownloadMapChunkByBBox(double left, double bottom, double right, double top) {
	string downloadChunk = "https://api.openstreetmap.org/api/0.6/map?bbox=";
	/*ostringstream downloadURL;
	//downloadURL << OpenStreetMap_API::OpenStreetURL << downloadChunk;
	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.openstreetmap.org/api/0.6/map?bbox=");
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));

		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();*/
	return "";
}