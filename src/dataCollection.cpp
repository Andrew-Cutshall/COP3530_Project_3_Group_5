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
#include "dataCollection.h"

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
SQLite::Database openYearDataBase(int year) {
	try {
		std::string dbPath = std::format("assets/year_{}.db", year);
		SQLite::Database db(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
		setupDatabase(db);
		std::cout << std::format("Temporary Database for year {} Opened at {}\n", year, dbPath);
		return db;
	}
	catch (const std::exception& e) {
		std::cerr << std::format("SQLite exception: {} \n", e.what());
		throw;
	}
 }

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
	std::this_thread::sleep_for(std::chrono::milliseconds(320)); //Request will be optimized to still give headroom, 3.6 request per second at 277 milliseconds
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

//Initially went from 1900 to 2025. Now setup for 1 year at a time, and able to detect if there are less than 500 pages properly
void runCollectionLoop(SQLite::Database& db,int year) {
	std::cout << "\n==========================================================\n";
	std::cout << "STARTING COLLECTION FOR YEAR: " << year << std::endl;
	std::cout << "==========================================================\n";
	int maxPages = 500;   //Default imit to 500 per year
	//Now for the page loop
	for (int page = 1; page <= maxPages; ++page) {
		std::string url = buildDiscoverURL(page, year); //URL Production
		std::cout << std::format("Fetching Page {} of year {}.\n", page, year);
		std::string jsonResponse = curlRequest(url);
		std::this_thread::sleep_for(std::chrono::milliseconds(320));
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
			std::cerr << std::format("JSON Parsing Error on page {} for year {}: {}\n", page, year, e.what());			}
	}
	std::cout << std::format("Data Collection for {} complete", year);
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
std::string buildDiscoverURL(int pageNumber, int year) {
	return std::format("https://api.themoviedb.org/3/discover/movie?api_key=43220ed9cc8b8898d0671739929f87e0&include_adult=false&include_video=false&language=en-US&page={}&year={}", pageNumber, year);
}

std::string buildMovieURL(int movieID) {
	return std::format("https://api.themoviedb.org/3/movie/{}?api_key=43220ed9cc8b8898d0671739929f87e0&append_to_response=credits", std::to_string(movieID));
}

//Add Actor URL builder later here for image urls


//=====================================================================================
//=====================================================================================
//						 Data Collection Parralelization Logic
//=====================================================================================
//=====================================================================================
// Includes filehandling and console output for running off of multiple computers at once
// (If it works, it'll be cool as well, as all that needs to be done will be running the program - Andrew)

//=====================================================================================
//								Year Status Management
//=====================================================================================


//Helper for year status management
yearStatus* findYear(std::vector<yearStatus>& years, int year) {
	auto it = std::find_if(years.begin(), years.end(), [year](const yearStatus& ys) { return ys.year == year; });
	if (it != years.end()) {
		return &(*it);
	}
	return nullptr;
}

//File for managing year status
std::vector<yearStatus> loadYearStatus(const std::string& filename) {
	std::vector<yearStatus> years;
	const int startYear = 1900;
	const int endYear = 2025;
	//Make file if it doesn't exist
	if (!std::filesystem::exists(filename)) {
		for (int year = startYear; year <= endYear; ++year) {
			years.push_back({ year, NOT_STARTED });
		}
		return years;
	}
	std::ifstream file(filename);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open year status file.");
		return years;
	}
	//Load existing data
	std::string line;
	std::getline(file, line); // Skip header
	while (std::getline(file, line)) {
		int year, status;
		char comma;
		std::istringstream ss(line);
		ss >> year >> comma >> status;
		years.push_back({ year, status });
	}
	file.close();
	return years;
}

int saveYearStatus(const std::string& filename, const std::vector<yearStatus>& years) {
	std::ofstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Failed to open year status file for writing.\n";
		return 1;
	}
	file << "Year,Status\n";
	for (const auto& ys : years) {
		file << ys.year << "," << ys.status << "\n";
	}
	file.close();
	return 0;
}

//=====================================================================================
//								GitHub File Management
//=====================================================================================

const std::string GIT_BRANCH = "master";

//Pull latest
void pullLatestFromGit(const std::string& yearFile) {
	std::string command = std::format("git pull origin {} --rebase 2>&1", GIT_BRANCH);
	int result = std::system(command.c_str());
	if (result != 0) {
		std::cerr << "Git pull failed with error code: " << result << std::endl;
		std::string resetCommand = std::format("git checkout HEAD -- {}", yearFile);
		std::system(resetCommand.c_str());
		std::system("git rebase --abort 2>/dev/null || true");
		std::cout << "Git status file reset to remote version\n";
	}
	else {
		std::cout << "Successfully pulled latest status\n";
	}
}

bool tryPushToGit(const std::string& yearFile, const std::string& commitCommand) {
	std::string addCommand = std::format("git add {} 2>&1", yearFile);
	std::system(addCommand.c_str());
	int commitResult = std::system(commitCommand.c_str());
	if (commitResult != 0) {
		std::cout << "No changes to commit.\n";
		return true;
	}
	std::string pushCommand = std::format("git push origin {} 2>&1", GIT_BRANCH);
	int pushResult = std::system(pushCommand.c_str());
	if (pushResult != 0) {
		std::cerr << "Git push failed with error code: " << pushResult << std::endl;
		std::system("git reset HEAD^ 2>/dev/null || true");
		return false;
	}

	std::cout << "Successfully pushed updates to GitHub\n";
	return true;
}

bool changeToGitRoot() {
	try {
		std::filesystem::path gitRoot(GIT_ROOT_DIR);
		std::filesystem::current_path(gitRoot);

		std::cout << "Changed CWD to Git root: " << gitRoot.string() << "\n";
		return true;
	}
	catch (const std::filesystem::filesystem_error& e) {
		std::cerr << "ERROR: Failed to change directory to Git root: " << e.what() << "\n";
		return false;
	}
}

//=====================================================================================
//									Worker Logic
//=====================================================================================

void runWorker(const std::string& yearStatusFile) {
	if (!changeToGitRoot()) {
		std::cerr << "FATAL: Could not access Git directory. Aborting worker.\n";
		return;
	}
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(2)); //Delay to avoid loop spam
		pullLatestFromGit(yearStatusFile);
		std::vector<yearStatus> years = loadYearStatus(yearStatusFile);
		yearStatus* yearToProcess = nullptr;
		for (auto& ys : years) {//Find a year that is NOT_STARTED
			if (ys.status == NOT_STARTED) {
				yearToProcess = &ys;
				break;
			}
		}
		if (!yearToProcess) {
			std::cout << "All years have been processed. Exiting worker.\n";
			break;
		}
		int claimedYear = yearToProcess->year;// Try to claim year
		yearToProcess->status = IN_PROGRESS;
		if (saveYearStatus(yearStatusFile, years) != 0) { //Attempt local save
			std::cerr << "CRITICAL ERROR: Failed to save status file locally. Retrying loop.\n";
			continue;
		}
		std::string claimMessage = std::format("[CLAIM] Year {} as IN_PROGRESS", claimedYear); //Git commit message for claiming
		std::string claimCommand = std::format("git commit -m \"{}\" 2>&1", claimMessage);
		if (!tryPushToGit(yearStatusFile, claimCommand)) {
			std::cerr << "Lock acquisition failed for year " << claimedYear << ". Restarting cycle.\n";
			continue;
		}
		std::cout << "--- LOCK ACQUIRED for year " << claimedYear << " --- Starting work.\n";
		bool success = false;
		try {
			success = workerDataCollection(claimedYear);
		}
		catch (const std::exception& e) {
			std::cerr << std::format("CRITICAL ERROR during data collection for year {}: {}\n", claimedYear, e.what());
			success = false;
		}
		pullLatestFromGit(yearStatusFile);
		years = loadYearStatus(yearStatusFile);
		yearToProcess = findYear(years, claimedYear);
		if (yearToProcess) {
			yearToProcess->status = success ? COMPLETED : FAILED;
			if (saveYearStatus(yearStatusFile, years) != 0) continue;
			std::string finalStatus = (yearToProcess->status == COMPLETED) ? "COMPLETED" : "FAILED";
			std::string finalMsg = std::format("[{}] Year {} as {}", success ? "DONE" : "FAIL", claimedYear, finalStatus);
			std::string finalCommand = std::format("git commit -m \"{}\" 2>&1", finalMsg);
			tryPushToGit(yearStatusFile, finalCommand);
		}
		else {
			std::cerr << "CRITICAL ERROR: Year " << claimedYear << " disappeared from status file during processing!\n";
		}
	}
}

bool workerDataCollection(int year) {
	try {
		SQLite::Database db = openYearDataBase(year);
		setupDatabase(db);
		runCollectionLoop(db, year);
		std::cout << std::format("Year {} data successfully saved to local DB file.\n", year);
		return true;
	}
	catch (const std::exception& e) {
		std::cerr << std::format("CRITICAL EXECUTION FAILURE for Year {}: {} \n", year, e.what());
		return false;
	}
}