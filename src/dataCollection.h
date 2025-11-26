//Libraries for data collection
#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h> //For HTTP requests
#include <nlohmann/json.hpp>    //For JSON parsing
#include <SQLiteCpp/SQLiteCpp.h> //For SQLite database
//A bunch of C++ built in libraries
#include <fstream>
#include <stdexcept> 
#include <filesystem>
#include <utility>
#include <cstdlib>
#include <thread>

//Easy alias for JSON
using json = nlohmann::json;

// URL stuff
// baseURL = "https://api.themoviedb.org/3/";
// discoverMovieAddon = "discover/movie?";
// actorDetailsAddon = "person/{PERSON_ID}?";
// sortByAddon = "sort_by=popularity.desc";
// creditsAddon = "apend_to_response=credits";
// 
//Examples of complete URLs
//
// Get's movies by popularity
// https://api.themoviedb.org/3/discover/movie?api_key=43220ed9cc8b8898d0671739929f87e0&include_adult=false&include_video=false&language=en-US&page=
// https://api.themoviedb.org/3/discover/movie?include_adult=false&include_video=false&language=en-US&page=1&sort_by=popularity.desc&year=1900'
//
// Get's actors for a specific movie
// https://api.themoviedb.org/3/movie/{MOVIE_ID}?api_key={YOUR_API_KEY}&append_to_response=credits
//
// Get's details about a specific actor
// https://api.themoviedb.org/3/person/{PERSON_ID}?api_key={YOUR_API_KEY}

// Database Section
//This setup Database is for collecting data from TMDB
void setupDatabase(SQLite::Database& db) {
	try {
		// 1. Movies Table (List of all movies)
		db.exec("CREATE TABLE IF NOT EXISTS Movies ("
			"movie_id INTEGER PRIMARY KEY,"
			"title TEXT NOT NULL"
			");");

		// 2. Actors Table (Unique list of all actors)
		db.exec("CREATE TABLE IF NOT EXISTS Actors ("
			"actor_id INTEGER PRIMARY KEY,"
			"actor_name TEXT NOT NULL"
			");");

		// 3. Cast_Links Table
		db.exec("CREATE TABLE IF NOT EXISTS Cast_Links ("
			"movie_id INTEGER NOT NULL,"
			"actor_id INTEGER NOT NULL,"
			"FOREIGN KEY (movie_id) REFERENCES Movies(movie_id),"
			"FOREIGN KEY (actor_id) REFERENCES Actors(actor_id),"
			"PRIMARY KEY (movie_id, actor_id)"
			");");
	}
	catch (const std::exception& e) {
		std::cerr << "Database error: " << e.what() << std::endl;
	}
}


//Curl Functions

//Callback function to handle data
size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
	size_t totalSize = size * nmemb;
	userp->append((char*)contents, totalSize);
	return totalSize;
}

std::string curlRequest(const std::string& url) {
	CURL* curl;
	CURLcode res;
	std::string buffer;
	curl = curl_easy_init();
	if (curl) {
		if (!(curl_version_info(CURLVERSION_NOW)->features & CURL_VERSION_SSL)) {
			std::cerr << "\nFATAL cURL ERROR: SSL support is missing. Cannot use HTTPS. "
				<< "Ensure cURL is built with SCHANNEL or OpenSSL.\n" << std::endl;
			curl_easy_cleanup(curl);
			return "";
		}
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			std::cerr << "curl failed: " << curl_easy_strerror(res) << std::endl;
		}
		curl_easy_cleanup(curl);
	} else {
		std::cerr << "Failed to initialize CURL." << std::endl;
		buffer.clear(); 
	}
	return buffer;
}

//Build URLS

std::string buildDiscoverURL(int pageNumber, int year) {
	std::string finalURL = "https://api.themoviedb.org/3/discover/movie?api_key=43220ed9cc8b8898d0671739929f87e0&include_adult=false&include_video=false&language=en-US&page=";
	finalURL += std::to_string(pageNumber);
	finalURL += "&year=";
	finalURL += std::to_string(year);
	return finalURL;
}

std::string buildMovieURL(int movieID) {
	std::string finalURL = "https://api.themoviedb.org/3/movie/";
	finalURL += std::to_string(movieID);
	finalURL += "?api_key=43220ed9cc8b8898d0671739929f87e0&append_to_response=credits";
	return finalURL;
}

//Add Actor URL builder later here for image urls


void saveMovieData(SQLite::Database & db, int movieID, const std::string & title, const json & castArray) {
	db.exec("BEGIN TRANSACTION;");
	try {
		SQLite::Statement stmtMovie(db, "INSERT OR IGNORE INTO Movies (movie_id, title) VALUES (?, ?);");
		SQLite::Statement stmtActor(db, "INSERT OR IGNORE INTO Actors (actor_id, actor_name) VALUES (?, ?);");
		SQLite::Statement stmtLink(db, "INSERT INTO Cast_Links (movie_id, actor_id) VALUES (?, ?) ON CONFLICT DO NOTHING;");
		stmtMovie.bind(1, movieID);
		stmtMovie.bind(2, title);
		stmtMovie.exec();
		for (const auto& actor : castArray) {
			int actorID = actor.value("id", -1);
			std::string actorName = actor.value("name", "N/A");
			if (actorID != -1) {
				std::cout << "Saving Actor: " << actorName << " (ID: " << actorID << ") for Movie ID: " << movieID << std::endl;
				stmtActor.bind(1, actorID);
				stmtActor.bind(2, actorName);					
				stmtActor.exec();
				stmtActor.reset();
				stmtLink.bind(1, movieID);
				stmtLink.bind(2, actorID);
				stmtLink.reset();
			}
		}
		db.exec("COMMIT;");
	}
	catch (const std::exception& e) {
		db.exec("ROLLBACK;");
		std::cerr << "Database Transaction Failed (Movie ID " << movieID << "): " << e.what() << std::endl;
	}
}

void processMovie(SQLite::Database& db, int movieID) {
	std::string movieURL = buildMovieURL(movieID);
	std::string movieResponse = curlRequest(movieURL);
	try {
		json movieData = json::parse(movieResponse);
		std::string title = movieData.value("title", "N/A");
		json castArray = movieData.value("credits", json::object()).value("cast", json::array());
		saveMovieData(db, movieID, title, castArray);
	}
	catch (json::parse_error& e) {
		std::cerr << "JSON Parse Error for Movie ID " << movieID << ": " << e.what() << std::endl;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void extractMovieIDs(SQLite::Database& db, const std::string& json_response) {
	try {
		json parsed = json::parse(json_response);
		if (parsed.contains("results")) {
			for (const auto& movie : parsed["results"]) {
				if (movie.contains("id")) {
					int movieID = movie["id"];
					if (movieID != -1) {
						processMovie(db, movieID);
					}
					else {
						std::cerr << "Invalid movie ID found." << std::endl;
					}
				}
			}
		}
	}
	catch (json::parse_error& e) {
		std::cerr << "JSON Parse Error: " << e.what() << std::endl;
	}
}

void runCollectionLoop(SQLite::Database& db) {
	const int maxPages = 500;   //Limit to 500 per year
	const int startYear = 1944;//Starting for collection
	const int currYear = 2025;//Current year for collection
	for (int year = startYear; year <= currYear; ++year) {
		std::cout << "\n==========================================================" << std::endl;
		std::cout << "STARTING COLLECTION FOR YEAR: " << year << std::endl;
		std::cout << "==========================================================" << std::endl;
		bool keepFetching = true;
		for (int page = 1; page <= maxPages && keepFetching; ++page) {
			std::string url = buildDiscoverURL(page, year); //URL Production
			std::cout << "Fetching Page " << page << " of year " << year << std::endl;
			std::string jsonResponse = curlRequest(url);
			if (jsonResponse.empty()) {
				std::cerr << "Failed to retrieve page " << page << std::endl;
				keepFetching = false;
				continue;
			}
			extractMovieIDs(db, jsonResponse);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
	std::cout << "\n--- Full Catalog Movie Data Collection Complete ---" << std::endl;
}

// Get's the total amount of unique actors in the database
int getTotalActors(SQLite::Database& db) {
	try {
		SQLite::Statement query(db, "SELECT COUNT(actor_id) FROM Actors;");
		if (query.executeStep()) {
			return query.getColumn(0).getInt();
		}
	}
	catch (std::exception& e) {
		std::cerr << "SQL error when counting actors: " << e.what() << std::endl;
	}
	return 1;
}