#ifndef DATACOLLECTION_H
#define DATACOLLECTION_H

// --- Libraries and Aliases ---
#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>

// Forward Declarations for External Libraries
#include <SQLiteCpp/SQLiteCpp.h> 
#include <nlohmann/json.hpp>

// Easy alias for JSON
using json = nlohmann::json;

// --- Data Collection Parralelization Logic ---

// Enum for status codes (Must be available to all files)
enum statusCodes { //Saw this, never used it before, but I kinda like it - Andrew
	NOT_STARTED = 0,
	IN_PROGRESS = 1,
	COMPLETED = 2,
	FAILED = 3
};

// Struct to hold year status (Must be available to all files)
struct yearStatus {
	int year;
	int status; // Uses statusCodes enum values
};

// --- URL Building Declarations ---
std::string buildDiscoverURL(int pageNumber, int year);
std::string buildMovieURL(int movieID);

// --- cURL Work Declarations ---
size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
std::string curlRequest(const std::string& url);

// --- Database Work Declarations ---
void setupDatabase(SQLite::Database& db);
void saveMovieData(SQLite::Database& db, int movieID, const std::string& title, const json& castArray);
void processMovie(SQLite::Database& db, int movieID);
int extractMovieIDs(SQLite::Database& db, const std::string& jsonResponse);
void runCollectionLoop(SQLite::Database& db, int year);

//=====================================================================================
//=====================================================================================
//									URL Building
//=====================================================================================
//=====================================================================================
// Basic URL building, API keys are not secured, so it can't be public, mutiple keys are
// present just as a backup in case there is a limit I couldn't find. - Andrew

SQLite::Database openYearDataBase(int year);
SQLite::Database openMainDatabase();
void mergeCollectionAndBuildGraph(SQLite::Database& mainDB, const std::vector<std::string>& filePaths);

//=====================================================================================
//=====================================================================================
//						 Data Collection Parralelization Logic Declarations
//=====================================================================================
//=====================================================================================
// Includes filehandling and console output for running off of multiple computers at once
// Super cool and simple method of having multiple machines working on the same database
// A CSV is created, which contains the codes listed in enum. At the start, every worker
// makes sure that it is in the root directory, and then does a fresh pull. It then checks
// for the first year that is NOT_STARTED and tries to claim it. It edits the CSV, then tries
// to push it, if it fails, it searches again, otherwise it get's a confirmed push, and then 
// starts building the database for that year. Once it finishes, it writes to the CSV again,
// then pushes that and the database. 
// This was a very fun section to work on, and I spent around 7 hours between researching 
// and fixing broken logic, but I think it's worth it. - Andrew

//=====================================================================================
//								Year Status Management Declarations
//=====================================================================================
yearStatus* findYear(std::vector<yearStatus>& years, int year);
std::vector<yearStatus> loadYearStatus(const std::string& filename);
int saveYearStatus(const std::string& filename, const std::vector<yearStatus>& years);

//=====================================================================================
//							GitHub File Management Declarations
//=====================================================================================
void pullLatestFromGit(const std::string& yearFile);
bool tryPushToGit(const std::string& yearFile, const std::string& commitCommand, const std::string& dbFilePath);

//=====================================================================================
//								Worker Logic Declarations
//=====================================================================================

bool changeToGitRoot();
bool workerDataCollection(int year);
void runWorker(const std::string& yearStatusFile);

//End of Declarations
#endif