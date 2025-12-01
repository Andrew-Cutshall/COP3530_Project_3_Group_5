//Libraries for data collection
#include <curl/curl.h> //For HTTP requests
#include <nlohmann/json.hpp>    //For JSON parsing
#include <SQLiteCpp/SQLiteCpp.h> //For SQLite database
//A bunch of C++ built in libraries
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <format>
#include <stdexcept> 
#include <filesystem>
#include <utility>
#include <cstdlib>
#include <thread>

//Easy alias for JSON
using json = nlohmann::json;

//=====================================================================================
//=====================================================================================
//									 URL Stuff
//=====================================================================================
//=====================================================================================
// 
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

//=====================================================================================
//=====================================================================================
//									Database Work
//=====================================================================================
//=====================================================================================

//Unlike before, database will now be handled entirely here, instead of being passed as varibles
//Opens database, returns the SQL object
SQLite::Database openMainDatabase() {
	try {
		SQLite::Database db("assets/movieData.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
		std::cout << "Database Opened\n";
		return db;
	}
	catch (const std::exception& e) {
		std::cerr << std::format("SQLite exception: {} \n", e.what());
		throw;
	}
}

//These databases are temporary, used for mass data collection from TMDB, due to my mistakes in using SQLite and collecting from TMDB - (Andrew)
//Creates a database, based on the main one, and adds data based on the particion section
//void setupSelectionDatabase(int particion)
//TODO: Add Logic


//Merges all temporary databases to the main one, and creates the graph into it as well
//The const below are needed for the next function

const char* SQL_INSERT_ACTORS = "INSERT OR IGNORE INTO Actors SELECT * FROM subDB.Actors;";
const char* SQL_INSERT_LINKS = "INSERT INTO Cast_Links SELECT * FROM subDB.Cast_Links;";
const char* SQL_CALC_EDGES = "INSERT INTO Actor_Edges (actor1_id, actor2_id, weight) SELECT T1.actor_id, T2.actor_id, COUNT(T1.movie_id) AS weight FROM Cast_Links AS T1 JOIN Cast_Links AS T2 ON T1.movie_id = T2.movie_id WHERE T1.actor_id < T2.actor_id GROUP BY T1.actor_id, T2.actor_id HAVING COUNT(T1.movie_id) >= 1;";

//Merges and builds everything. Massive SQLite transaction, with timer, since I like stats - Andrew
void mergeCollectionAndBuildGraph(SQLite::Database& mainDB, const std::vector<std::string>& filePaths) {
	mainDB.exec("BEGIN TRANSACTION;");
	auto start = std::chrono::high_resolution_clock::now();
	try {
		int databasesCount = 0;
		for (const std::string& path : filePaths) {
			databasesCount++;
			std::cout << std::format("Merging file {}/{}: {} \n", databasesCount, filePaths.size(), path);
			std::string attach = std::format("ATTACH DATABASE '{}' AS subDB;", path);
			mainDB.exec(attach);
			mainDB.exec(SQL_INSERT_ACTORS);
			mainDB.exec(SQL_INSERT_LINKS);
			mainDB.exec("DETACH DATABASE subDB;");
		}
		std::cout << "Done Merging, now starting graph calculation\n";
		mainDB.exec("DELETE FROM Actor_Edges;");
		mainDB.exec(SQL_CALC_EDGES);
		mainDB.exec("COMMIT;");
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
		std::cout << std::format("SUCCESS!! Completed in {} seconds\n", duration.count());
	}
	catch (const std::exception& e) {
		mainDB.exec("ROLLBACK;");
		std::cerr << std::format("CRITICAL PROCESS FAILURE: Rollback executed. Error: {} \n", e.what());
	}
}

//Previous database logic, cleaned up, and includes new section for saving the graph into it as well
void setupDatabase(SQLite::Database& db) {
	try {
		//List of Movies and their IDs. (Side note, this section saved most of the data from the database, as otherwise it would've needed a full rebuild - Andrew)
		db.exec("CREATE TABLE IF NOT EXISTS Movies (movie_id INTEGER PRIMARY KEY,title TEXT NOT NULL);");

		//List of All actors and their IDs, all Unique
		db.exec("CREATE TABLE IF NOT EXISTS Actors (actor_id INTEGER PRIMARY KEY,actor_name TEXT NOT NULL);");

		//Cast-Link Table, connects movie IDs to Actor IDs, and makes sure that the links valid (IDs have to exist for both the movie and the actor) and are unique (same link can't happen twice)
		db.exec("CREATE TABLE IF NOT EXISTS Cast_Links (movie_id INTEGER NOT NULL,actor_id INTEGER NOT NULL,FOREIGN KEY (movie_id) REFERENCES Movies(movie_id),FOREIGN KEY (actor_id) REFERENCES Actors(actor_id),PRIMARY KEY (movie_id, actor_id));");

		//Edges for the graph, simply how it's stored in the database, similar to the Cast-Link, except it's between actors, and has weight
		db.exec("CREATE TABLE IF NOT EXISTS Actor_Edges (actor1_id INTEGER NOT NULL,actor2_id INTEGER NOT NULL,weight INTEGER NOT NULL,FOREIGN KEY (actor1_id) REFERENCES Actors(actor_id),FOREIGN KEY (actor2_id) REFERENCES Actors(actor_id),PRIMARY KEY (actor1_id, actor2_id));");
	}
	catch (const std::exception& e) {
		std::cerr << std::format("Database error: {} \n", e.what());
		throw;
	}
}

//Now would work properly. Takes in the ID, Title, and Cast info, and saves it properly
void saveMovieData(SQLite::Database& db, int movieID, const std::string& title, const json& castArray) {
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
				stmtLink.exec(); // The Line that was forgoten, leading to a full recollection of cast list, but added for future use - Andrew
				stmtLink.reset();
			}
		}
		db.exec("COMMIT;");
	}
	catch (const std::exception& e) {
		db.exec("ROLLBACK;"); //Cancels Transaction
		std::cerr << std::format("Database Transaction Failed (Movie ID {} ): {} \n", movieID, e.what());
	}
}

//Saves movie to a Database, requires Movie ID and the Database to be specified
void processMovie(SQLite::Database& db, int movieID) {
	std::string movieURL = buildMovieURL(movieID);
	std::this_thread::sleep_for(std::chrono::milliseconds(277)); //Request will be optimized to still give headroom, 3.6 request per second at 277 milliseconds
	std::string movieResponse = curlRequest(movieURL);
	try {
		json movieData = json::parse(movieResponse);
		std::string title = movieData.value("title", "N/A");
		json castArray = movieData.value("credits", json::object()).value("cast", json::array());
		saveMovieData(db, movieID, title, castArray);
	}
	catch (json::parse_error& e) {
		std::cerr << std::format("JSON Parse Error for Movie ID ({}): {} \n", movieID, e.what());
	}
}

//Gets the Movie IDs from the json Response, then sends it to be processed, if results are present, then returns 0, else, returns 1
int extractMovieIDs(SQLite::Database& db, const std::string& jsonResponse) {
	try {
		json parsed = json::parse(jsonResponse);
		if (!parsed.contains("results") || parsed["results"].empty()) {
			return 1;
		}
		for (const auto& movie : parsed["results"]) {
			if (movie.contains("id")) {
				int movieID = movie["id"];
				if (movieID != -1) {
					processMovie(db, movieID);
				}
				else {
					std::cerr << "Invalid movie ID found.\n";
				}
			}
		}
	}
	catch (json::parse_error& e) {
		std::cerr << std::format("JSON Parse Error: {}\n", e.what());
		return 1;
	}
	return 0;
}

//Initially went from 1900 to 2025. Now setup for a Start and End year, and able to detect if there are less than 500 pages properly
void runCollectionLoop(SQLite::Database& db,int startYear,int endYear) {
	//Year Loop	
	for (int year = startYear; year <= endYear; ++year) {
		std::cout << "\n==========================================================\n";
		std::cout << "STARTING COLLECTION FOR YEAR: " << year << std::endl;
		std::cout << "==========================================================\n";
		int maxPages = 500;   //Default imit to 500 per year
		//Now for the page loop
		for (int page = 1; page <= maxPages; ++page) {
			std::string url = buildDiscoverURL(page, year); //URL Production
			std::cout << std::format("Fetching Page {} of year {}.\n", page, year);
			std::string jsonResponse = curlRequest(url);
			std::this_thread::sleep_for(std::chrono::milliseconds(277)); //Request will be optimized to still give headroom, 3.6 request per second at 277 milliseconds
			if (jsonResponse.empty()) {
				std::cerr << "Failed to retrieve page " << page << std::endl;
				break;
			}
			try {
				int stillResults = extractMovieIDs(db, jsonResponse);
				if (stillResults == 1) {
					break;
				}
			}
			catch (const json::parse_error& e) {
				std::cerr << std::format("JSON Parsing Error on page {} for year {}: {}\n", page, year, e.what());
			}
		}
	}
	std::cout << "\n--- Full Catalog Movie Data Collection Complete ---\n";
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
		std::cerr << std::format("SQL error when counting actors: {} \n", e.what());
		throw;
	}
}

//=====================================================================================
//=====================================================================================
//								 	cURL Work
//=====================================================================================
//=====================================================================================

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
			std::cerr << "\nFATAL cURL ERROR: SSL support is missing. Cannot use HTTPS. Ensure cURL is built with SCHANNEL or OpenSSL.\n";
			curl_easy_cleanup(curl);
			return "";
		}
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			std::cerr << std::format("cURL failed: {}\n", curl_easy_strerror(res));
		}
		curl_easy_cleanup(curl);
	} else {
		std::cerr << "Failed to initialize CURL.\n";
		buffer.clear(); 
	}
	return buffer;
}

//=====================================================================================
//=====================================================================================
//									URL Building
//=====================================================================================
//=====================================================================================
//NOT CLEANED YET
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
