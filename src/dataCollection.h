//Libraries for data collection
#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h> //For HTTP requests
#include "json.hpp"    //For JSON parsing
#include "apiKeys.h"

//Easy alias for JSON
using json = nlohmann::json;

//URL stuff
std::string baseURL = "https://api.themoviedb.org/3/movie";
std::string sortByAddon = "sort_by=popularity.desc";
std::string creditsAddon = "apend_to_response=credits";

//Examples of complete URLs
//
// Get's movies by popularity
// https://api.themoviedb.org/3/discover/movie?api_key={YOUR_API_KEY}&page={PAGE_NUMBER}&sort_by=popularity.desc
//
// Get's actors for a specific movie
// https://api.themoviedb.org/3/movie/{MOVIE_ID}?api_key={YOUR_API_KEY}&append_to_response=credits

//Curl Functions

//Callback function to handle data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
	size_t totalSize = size * nmemb;
	userp->append((char*)contents, totalSize);
	return totalSize;
}

