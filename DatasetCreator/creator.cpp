#define _CRT_SECURE_NO_WARNINGS

#include "creator.h"
#include <vector>
#include <direct.h>
#include <string.h>
#include <filesystem>
#include <string>
#include <sstream>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include <list>


#define TIME_FACTOR 12.0
#define FPS 30
#define DISPLAY_FLAG TRUE
#define WANDERING_RADIUS 10.0
#define MAX_PED_TO_CAM_DISTANCE 10000.0
#define DEMO FALSE
#define MAX_VARIATION_COMPONENTS 12


static char scenarioTypes[14][40]{
	"NEAREST",
	"RANDOM",
	"WORLD_HUMAN_MUSICIAN",
	"WORLD_HUMAN_SMOKING",
	"WORLD_HUMAN_BINOCULARS",
	"WORLD_HUMAN_CHEERING",
	"WORLD_HUMAN_DRINKING",
	"WORLD_HUMAN_PARTYING",
	"WORLD_HUMAN_PICNIC",
	"WORLD_HUMAN_STUPOR",
	"WORLD_HUMAN_PUSH_UPS",
	"WORLD_HUMAN_LEANING",
	"WORLD_HUMAN_MUSCLE_FLEX",
	"WORLD_HUMAN_YOGA"
};


std::string statusText;
DWORD statusTextDrawTicksMax;
bool statusTextGxtEntry;
std::vector<PedAppearance>::iterator pedAppsIt;
int viewCameraIndex = -1;


bool menuActive = false;

//menu commands
bool	bUp = false,
bDown = false,
bLeft = false,
bRight = false,
bEnter = false,
bBack = false,
bQuit = false;
bool stopCoords = false;

//menu parameters
float mHeight = 9, mTopFlat = 60, mTop = 36, mLeft = 3, mTitle = 5;
int activeLineIndexMain = 0;
bool visible = false;

bool subMenuActive = false;
bool trackMenuActive = false;
bool scenarioMenuActive = false;
auto startRecordingTime = std::chrono::system_clock::now();
std::vector<std::vector<TrackPosition>> pedTracks;
int currentTrack;
int currentSpawnStream;
int activeLineIndexCreateTracks;
int activeLineIndexCreateWallGroup;
int activeLineIndexCreateScenario;
int activeLineIndexCreateNetwork;
float defaultRandomRadius = 1.0f;

//peds spawned into the world should be deleted when they arrive at the target location
std::unordered_map<Ped,PedSpawned> spawnedPeds;

bool arePedsVisible = true;

int currentWallGroup = -1;
bool showWallGroups = false;
std::vector<WallGroup> wallGroups;

bool isWalkingSceneRunning = false;

int currentMaxIdentity = -1;

int currentSpawnId = 0;

void update_status_text()
{
	if (GetTickCount() < statusTextDrawTicksMax)
	{
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_SCALE(0.55f, 0.55f);
		UI::SET_TEXT_COLOUR(255, 255, 255, 255);
		UI::SET_TEXT_WRAP(0.0, 1.0);
		UI::SET_TEXT_CENTRE(1);
		UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
		UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
		if (statusTextGxtEntry)
		{
			UI::_SET_TEXT_ENTRY((char *)statusText.c_str());
		}
		else
		{
			UI::_SET_TEXT_ENTRY((char*)"STRING");
			UI::_ADD_TEXT_COMPONENT_STRING((char *)statusText.c_str());
		}
		UI::_DRAW_TEXT(0.5f, 0.1f);
	}
}

void set_status_text(std::string str, DWORD time = 2000, bool isGxtEntry = false)
{
	statusText = str;
	statusTextDrawTicksMax = GetTickCount() + time;
	statusTextGxtEntry = isGxtEntry;
}


float random_float(float min, float max) {
	return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
}

int random_int(int min, int max) {
	return min + rand() % (max - min + 1);
}

Vector3 coordsToVector(float x, float y, float z)
{
	Vector3 v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}


int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

int StringToWString(std::wstring &ws, const std::string &s)
{
	std::wstring wsTmp(s.begin(), s.end());
	ws = wsTmp;
	return 0;
}


void Creator::registerParams() {
	int_params_.registerParam(this->screen_height_param_name_);
	int_params_.registerParam(this->screen_width_param_name_);
	int_params_.registerParam(this->wait_ms_after_set_camera_param_name_);
	float_params_.registerParam(this->time_scale_param_name_);
	bool_params_.registerParam(this->is_debug_param_name_);
	int_params_.registerParam(this->fps_per_cam_param_name_);
	
}


Creator::Creator(std::string _output_path, std::string config_file) :int_params_(config_file),
																float_params_(config_file),
																string_params_(config_file),
																bool_params_(config_file) {
	
	this->registerParams();

	this->SCREEN_HEIGHT = int_params_.getParam(this->screen_height_param_name_);
	this->SCREEN_WIDTH = int_params_.getParam(this->screen_width_param_name_);
	this->waitTimeAfterSetCamera = int_params_.getParam(this->wait_ms_after_set_camera_param_name_);
	this->is_debug = bool_params_.getParam(this->is_debug_param_name_);
	this->timeScale = float_params_.getParam(this->time_scale_param_name_);
	this->fpsPerCam = int_params_.getParam(this->fps_per_cam_param_name_);
	
	
	

	//Avoid bad things that can happen to the player
	PLAYER::SET_EVERYONE_IGNORE_PLAYER(PLAYER::PLAYER_PED_ID(), TRUE);
	PLAYER::SET_POLICE_IGNORE_PLAYER(PLAYER::PLAYER_PED_ID(), TRUE);
	PLAYER::CLEAR_PLAYER_WANTED_LEVEL(PLAYER::PLAYER_PED_ID());
	ENTITY::SET_ENTITY_COLLISION(PLAYER::PLAYER_PED_ID(), TRUE, TRUE);
	ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), TRUE, FALSE);
	ENTITY::SET_ENTITY_ALPHA(PLAYER::PLAYER_PED_ID(), 255, FALSE);
	ENTITY::SET_ENTITY_CAN_BE_DAMAGED(PLAYER::PLAYER_PED_ID(), FALSE);


	//Screen capture buffer
	GRAPHICS::_GET_SCREEN_ACTIVE_RESOLUTION(&windowWidth, &windowHeight);
	hWnd = ::FindWindow(NULL, "Compatitibility Theft Auto V");
	hWindowDC = GetDC(hWnd);
	hCaptureDC = CreateCompatibleDC(hWindowDC);
	hCaptureBitmap = CreateCompatibleBitmap(hWindowDC, SCREEN_WIDTH, SCREEN_HEIGHT);
	SelectObject(hCaptureDC, hCaptureBitmap);
	SetStretchBltMode(hCaptureDC, COLORONCOLOR);

	// initialize recording stuff
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	GetEncoderClsid(L"image/jpeg", &pngClsid);


	joint_int_codes[0] = m.find("SKEL_Head")->second;
	joint_int_codes[1] = m.find("SKEL_Neck_1")->second;
	joint_int_codes[2] = m.find("SKEL_R_Clavicle")->second;
	joint_int_codes[3] = m.find("SKEL_R_UpperArm")->second;
	joint_int_codes[4] = m.find("SKEL_R_Forearm")->second;
	joint_int_codes[5] = m.find("SKEL_R_Hand")->second;
	joint_int_codes[6] = m.find("SKEL_L_Clavicle")->second;
	joint_int_codes[7] = m.find("SKEL_L_UpperArm")->second;
	joint_int_codes[8] = m.find("SKEL_L_Forearm")->second;
	joint_int_codes[9] = m.find("SKEL_L_Hand")->second;
	joint_int_codes[10] = m.find("SKEL_Spine3")->second;
	joint_int_codes[11] = m.find("SKEL_Spine2")->second;
	joint_int_codes[12] = m.find("SKEL_Spine1")->second;
	joint_int_codes[13] = m.find("SKEL_Spine0")->second;
	joint_int_codes[14] = m.find("SKEL_Spine_Root")->second;
	joint_int_codes[15] = m.find("SKEL_R_Thigh")->second;
	joint_int_codes[16] = m.find("SKEL_R_Calf")->second;
	joint_int_codes[17] = m.find("SKEL_R_Foot")->second;
	joint_int_codes[18] = m.find("SKEL_L_Thigh")->second;
	joint_int_codes[19] = m.find("SKEL_L_Calf")->second;
	joint_int_codes[20] = m.find("SKEL_L_Foot")->second;


	this->fov = 50;

	//At a value of 0 the game will still run at a minimum time scale. Less than one is slow motion.
	GAMEPLAY::SET_TIME_SCALE(this->currTimeScale);

	srand((unsigned int)time(NULL)); //Initialises randomiser or sum' like that
	this->output_path = _output_path;

	_mkdir(_output_path.c_str());

	pedAppearancesPath = this->output_path + "pedAppearances.csv";
	cameraSettingsPath = this->output_path + "cameraSettings.csv";
	walkingPositionsPath = this->output_path + "walkingPositions.csv";
	networkNodesPath = this->output_path + "networkNodes.csv";
	networkEdgesPath = this->output_path + "networkEdges.csv";
	scenarioPath = this->output_path + "scenario.csv";
	nodeTasksPath = this->output_path + "nodeTasks.csv";
	wallGroupsPath = this->output_path + "wallGroups.csv";

	log_file.open(output_path + "\\log.txt");


	//setTimeScaleViaPerCamFPS(this->fpsPerCam);
}

Creator::Creator(std::string _output_path, const char* _file_scenario, int _max_samples, int _is_night)
{
	PLAYER::SET_EVERYONE_IGNORE_PLAYER(PLAYER::PLAYER_PED_ID(), TRUE);
	PLAYER::SET_POLICE_IGNORE_PLAYER(PLAYER::PLAYER_PED_ID(), TRUE);
	PLAYER::CLEAR_PLAYER_WANTED_LEVEL(PLAYER::PLAYER_PED_ID());
	ENTITY::SET_ENTITY_COLLISION(PLAYER::PLAYER_PED_ID(), TRUE, TRUE);
	ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), TRUE, FALSE);
	ENTITY::SET_ENTITY_ALPHA(PLAYER::PLAYER_PED_ID(), 255, FALSE);
	ENTITY::SET_ENTITY_CAN_BE_DAMAGED(PLAYER::PLAYER_PED_ID(), FALSE);

    //At a value of 0 the game will still run at a minimum time scale. Less than one is slow motion.
	GAMEPLAY::SET_TIME_SCALE(1);

	this->output_path = _output_path;
	this->file_scenario = _file_scenario;
	this->max_samples = _max_samples;
	this->is_night = _is_night;

	srand((unsigned int)time(NULL)); //Initialises randomiser or sum' like that

	this->bad_scenarios = {
		"WORLD_BOAR_GRAZING",
		"WORLD_CAT_SLEEPING_GROUND",
		"WORLD_CAT_SLEEPING_LEDGE",
		"WORLD_COW_GRAZING",
		"WORLD_COYOTE_HOWL",
		"WORLD_COYOTE_REST",
		"WORLD_COYOTE_WANDER",
		"WORLD_CHICKENHAWK_FEEDING",
		"WORLD_CHICKENHAWK_STANDING",
		"WORLD_CORMORANT_STANDING",
		"WORLD_CROW_FEEDING",
		"WORLD_CROW_STANDING",
		"WORLD_DEER_GRAZING",
		"WORLD_DOG_BARKING_ROTTWEILER",
		"WORLD_DOG_BARKING_RETRIEVER",
		"WORLD_DOG_BARKING_SHEPHERD",
		"WORLD_DOG_SITTING_ROTTWEILER",
		"WORLD_DOG_SITTING_RETRIEVER",
		"WORLD_DOG_SITTING_SHEPHERD",
		"WORLD_DOG_BARKING_SMALL",
		"WORLD_DOG_SITTING_SMALL",
		"WORLD_FISH_IDLE",
		"WORLD_GULL_FEEDING",
		"WORLD_GULL_STANDING",
		"WORLD_HEN_PECKING",
		"WORLD_HEN_STANDING",
		"WORLD_MOUNTAIN_LION_REST",
		"WORLD_MOUNTAIN_LION_WANDER",
		"WORLD_PIG_GRAZING",
		"WORLD_PIGEON_FEEDING",
		"WORLD_PIGEON_STANDING",
		"WORLD_RABBIT_EATING",
		"WORLD_RATS_EATING",
		"WORLD_SHARK_SWIM",
		"PROP_BIRD_IN_TREE",
		"PROP_BIRD_TELEGRAPH_POLE"
	};

	
	// inizialize the coords_file used to storage coords data
	log_file.open(output_path + "\\log.txt");
	

	this->player = PLAYER::PLAYER_ID();
	this->playerPed = PLAYER::PLAYER_PED_ID();
	this->line = "";
	this->log = "";
	this->captureFreq = (int)(FPS / TIME_FACTOR);
	this->SHOW_JOINT_RECT = DISPLAY_FLAG;

	this->fov = 50;

	std::vector<const char*> weathers_night = {
		"CLEAR",
		"CLEAR",
		"CLEAR",
		"CLEAR",
		"CLEAR",
		"EXTRASUNNY",
		"EXTRASUNNY",
		"EXTRASUNNY",
		"RAIN",
		"THUNDER",
		"SMOG",
		"FOGGY",
		"XMAS",
		"BLIZZARD"
	};

	std::vector<const char*> weathers_day = {
		"CLEAR",
		"CLEAR",
		"CLEAR",
		"CLEAR",
		"EXTRASUNNY",
		"EXTRASUNNY",
		"EXTRASUNNY",
		"RAIN",
		"THUNDER",
		"CLOUDS",
		"OVERCAST",
		"SMOG",
		"FOGGY",
		"XMAS",
		"BLIZZARD"
	};

	//randomizing time
	int time_h, time_m, time_s;
	int date_m;
	char* weather;

	std::vector<int> night_hours = { 20, 22, 23, 4, 5, 6 };

	// set time
	if (is_night) {
		time_h = night_hours[random_int(0, 5)];
		weather = (char *)weathers_night[rand() % weathers_night.size()];
	}
	else {
		time_h = random_int(7, 19);
		weather = (char *)weathers_day[rand() % weathers_day.size()];
	}
	time_m = random_int(0, 59);
	time_s = random_int(0, 59);
	date_m = random_int(1, 12);

	TIME::SET_CLOCK_TIME(time_h, time_m, time_s);

	// randomizing weather
	GAMEPLAY::SET_RANDOM_WEATHER_TYPE();
	GAMEPLAY::CLEAR_WEATHER_TYPE_PERSIST();
	GAMEPLAY::SET_OVERRIDE_WEATHER(weather);
	GAMEPLAY::SET_WEATHER_TYPE_NOW(weather);

	loadScenario(file_scenario);

	//Screen capture buffer
	GRAPHICS::_GET_SCREEN_ACTIVE_RESOLUTION(&windowWidth, &windowHeight);
	hWnd = ::FindWindow(NULL, "Compatitibility Theft Auto V");
	hWindowDC = GetDC(hWnd);
	hCaptureDC = CreateCompatibleDC(hWindowDC);
	hCaptureBitmap = CreateCompatibleBitmap(hWindowDC, SCREEN_WIDTH, SCREEN_HEIGHT);
	SelectObject(hCaptureDC, hCaptureBitmap);
	SetStretchBltMode(hCaptureDC, COLORONCOLOR);

	// used to decide how often save the sample
	recordingPeriod = 1.0f / captureFreq;

	// initialize recording stuff
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	GetEncoderClsid(L"image/jpeg", &pngClsid);

	// inizialize the int used to count the saved frame
	nsample = 0;

	//Avoid bad things such as getting killed by the police, robbed, dying in car accidents or other horrible stuff
	PLAYER::SET_EVERYONE_IGNORE_PLAYER(player, TRUE);
	PLAYER::SET_POLICE_IGNORE_PLAYER(player, TRUE);
	PLAYER::CLEAR_PLAYER_WANTED_LEVEL(player);

	PLAYER::SET_PLAYER_INVINCIBLE(player, TRUE);
	PLAYER::SPECIAL_ABILITY_FILL_METER(player, 1);
	PLAYER::SET_PLAYER_NOISE_MULTIPLIER(player, 0.0);
	PLAYER::SET_SWIM_MULTIPLIER_FOR_PLAYER(player, 1.49f);
	PLAYER::SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER(player, 1.49f);
	PLAYER::DISABLE_PLAYER_FIRING(player, TRUE);
	PLAYER::SET_DISABLE_AMBIENT_MELEE_MOVE(player, TRUE);

	// invisible and intangible player
	/*ENTITY::SET_ENTITY_COLLISION(PLAYER::PLAYER_PED_ID(), TRUE, TRUE);
	ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), FALSE, FALSE);
	ENTITY::SET_ENTITY_ALPHA(PLAYER::PLAYER_PED_ID(), 0.0, TRUE);*/
	if (moving)
		ENTITY::SET_ENTITY_COLLISION(PLAYER::PLAYER_PED_ID(), TRUE, TRUE);
	else
		ENTITY::SET_ENTITY_COLLISION(PLAYER::PLAYER_PED_ID(), FALSE, TRUE);
	ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), FALSE, FALSE);
	ENTITY::SET_ENTITY_ALPHA(PLAYER::PLAYER_PED_ID(), 0, FALSE);
	ENTITY::SET_ENTITY_CAN_BE_DAMAGED(PLAYER::PLAYER_PED_ID(), FALSE);

	// randomizing number of peds
	//this->n_peds = random_int(20, 50);
	//this->n_peds = 20;

	// seconds are proportional to number of peds
	if (DEMO) 
		this->secondsBeforeSaveImages = 10;
	else
		this->secondsBeforeSaveImages = max_waiting_time / 1000 + 10 + 10;

	lastRecordingTime = std::clock() + (clock_t)((float)(secondsBeforeSaveImages * CLOCKS_PER_SEC));

	// spawn pedestrians
	//Scenario::spawnPed(ped_spawn_pos, n_peds);
}




Creator::~Creator()
{
	// todo: implement a destroyer
	ReleaseDC(hWnd, hWindowDC);
	DeleteDC(hCaptureDC);
	DeleteObject(hCaptureBitmap);
	closeCamCoordFiles();
	frameRateLog.close();
	pedTasksLog.close();
	log_file.close();
	deleteWallGroups();
}


std::vector<std::vector<std::string>> Creator::readStringCSV(std::string filePath)
{
	std::string line = "";
	std::ifstream file(filePath);
	std::vector<std::vector<std::string>> result;

	while (std::getline(file, line)) {
		std::stringstream lineStream(line);
		std::vector<std::string> newEntry;

		while (lineStream.good())
		{
			std::string cell = "";
			std::getline(lineStream, cell, ',');
			newEntry.push_back(cell);
		}
		result.push_back(newEntry);
	}
	file.close();
	return result;
}

std::vector<std::vector<float>> Creator::readFloatCSV(std::string filePath)
{
	std::string line = "";
	std::ifstream file(filePath);
	std::vector<std::vector<float>> result;

	while (std::getline(file, line)) {
		std::stringstream lineStream(line);
		std::vector<float> newEntry;
		
		while (lineStream.good())
		{
			std::string cell = "";
			std::getline(lineStream, cell, ',');
			newEntry.push_back(std::stof(cell));
		}
		result.push_back(newEntry);
	}
	file.close();
	return result;
}

void Creator::addCurrentCameraView() {
	loadCameraSettings();
	Vector3 oldCamCoords = CAM::GET_GAMEPLAY_CAM_COORD();
	Vector3 oldCamRots = CAM::GET_GAMEPLAY_CAM_ROT(2);
	
	CameraSetting cameraSetting(oldCamCoords, oldCamRots);
	
	set_status_text("New Camera View Added");

	cameraSettings.push_back(cameraSetting);
	saveCameraSettings();

}

void Creator::updateNew() {
	GAMEPLAY::SET_TIME_SCALE(this->currTimeScale);
	
	runWalkingScenes();
	runSpawnedPedActions();
	listenKeyStrokes();
	visualizeTrack();
	visualizeNetwork();
	visualizeTaskNodes();
	update_status_text();
	setNativePedsInvisible();
	recordAllCamsOnce();
	showFrameRate();
	drawPed2dBoxViaJoints();
	rotateAndMoveWallElements();
	
}

int Creator::update()
{
	float delay = ((float)(std::clock() - lastRecordingTime)) / CLOCKS_PER_SEC;
	if (delay >= recordingPeriod)
		lastRecordingTime = std::clock();
	else
		return nsample;
	GAMEPLAY::SET_TIME_SCALE(1.0f / (float)TIME_FACTOR);

	//1.0 = normal peds on streets
	//0.0 = no peds on streets
	//Use in looped function
	PED::SET_PED_DENSITY_MULTIPLIER_THIS_FRAME(1.0);

	Ped peds[max_number_of_peds];											// array of pedestrians
	int number_of_peds = worldGetAllPeds(peds, max_number_of_peds);			// number of pedestrians taken
	float C;																// coefficient used to adjust the size of rectangles drawn around the joints


	for (int i = 0; i < number_of_peds; i++) {
		if (!PED::IS_PED_A_PLAYER(peds[i]) && peds[i] != ped_with_cam) {
			ENTITY::SET_ENTITY_COLLISION(peds[i], TRUE, TRUE);
			ENTITY::SET_ENTITY_VISIBLE(peds[i], TRUE, FALSE);
			ENTITY::SET_ENTITY_ALPHA(peds[i], 255, FALSE);
			ENTITY::SET_ENTITY_CAN_BE_DAMAGED(peds[i], FALSE);
		}
		else if (PED::IS_PED_A_PLAYER(peds[i]))
		{
			if (moving) {
				ENTITY::SET_ENTITY_COLLISION(peds[i], TRUE, TRUE);
				ENTITY::SET_ENTITY_VISIBLE(peds[i], FALSE, FALSE);
				ENTITY::SET_ENTITY_ALPHA(peds[i], 0, FALSE);
				ENTITY::SET_ENTITY_CAN_BE_DAMAGED(peds[i], FALSE);
			}
			else {
				ENTITY::SET_ENTITY_COLLISION(peds[i], FALSE, TRUE);
				ENTITY::SET_ENTITY_VISIBLE(peds[i], FALSE, FALSE);
				ENTITY::SET_ENTITY_ALPHA(peds[i], 0, FALSE);
				ENTITY::SET_ENTITY_CAN_BE_DAMAGED(peds[i], FALSE);
			}
			
		}
		else if (peds[i] == ped_with_cam) {
			ENTITY::SET_ENTITY_COLLISION(ped_with_cam, TRUE, TRUE);
			ENTITY::SET_ENTITY_VISIBLE(ped_with_cam, FALSE, FALSE);
			ENTITY::SET_ENTITY_ALPHA(ped_with_cam, 0, FALSE);
			ENTITY::SET_ENTITY_CAN_BE_DAMAGED(ped_with_cam, FALSE);
		}
	}

	
	if (moving) {
		this->cam_coords = CAM::GET_CAM_COORD(camera);
		Vector3 ped_with_cam_rot = ENTITY::GET_ENTITY_ROTATION(this->ped_with_cam, 2);
		CAM::SET_CAM_ROT(camera, ped_with_cam_rot.x, ped_with_cam_rot.y, ped_with_cam_rot.z, 2);
		this->cam_rot = CAM::GET_CAM_ROT(camera, 2);
		//this->cam_rot = ped_with_cam_rot;
	}

	//this->cam_coords = CAM::GET_GAMEPLAY_CAM_COORD();
	//this->cam_rot = CAM::GET_GAMEPLAY_CAM_ROT(2);
	//this->fov = CAM::GET_GAMEPLAY_CAM_FOV();

	// scan all the pedestrians taken
	for (int i = 0; i < number_of_peds; i++){

		// ignore pedestrians in vehicles or dead pedestrians
		if(PED::IS_PED_IN_ANY_VEHICLE(peds[i], TRUE) || PED::IS_PED_DEAD_OR_DYING(peds[i], TRUE)) {
			//log_file << "veicolo o morto\n";
			continue;
		}
		// ignore player
		else if (PED::IS_PED_A_PLAYER(peds[i])) {
			//log_file << "player\n";
			continue;
		}
		else if (!ENTITY::IS_ENTITY_ON_SCREEN(peds[i])) {
			//log_file << "non su schermo\n";
			continue;
		}
		else if (!PED::IS_PED_HUMAN(peds[i])) {
			//log_file << "non umano\n";
			continue;
		}
		else if (!ENTITY::IS_ENTITY_VISIBLE(peds[i])) {
			//log_file << "invisibile\n";
			continue;
		}

		Vector3 ped_coords = ENTITY::GET_ENTITY_COORDS(peds[i], TRUE);
		float ped2cam_distance = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(
			cam_coords.x, cam_coords.y, cam_coords.z, 
			ped_coords.x, ped_coords.y, ped_coords.z, 1
		);

		if (ped2cam_distance < MAX_PED_TO_CAM_DISTANCE) {
			
			// for each pedestrians scan all the joint_ID we choose on the subset
			for (int n = -1; n < number_of_joints; n++) {

				Vector3 joint_coords;
				if (n == -1) {
					Vector3 head_coords = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(peds[i], PED::GET_PED_BONE_INDEX(peds[i], joint_int_codes[0]));
					Vector3 neck_coords = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(peds[i], PED::GET_PED_BONE_INDEX(peds[i], joint_int_codes[1]));
					float head_neck_norm = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(neck_coords.x, neck_coords.y, neck_coords.z, head_coords.x, head_coords.y, head_coords.z, 1);
					float dx = (head_coords.x - neck_coords.x) / head_neck_norm;
					float dy = (head_coords.y - neck_coords.y) / head_neck_norm;
					float dz = (head_coords.z - neck_coords.z) / head_neck_norm;

					joint_coords.x = head_coords.x + head_neck_norm * dx;
					joint_coords.y = head_coords.y + head_neck_norm * dy;
					joint_coords.z = head_coords.z + head_neck_norm * dz;
				}
				else 
					joint_coords = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(peds[i], PED::GET_PED_BONE_INDEX(peds[i], joint_int_codes[n]));
				
				// finding the versor (dx, dy, dz) pointing from the joint to the cam
				float joint2cam_distance = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(
					joint_coords.x, joint_coords.y, joint_coords.z, 
					cam_coords.x, cam_coords.y, cam_coords.z, 1
				);
				float dx = (cam_coords.x - joint_coords.x) / joint2cam_distance;
				float dy = (cam_coords.y - joint_coords.y) / joint2cam_distance;
				float dz = (cam_coords.z - joint_coords.z) / joint2cam_distance;

				// ray #1: from joint to cam_coords (ignoring the pedestrian to whom the joint belongs and intersecting only pedestrian (8))
				// ==> useful for detecting occlusions of pedestrian
				Vector3 end_coords1, surface_norm1;
				BOOL occlusion_ped;
				Entity entityHit1 = 0;

				int ray_ped_occlusion = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(
					joint_coords.x, joint_coords.y, joint_coords.z,
					cam_coords.x, cam_coords.y, cam_coords.z,
					8, peds[i], 7
				);
				WORLDPROBE::_GET_RAYCAST_RESULT(ray_ped_occlusion, &occlusion_ped, &end_coords1, &surface_norm1, &entityHit1);

				if (entityHit1 == ped_with_cam)
					occlusion_ped = FALSE;


				// ray #2: from joint to camera (without ignoring the pedestrian to whom the joint belongs and intersecting only pedestrian (8))
				// ==> useful for detecting self-occlusions
				Vector3 endCoords2, surfaceNormal2;
				BOOL occlusion_self;
				Entity entityHit2 = 0;
				int ray_joint2cam = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(
					joint_coords.x + 0.1f*dx, joint_coords.y + 0.1f*dy, joint_coords.z + 0.1f*dz,
					cam_coords.x, cam_coords.y, cam_coords.z, 
					8, 0, 7
				);
				WORLDPROBE::_GET_RAYCAST_RESULT(ray_joint2cam, &occlusion_self, &endCoords2, &surfaceNormal2, &entityHit2);

				if (entityHit2 == ped_with_cam)
					occlusion_self = FALSE;


				// ray #3: from camera to joint (ignoring the pedestrian to whom the joint belongs and intersecting everything but peds (4 and 8))
				// ==> useful for detecting occlusions with objects
				Vector3 endCoords3, surfaceNormal3;
				BOOL occlusion_object;
				Entity entityHit3 = 0;
				int ray_joint2cam_obj = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(
					cam_coords.x, cam_coords.y, cam_coords.z,
					joint_coords.x, joint_coords.y, joint_coords.z,
					(~0 ^ (8|4)), peds[i], 7
					);
				WORLDPROBE::_GET_RAYCAST_RESULT(ray_joint2cam_obj, &occlusion_object, &endCoords3, &surfaceNormal3, &entityHit3);

				
				BOOL occluded = occlusion_ped || occlusion_object;


				if (DISPLAY_FLAG) {
					float x, y;
					get_2D_from_3D(joint_coords, &x, &y);

					// C calculation based on distance between the current pedestrians and the camera
					if (ped2cam_distance > 6)
						C = (float)(1.5 / cbrt(ped2cam_distance));
					else
						C = 1;

					if (occluded) {
						/*GRAPHICS::DRAW_BOX(
						joint_coords.x, joint_coords.y, joint_coords.z,
						joint_coords.x + 0.1, joint_coords.y + 0.1, joint_coords.z + 0.1,
						255, 0, 64, 175
						);*/
						GRAPHICS::DRAW_RECT(x, y, 0.005f*C, 0.005f*C, 255, 0, 64, 175);
					}
					else if (occlusion_self) {
						/*GRAPHICS::DRAW_BOX(
							joint_coords.x, joint_coords.y, joint_coords.z,
							joint_coords.x + 0.1, joint_coords.y + 0.1, joint_coords.z + 0.1,
							255, 128, 16, 175
							);*/
						GRAPHICS::DRAW_RECT(x, y, 0.005f*C, 0.005f*C, 255, 128, 16, 175);
					}
					else {
						/*GRAPHICS::DRAW_BOX(
							joint_coords.x, joint_coords.y, joint_coords.z,
							joint_coords.x + 0.1, joint_coords.y + 0.1, joint_coords.z + 0.1,
							0, 255, 64, 175
							);*/
						GRAPHICS::DRAW_RECT(x, y, 0.005f*C, 0.005f*C, 0, 255, 64, 175);
					}
				}
				float x, y;
				get_2D_from_3D(joint_coords, &x, &y);
				x = x * SCREEN_WIDTH;
				y = y * SCREEN_HEIGHT;

			}
		}
	}
	save_frame();
	nsample++;
	if (nsample == max_samples) {
		for (int i = 0; i < nwPeds; i++) {
			PED::DELETE_PED(&wPeds[i].ped);
		}
		for (int i = 0; i < nwPeds_scenario; i++) {
			PED::DELETE_PED(&wPeds_scenario[i].ped);
		}
	}

	return nsample;
}


void Creator::appendLineToFile(std::string path, std::string line)
{
	
	std::ofstream file(path, std::ios_base::app | std::ios_base::out);

	file << line << "\n";
	file.close();

	path = path + "  SAVED!";
	set_status_text(path.c_str());
}

std::string floatToString(float val) {
	std::ostringstream ss;
	ss << val;

	return ss.str();
}

void Creator::setCamera(Vector3 coords, Vector3 rots) {
	
	CAM::DESTROY_ALL_CAMS(TRUE);
	this->camera = CAM::CREATE_CAM((char *)"DEFAULT_SCRIPTED_CAMERA", TRUE);
	CAM::SET_CAM_COORD(this->camera, coords.x, coords.y, coords.z);
	CAM::SET_CAM_ROT(this->camera, rots.x, rots.y, rots.z, 2);
	CAM::SET_CAM_ACTIVE(this->camera, TRUE);
	CAM::SET_CAM_FOV(this->camera, (float)this->fov);
	CAM::RENDER_SCRIPT_CAMS(TRUE, FALSE, 0, TRUE, TRUE);

	this->cam_coords = CAM::GET_CAM_COORD(this->camera);
	this->cam_rot = CAM::GET_CAM_ROT(this->camera, 2);
	this->fov = (int)CAM::GET_CAM_FOV(this->camera);
	showingGameplayCam = false;
}


void Creator::appendCSVLinesToFile(std::shared_ptr<std::ofstream> file, std::vector<std::vector<std::string>> stringVector) {
	

	for (std::vector<std::string> vectorCSVLine : stringVector) {
		std::string line = "";
		for (int i = 0; i < vectorCSVLine.size(); i++) {
			line += vectorCSVLine[i];
			if (i < vectorCSVLine.size() - 1) {
				line += ",";
			}
			else {
				line += "\n";
			}

		}
		
		*file << line;
	}

	
	
}



void Creator::loadPedAppearanceSet() {


	if (!pedAppearanceSet.empty()) {
		return;
	}

		std::vector<std::vector<std::string>> csvPedAppearances = readStringCSV(pedAppearancesPath);

		std::vector<std::vector<std::string>> onePedAppearance;
		std::string identityStr = "";
		

		

		for (std::vector<std::string> row : csvPedAppearances) {
			if (row[0] != identityStr) {
				identityStr = row[0];

				int newMax = std::stoi(identityStr);
				if (newMax > currentMaxIdentity) {
					currentMaxIdentity = newMax;
				}
				

				//create new PedAppearance
				if (onePedAppearance.size() > 0) {

					PedAppearance pedAppearance(onePedAppearance);
					pedAppearanceSet.insert(pedAppearance);
					onePedAppearance.clear();
				}
			}
			onePedAppearance.push_back(row);
		}
		//To insert the last entry. Because there is no next row to trigger the row[0] != identityStr
		if (onePedAppearance.size() == MAX_VARIATION_COMPONENTS) {

			PedAppearance pedAppearance(onePedAppearance);
			pedAppearanceSet.insert(pedAppearance);
			onePedAppearance.clear();
		}
		
		
		pedAppearanceVector.clear();
		std::copy(pedAppearanceSet.begin(), pedAppearanceSet.end(), std::back_inserter(pedAppearanceVector));
		
		auto deleteCondition = [](const PedAppearance& pedAppearance) {
			return !pedAppearance.isSpawnable; // put your condition here
		};

		pedAppearanceVector.erase(std::remove_if(
			pedAppearanceVector.begin(), pedAppearanceVector.end(),
			deleteCondition), pedAppearanceVector.end());

		//Will then be used to spawn random people from the list
		std::random_shuffle(pedAppearanceVector.begin(), pedAppearanceVector.end());


		pedAppsIt = pedAppearanceVector.begin();

		if (this->is_debug) {
			set_status_text("ped app. loaded. Spawnable Size: " + std::to_string(pedAppearanceVector.size()) + " of " + std::to_string(pedAppearanceSet.size()));

		}
		


}




void Creator::savePedAppearanceSet() {
	std::ofstream pedAppearanceFile(pedAppearancesPath);
	

	const auto cmp = [](const PedAppearance& lhs, const PedAppearance& rhs) {
		return lhs.identityNo < rhs.identityNo;
	};

	std::vector<PedAppearance> pedAppearancesToSort;
	
	std::copy(pedAppearanceSet.begin(), pedAppearanceSet.end(), std::back_inserter(pedAppearancesToSort));
	std::sort(pedAppearancesToSort.begin(), pedAppearancesToSort.end(), cmp);


	for (int i = 0; i < pedAppearancesToSort.size(); i++) {
		pedAppearancesToSort[i].appendToOfstream(pedAppearanceFile);
	}

	
	
	pedAppearanceFile.close();
}



void Creator::logPedAppearance() {

	loadPedAppearanceSet();
	

	const int maxWorldPedCount = 5000;
	Ped worldPeds[maxWorldPedCount];
	int worldPedCount = worldGetAllPeds(worldPeds, maxWorldPedCount);

	
	int addedPedCount = 0;
	for (int i = 0; i < worldPedCount; i++) {

		//check if it is not contained and insert if not.
		PedAppearance newPedAppearance(worldPeds[i],-1);

		if ((pedAppearanceSet.find(newPedAppearance) == pedAppearanceSet.end()) && PED::IS_PED_HUMAN(worldPeds[i])) {
			
			addedPedCount++;
			currentMaxIdentity++;
			newPedAppearance.identityNo = currentMaxIdentity;
			pedAppearanceSet.insert(newPedAppearance);
			

		}
		
	}

	
	savePedAppearanceSet();


	set_status_text("Added " + std::to_string(addedPedCount) + " of " + std::to_string(worldPedCount) + " all: " + std::to_string(pedAppearanceSet.size()));

}

void Creator::logTasksPed(Ped ped) {
	
	
	pedTasksLog << std::to_string(ped) << "," << std::to_string(GAMEPLAY::GET_FRAME_COUNT());
	const unsigned int maxTaskNumber = 528;
	for (int taskNo = 0; taskNo < maxTaskNumber; taskNo++) {
		if (AI::GET_IS_TASK_ACTIVE(ped, taskNo)) {
			pedTasksLog << "," + std::to_string(taskNo);
		}
	}

	pedTasksLog << std::endl;
	
}




void Creator::logPedestrians(int imageCountPerCam, int frameCount, int camId, std::shared_ptr<std::ofstream> coordsFile) {
	
	

	Ped peds[max_number_of_peds];											// array of pedestrians
	int number_of_peds = worldGetAllPeds(peds, max_number_of_peds);			// number of pedestrians taken



	//CAM::GET_CAM_ROT
	this->cam_coords = CAM::GET_CAM_COORD(this->camera);
	this->cam_rot = CAM::GET_CAM_ROT(this->camera,2);
	
	


	// scan all the pedestrians taken
	for (int i = 0; i < number_of_peds; i++) {


		// ignore pedestrians in vehicles or dead pedestrians
		if (PED::IS_PED_IN_ANY_VEHICLE(peds[i], TRUE) || PED::IS_PED_DEAD_OR_DYING(peds[i], TRUE)) {
			//log_file << "veicolo o morto\n";
			continue;
		}
		// ignore player
		if (PED::IS_PED_A_PLAYER(peds[i])) {
			//log_file << "player\n";
			continue;
		}
		if (!ENTITY::IS_ENTITY_ON_SCREEN(peds[i])) {
			//log_file << "non su schermo\n";

			//We want to get all pedestrians
			continue;
		}
		if (!PED::IS_PED_HUMAN(peds[i])) {
			//log_file << "non umano\n";
			continue;
		}
		

		if (!ENTITY::IS_ENTITY_VISIBLE(peds[i])) {
			//log_file << "invisibile\n";
			
			//We want to get also invisible pedestrians
			continue;
		}
		auto foundSpawnedPed = spawnedPeds.find(peds[i]);
		if (foundSpawnedPed == spawnedPeds.end()) {
			//If it is not spawned by us us then it should not be logged
			continue;
		}
		
		

		int pedAppearanceId = -1;
		PedAppearance currentPedAppearance(peds[i],-1);
		//get the ped appearance id from the ped appearance set 
		auto foundPedAppearance = pedAppearanceSet.find(currentPedAppearance);
		if (foundPedAppearance != pedAppearanceSet.end()) {
			pedAppearanceId = (*foundPedAppearance).identityNo;
		}
		else {
			std::ofstream changedAppearancePeds(output_path + "changedAppearancePeds.csv",std::ofstream::app);

			changedAppearancePeds << "cam_id: " << std::to_string(camId) << "\n";
			changedAppearancePeds << "GTA_FRAME_NO: " << std::to_string(GAMEPLAY::GET_FRAME_COUNT()) << "\n";
			changedAppearancePeds << "ped_id: " << std::to_string(peds[i]) << "\n";
			changedAppearancePeds << "current_appearance_id: " << std::to_string(currentPedAppearance.identityNo) << "\n";
			currentPedAppearance.appendToOfstream(changedAppearancePeds);
			changedAppearancePeds << "\n";
			foundSpawnedPed->second.pedAppearance.appendToOfstream(changedAppearancePeds);
			

			currentMaxIdentity++;
			currentPedAppearance.identityNo = currentMaxIdentity;
			pedAppearanceId = currentMaxIdentity;

			changedAppearancePeds << "new_appearance_id: " << std::to_string(pedAppearanceId) << "\n";

			currentPedAppearance.isSpawnable = false;
			pedAppearanceSet.insert(currentPedAppearance);
			savePedAppearanceSet();


			changedAppearancePeds.close();

		}


		Vector3 ped_coords = ENTITY::GET_ENTITY_COORDS(peds[i], TRUE);
		float ped2cam_distance = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(
			cam_coords.x, cam_coords.y, cam_coords.z,
			ped_coords.x, ped_coords.y, ped_coords.z, 1
		);

		if (ped2cam_distance < MAX_PED_TO_CAM_DISTANCE) {


			int ped_type = PED::GET_PED_TYPE(peds[i]);

			int wears_glasses = 0;
			if (PED::GET_PED_PROP_INDEX(peds[i], 1) > -1) {
				wears_glasses = 1;
			}
			
			BoundingBox nonOccludedBox;
			nonOccludedBox.x_max = 0;
			nonOccludedBox.x_min = (float)SCREEN_WIDTH;
			nonOccludedBox.y_max = 0;
			nonOccludedBox.y_min = (float)SCREEN_HEIGHT;

			BoundingBox occludedBox;
			occludedBox.x_max = 0;
			occludedBox.x_min = (float)SCREEN_WIDTH;
			occludedBox.y_max = 0;
			occludedBox.y_min = (float)SCREEN_HEIGHT;

			std::vector<std::vector<std::string>> jointPedLog;

			// for each pedestrians scan all the joint_ID we choose on the subset
			int notOccludedJointsCount = 0;
			for (int n = -1; n < number_of_joints; n++) {

				Vector3 joint_coords;
				if (n == -1) {
					Vector3 head_coords = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(peds[i], PED::GET_PED_BONE_INDEX(peds[i], joint_int_codes[0]));
					Vector3 neck_coords = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(peds[i], PED::GET_PED_BONE_INDEX(peds[i], joint_int_codes[1]));
					float head_neck_norm = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(neck_coords.x, neck_coords.y, neck_coords.z, head_coords.x, head_coords.y, head_coords.z, 1);
					float dx = (head_coords.x - neck_coords.x) / head_neck_norm;
					float dy = (head_coords.y - neck_coords.y) / head_neck_norm;
					float dz = (head_coords.z - neck_coords.z) / head_neck_norm;

					joint_coords.x = head_coords.x + head_neck_norm * dx;
					joint_coords.y = head_coords.y + head_neck_norm * dy;
					joint_coords.z = head_coords.z + head_neck_norm * dz;
				}
				else
					joint_coords = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(peds[i], PED::GET_PED_BONE_INDEX(peds[i], joint_int_codes[n]));

				// finding the versor (dx, dy, dz) pointing from the joint to the cam
				float joint2cam_distance = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(
					joint_coords.x, joint_coords.y, joint_coords.z,
					cam_coords.x, cam_coords.y, cam_coords.z, 1
				);
				float dx = (cam_coords.x - joint_coords.x) / joint2cam_distance;
				float dy = (cam_coords.y - joint_coords.y) / joint2cam_distance;
				float dz = (cam_coords.z - joint_coords.z) / joint2cam_distance;

				// ray #1: from joint to cam_coords (ignoring the pedestrian to whom the joint belongs and intersecting only pedestrian (8))
				// ==> useful for detecting occlusions of pedestrian
				Vector3 end_coords1, surface_norm1;
				BOOL occlusion_ped;
				Entity entityHit1 = 0;

				int ray_ped_occlusion = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(
					joint_coords.x, joint_coords.y, joint_coords.z,
					cam_coords.x, cam_coords.y, cam_coords.z,
					8, peds[i], 7
				);
				WORLDPROBE::_GET_RAYCAST_RESULT(ray_ped_occlusion, &occlusion_ped, &end_coords1, &surface_norm1, &entityHit1);

				if (entityHit1 == ped_with_cam)
					occlusion_ped = FALSE;


				// ray #2: from joint to camera (without ignoring the pedestrian to whom the joint belongs and intersecting only pedestrian (8))
				// ==> useful for detecting self-occlusions
				Vector3 endCoords2, surfaceNormal2;
				BOOL occlusion_self;
				Entity entityHit2 = 0;
				int ray_joint2cam = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(
					joint_coords.x + 0.1f*dx, joint_coords.y + 0.1f*dy, joint_coords.z + 0.1f*dz,
					cam_coords.x, cam_coords.y, cam_coords.z,
					8, 0, 7
				);
				WORLDPROBE::_GET_RAYCAST_RESULT(ray_joint2cam, &occlusion_self, &endCoords2, &surfaceNormal2, &entityHit2);

				if (entityHit2 == ped_with_cam)
					occlusion_self = FALSE;


				// ray #3: from camera to joint (ignoring the pedestrian to whom the joint belongs and intersecting everything but peds (4 and 8))
				// ==> useful for detecting occlusions with objects
				Vector3 endCoords3, surfaceNormal3;
				BOOL occlusion_object;
				Entity entityHit3 = 0;
				int ray_joint2cam_obj = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(
					cam_coords.x, cam_coords.y, cam_coords.z,
					joint_coords.x, joint_coords.y, joint_coords.z,
					(~0 ^ (8 | 4)), peds[i], 7
				);
				WORLDPROBE::_GET_RAYCAST_RESULT(ray_joint2cam_obj, &occlusion_object, &endCoords3, &surfaceNormal3, &entityHit3);


			

				BOOL occluded = occlusion_ped || occlusion_object;


				float x_joint, y_joint;
				get_2D_from_3D(joint_coords, &x_joint, &y_joint);
				x_joint = x_joint * (float)SCREEN_WIDTH;
				y_joint = y_joint * (float)SCREEN_HEIGHT;

				if (!(occluded || occlusion_self)) {
					notOccludedJointsCount++;

					nonOccludedBox.x_min = std::min<float>(x_joint, nonOccludedBox.x_min);
					nonOccludedBox.x_max = std::max<float>(x_joint, nonOccludedBox.x_max);
					nonOccludedBox.y_min = std::min<float>(y_joint, nonOccludedBox.y_min);
					nonOccludedBox.y_max = std::max<float>(y_joint, nonOccludedBox.y_max);


				}

				occludedBox.x_min = std::min<float>(x_joint, occludedBox.x_min);
				occludedBox.x_max = std::max<float>(x_joint, occludedBox.x_max);
				occludedBox.y_min = std::min<float>(y_joint, occludedBox.y_min);
				occludedBox.y_max = std::max<float>(y_joint, occludedBox.y_max);

				float x_ped, y_ped;
				get_2D_from_3D(ped_coords, &x_ped, &y_ped);
				x_ped = x_ped * (float)SCREEN_WIDTH;
				y_ped = y_ped * (float)SCREEN_HEIGHT;

				std::vector<std::string> jointLogEntry;
				jointLogEntry.push_back(std::to_string(frameCount));					  // frame number
				jointLogEntry.push_back(std::to_string(imageCountPerCam));
				jointLogEntry.push_back(std::to_string(peds[i])); // pedestrian ID
				jointLogEntry.push_back(std::to_string(foundSpawnedPed->second.spawnId)); //spawnId to be sure to have a unique id
				jointLogEntry.push_back(std::to_string(pedAppearanceId)); // appearance ID from the pedAppearance.csv 
				jointLogEntry.push_back(std::to_string(n + 1));   // joint type
				jointLogEntry.push_back(std::to_string(x_joint)); // camera 2D joint x [px]			  
				jointLogEntry.push_back(std::to_string(y_joint)); // camera 2D joint y [px]
				jointLogEntry.push_back(std::to_string(joint_coords.x)); // joint 3D x [m]
				jointLogEntry.push_back(std::to_string(joint_coords.y)); // joint 3D y [m]
				jointLogEntry.push_back(std::to_string(joint_coords.z)); // joint 3D z [m]
				jointLogEntry.push_back(std::to_string(occluded)); // is joint occluded?
				jointLogEntry.push_back(std::to_string(occlusion_self)); // is joint self-occluded?
				jointLogEntry.push_back(std::to_string(cam_coords.x)); // camera 3D x [m]
				jointLogEntry.push_back(std::to_string(cam_coords.y)); // camera 3D y [m]
				jointLogEntry.push_back(std::to_string(cam_coords.z)); // camera 3D z [m]
				jointLogEntry.push_back(std::to_string(cam_rot.x)); // camera 3D rot x [m]
				jointLogEntry.push_back(std::to_string(cam_rot.y)); // camera 3D rot y [m]
				jointLogEntry.push_back(std::to_string(cam_rot.z)); // camera 3D rot z [m]
				jointLogEntry.push_back(std::to_string(fov));
				jointLogEntry.push_back(std::to_string(ped_coords.x));
				jointLogEntry.push_back(std::to_string(ped_coords.y));
				jointLogEntry.push_back(std::to_string(ped_coords.z));
				jointLogEntry.push_back(std::to_string(x_ped));
				jointLogEntry.push_back(std::to_string(y_ped));
				jointLogEntry.push_back(std::to_string(ped_type));
				jointLogEntry.push_back(std::to_string(wears_glasses));
				jointLogEntry.push_back(std::to_string(ENTITY::GET_ENTITY_HEADING(peds[i])));
				jointLogEntry.push_back(std::to_string(TIME::GET_CLOCK_HOURS()));
				jointLogEntry.push_back(std::to_string(TIME::GET_CLOCK_MINUTES()));
				jointLogEntry.push_back(std::to_string(TIME::GET_CLOCK_SECONDS()));
		

				jointPedLog.push_back(jointLogEntry);
			}

			if (notOccludedJointsCount >= 1) {
				BoundingBox paddedBox = getPaddedBoundingBox(occludedBox, nonOccludedBox);

				for (int i = 0; i < jointPedLog.size(); i++) {
					jointPedLog[i].push_back(std::to_string(paddedBox.x_min));
					jointPedLog[i].push_back(std::to_string(paddedBox.y_min));
					jointPedLog[i].push_back(std::to_string(paddedBox.x_max));
					jointPedLog[i].push_back(std::to_string(paddedBox.y_max));
				}

				logTasksPed(peds[i]);
				appendCSVLinesToFile(coordsFile, jointPedLog);
			}
		


		}
	}
}

void Creator::recordCurrentCamera() {
	for (int i = 0; i < 1000; i++) {
		updateNew();
		std::string camFolderPath = this->output_path + "cam_current\\";
		_mkdir(camFolderPath.c_str());
		saveScreenImage(camFolderPath + "image_" + std::to_string(i) + ".jpg");
		WAIT(10); //without this line the old camera image will be recorded
		Sleep(0); //without this line the old camera image will be recorded
	}
}

void Creator::drawPedBox3D() {
	const int maxWorldPeds = 1500;
	int allPeds[maxWorldPeds];
	int foundWorldPeds = worldGetAllPeds(allPeds, maxWorldPeds);

	for(int i = 0; i < foundWorldPeds; i++) {
		
		Vector3 pedPos = ENTITY::GET_ENTITY_COORDS(allPeds[i], true);
		Vector3 pedRot = ENTITY::GET_ENTITY_ROTATION(allPeds[i], 2);


		Vector3 minDim;
		Vector3 maxDim;
		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(allPeds[i]), &minDim, &maxDim);

		Cuboid ped3dBox(pedPos, pedRot, maxDim.y - minDim.y , maxDim.z - minDim.z , maxDim.x - minDim.x);
		ped3dBox.draw();
	}

}

std::vector<JointPosition> Creator::getPedJointPoints(Ped ped) {
	
	std::vector<JointPosition> resultJoints;
	for (int n = -1; n < number_of_joints; n++) {
		JointPosition jointPosition;
		Vector3 joint_coords;
		if (n == -1) {
			Vector3 head_coords = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(ped, PED::GET_PED_BONE_INDEX(ped, joint_int_codes[0]));
			Vector3 neck_coords = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(ped, PED::GET_PED_BONE_INDEX(ped, joint_int_codes[1]));
			float head_neck_norm = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(neck_coords.x, neck_coords.y, neck_coords.z, head_coords.x, head_coords.y, head_coords.z, 1);
			float dx = (head_coords.x - neck_coords.x) / head_neck_norm;
			float dy = (head_coords.y - neck_coords.y) / head_neck_norm;
			float dz = (head_coords.z - neck_coords.z) / head_neck_norm;

			joint_coords.x = head_coords.x + head_neck_norm * dx;
			joint_coords.y = head_coords.y + head_neck_norm * dy;
			joint_coords.z = head_coords.z + head_neck_norm * dz;
		}
		else
			joint_coords = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(ped, PED::GET_PED_BONE_INDEX(ped, joint_int_codes[n]));

		// finding the versor (dx, dy, dz) pointing from the joint to the cam
		float joint2cam_distance = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(
			joint_coords.x, joint_coords.y, joint_coords.z,
			cam_coords.x, cam_coords.y, cam_coords.z, 1
		);
		float dx = (cam_coords.x - joint_coords.x) / joint2cam_distance;
		float dy = (cam_coords.y - joint_coords.y) / joint2cam_distance;
		float dz = (cam_coords.z - joint_coords.z) / joint2cam_distance;

		// ray #1: from joint to cam_coords (ignoring the pedestrian to whom the joint belongs and intersecting only pedestrian (8))
		// ==> useful for detecting occlusions of pedestrian
		Vector3 end_coords1, surface_norm1;
		BOOL occlusion_ped;
		Entity entityHit1 = 0;

		int ray_ped_occlusion = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(
			joint_coords.x, joint_coords.y, joint_coords.z,
			cam_coords.x, cam_coords.y, cam_coords.z,
			8, ped, 7
		);
		WORLDPROBE::_GET_RAYCAST_RESULT(ray_ped_occlusion, &occlusion_ped, &end_coords1, &surface_norm1, &entityHit1);

		if (entityHit1 == ped_with_cam)
			occlusion_ped = FALSE;

		jointPosition.occluded_ped = occlusion_ped;

		// ray #2: from joint to camera (without ignoring the pedestrian to whom the joint belongs and intersecting only pedestrian (8))
		// ==> useful for detecting self-occlusions
		Vector3 endCoords2, surfaceNormal2;
		BOOL occlusion_self;
		Entity entityHit2 = 0;
		int ray_joint2cam = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(
			joint_coords.x + 0.1f*dx, joint_coords.y + 0.1f*dy, joint_coords.z + 0.1f*dz,
			cam_coords.x, cam_coords.y, cam_coords.z,
			8, 0, 7
		);
		WORLDPROBE::_GET_RAYCAST_RESULT(ray_joint2cam, &occlusion_self, &endCoords2, &surfaceNormal2, &entityHit2);

		if (entityHit2 == ped_with_cam)
			occlusion_self = FALSE;

		jointPosition.occluded_self = occlusion_self;

		// ray #3: from camera to joint (ignoring the pedestrian to whom the joint belongs and intersecting everything but peds (4 and 8))
		// ==> useful for detecting occlusions with objects
		Vector3 endCoords3, surfaceNormal3;
		BOOL occlusion_object;
		Entity entityHit3 = 0;
		int ray_joint2cam_obj = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(
			cam_coords.x, cam_coords.y, cam_coords.z,
			joint_coords.x, joint_coords.y, joint_coords.z,
			(~0 ^ (8 | 4)), ped, 7
		);
		
		WORLDPROBE::_GET_RAYCAST_RESULT(ray_joint2cam_obj, &occlusion_object, &endCoords3, &surfaceNormal3, &entityHit3);

		
		
		jointPosition.occluded_object = occlusion_object;
	

		
		
		jointPosition.position = joint_coords;
		resultJoints.push_back(jointPosition);
		
	}

	return resultJoints;
}

void Creator::drawPed2dBoxViaJoints() {

	if (!shouldDrawPed2dBox) {
		return;
	}

	const int maxWorldPeds = 1500;
	int allPeds[maxWorldPeds];
	int foundWorldPeds = worldGetAllPeds(allPeds, maxWorldPeds);

	for (int i = 0; i < foundWorldPeds; i++) {
		int numJointsFound = 0;
		if (PED::IS_PED_IN_ANY_VEHICLE(allPeds[i], TRUE) || PED::IS_PED_DEAD_OR_DYING(allPeds[i], TRUE)) {
			//log_file << "veicolo o morto\n";
			continue;
		}
		// ignore player
		if (PED::IS_PED_A_PLAYER(allPeds[i])) {
			//log_file << "player\n";
			continue;
		}
		if (!ENTITY::IS_ENTITY_ON_SCREEN(allPeds[i])) {
			//log_file << "non su schermo\n";

			//We want to get all pedestrians
			continue;
		}
		if (!PED::IS_PED_HUMAN(allPeds[i])) {
			//log_file << "non umano\n";
			continue;
		}


		if (!ENTITY::IS_ENTITY_VISIBLE(allPeds[i])) {
			//log_file << "invisibile\n";

			//We want to get also invisible pedestrians
			continue;
		}

		std::vector<JointPosition> jointPositions = getPedJointPoints(allPeds[i]);

		BoundingBox nonOccludedBox;
		nonOccludedBox.x_max = 0;
		nonOccludedBox.x_min = (float)SCREEN_WIDTH;
		nonOccludedBox.y_max = 0;
		nonOccludedBox.y_min = (float)SCREEN_HEIGHT;

		BoundingBox occludedBox;
		occludedBox.x_max = 0;
		occludedBox.x_min = (float)SCREEN_WIDTH;
		occludedBox.y_max = 0;
		occludedBox.y_min = (float)SCREEN_HEIGHT;

		for (JointPosition jointPos : jointPositions) {
			
			
			
			if (showingGameplayCam) {
				this->cam_coords = CAM::GET_GAMEPLAY_CAM_COORD();
				this->cam_rot = CAM::GET_GAMEPLAY_CAM_ROT(2);
			}

			float x_joint, y_joint;
			get_2D_from_3D(jointPos.position, &x_joint, &y_joint);
			x_joint = x_joint * (float)SCREEN_WIDTH;
			y_joint = y_joint * (float)SCREEN_HEIGHT;



			if (!(jointPos.occluded_object || jointPos.occluded_ped)) {
				numJointsFound++;

				nonOccludedBox.x_min = std::min<float>(x_joint, nonOccludedBox.x_min);
				nonOccludedBox.x_max = std::max<float>(x_joint, nonOccludedBox.x_max);
				nonOccludedBox.y_min = std::min<float>(y_joint, nonOccludedBox.y_min);
				nonOccludedBox.y_max = std::max<float>(y_joint, nonOccludedBox.y_max);


			}
			
			occludedBox.x_min = std::min<float>(x_joint, occludedBox.x_min);
			occludedBox.x_max = std::max<float>(x_joint, occludedBox.x_max);
			occludedBox.y_min = std::min<float>(y_joint, occludedBox.y_min);
			occludedBox.y_max = std::max<float>(y_joint, occludedBox.y_max);
		

		}
		if (numJointsFound >= 1) {
			BoundingBox paddedBox = getPaddedBoundingBox(occludedBox, nonOccludedBox);

			Helper::drawBox2D((int)paddedBox.x_min
				, (int)paddedBox.y_min
				,(int)paddedBox.x_max
				,(int)paddedBox.y_max, 2.0f);
		}
		
		
	}
}

BoundingBox Creator::getPaddedBoundingBox(BoundingBox occludedBoxMaxMin, BoundingBox nonOccludedBoxMaxMin) {
	BoundingBox resultBox;

	float ocl_box_height = (occludedBoxMaxMin.y_max - occludedBoxMaxMin.y_min);
	float ocl_box_width = (occludedBoxMaxMin.x_max - occludedBoxMaxMin.x_min);

	float bottomBoxPadding = ocl_box_height * 0.18f;
	float topBoxPadding = ocl_box_height * 0.12f;
	float leftBoxPadding = ocl_box_width * 0.29f;
	float rightBoxPadding = ocl_box_width * 0.29f;


	resultBox.x_min = nonOccludedBoxMaxMin.x_min - leftBoxPadding;
	resultBox.x_max = nonOccludedBoxMaxMin.x_max + rightBoxPadding;
	resultBox.y_min = nonOccludedBoxMaxMin.y_min - topBoxPadding;
	resultBox.y_max = nonOccludedBoxMaxMin.y_max + bottomBoxPadding;



	return resultBox;
}

void Creator::saveNodeTasks() {
	
	
	std::ofstream nodeTasksFile(nodeTasksPath);
	for (auto nodeTask : nodeTasks) {
		nodeTasksFile << nodeTask.to_csv() << "\n";
	}
	nodeTasksFile.close();
}


void Creator::loadNodeTasks() {

	if (!nodeTasks.empty()) {
		return;
	}

	std::vector<std::vector<std::string>> csvStrings = readStringCSV(nodeTasksPath);


	for (auto csvNodeTask : csvStrings) {

		nodeTasks.insert(NodeTask(std::stoi(csvNodeTask[0]), std::stoi(csvNodeTask[1]), std::stof(csvNodeTask[2])));

	}

}

void Creator::loadCameraSettings() {
	if (!this->cameraSettings.empty()) {
		return;
	}

	auto cameraSettingsStrings = readStringCSV(cameraSettingsPath);
	
	for (std::vector<std::string> cameraSettingString : cameraSettingsStrings) {
		cameraSettings.push_back(CameraSetting(cameraSettingString));
	}

}

void Creator::saveCameraSettings() {

	std::ofstream cameraSettingsFile(cameraSettingsPath);
	for (CameraSetting cameraSetting : cameraSettings) {
		cameraSettingsFile << cameraSetting.getCSV() << "\n";
	}
	cameraSettingsFile.close();
}

void Creator::resetPlayerCam() {
	
	CAM::RENDER_SCRIPT_CAMS(0, 0, 3000, 1, 0);
	showingGameplayCam = true;
}

//Can't be done in a loop because it blocks actions of peds that will be set
void Creator::recordAllCamsOnce() {
	

	if (!recordInLoop) {
		
		return;
	}
	
	loadCameraSettings();

	for (int camId = 0; camId < cameraSettings.size(); camId++) {


		CameraSetting cameraSetting = cameraSettings[camId];


		setCamera(cameraSetting.position, cameraSetting.rotation);
		WAIT(0); //without this line the old camera image will be recorded
		Sleep(waitTimeAfterSetCamera); //without this line the old camera image will be recorded
		setNativePedsInvisible();
		int frameCount = GAMEPLAY::GET_FRAME_COUNT();
		logPedestrians(imageCountPerCam, frameCount, camId, camCoordsFiles[camId]);

		logFramerate();

		std::string camFolder = "cam_" + std::to_string(camId) + "\\";
		std::string camFolderPath = this->output_path + camFolder;
		std::string imageName = "image_" + std::to_string(imageCountPerCam) + "_" + std::to_string(camId);
		_mkdir(camFolderPath.c_str());

		/**
		std::string camFolderStencil = "stencil_cam_" + std::to_string(camId) + "\\";
		std::string camFolderPathStencil = this->output_path + camFolderStencil;
		_mkdir(camFolderPathStencil.c_str());
		std::string pathStencil = camFolderPathStencil + imageName + ".png";
		**/

		std::string pathImage = camFolderPath + imageName + ".jpg";



		//Problem: People don't accept new commands very fast if game is paused ... 
		//GAMEPLAY::SET_GAME_PAUSED(true);
		//WAIT(waitTimeAfterSetCamera);
		//saveBuffersAndAnnotations(pathStencil);
		//GAMEPLAY::SET_GAME_PAUSED(false);


		saveScreenImage(pathImage);
		

	}
	
	
		
	imageCountPerCam++;
	
	
}

void Creator::startWalkingScene() {

	isWalkingSceneRunning = true;
	currentSpawnId = 0;
}

void Creator::recordAtCamSettingsLoop() {
	
	loadPedAppearanceSet();

	loadCameraSettings();
	
	for (int camId = 0; camId < cameraSettings.size(); camId++) {

		std::string camFolder = "cam_" + std::to_string(camId) + "\\";
		std::string camFolderPath = this->output_path + camFolder;
		_mkdir(camFolderPath.c_str());

		std::string coordsCamPath = camFolderPath + "coords_cam_" + std::to_string(camId) + ".csv";

		std::shared_ptr<std::ofstream> coordsCamFile = std::make_shared<std::ofstream>(coordsCamPath);

		(*coordsCamFile) << "frame_no_gta,frame_no_cam,ped_id,spawn_id,appearance_id,joint_type,x_2D_joint,y_2D_joint,x_3D_joint,y_3D_joint,z_3D_joint,joint_occluded,joint_self_occluded,";
		(*coordsCamFile) << "x_3D_cam,y_3D_cam,z_3D_cam,x_rot_cam,y_rot_cam,z_rot_cam,fov,x_3D_person,y_3D_person,z_3D_person,";
		(*coordsCamFile) << "x_2D_person,y_2D_person,ped_type,wears_glasses,yaw_person,hours_gta,minutes_gta,seconds_gta,x_top_left_BB,y_top_left_BB,x_bottom_right_BB,y_bottom_right_BB\n";
		camCoordsFiles.push_back(coordsCamFile);
		

	}

	pedTasksLog.open(output_path + "pedTasksLog.csv");
	frameRateLog.open(output_path + "frameRateLog.txt");
	imageCountPerCam = 0;
	
	recordInLoop = true;
	startRecordingTime = std::chrono::system_clock::now();

}


void Creator::deletePedAppearanceSetNonHumans() {
	loadPedAppearanceSet();

	Vector3 pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), TRUE);
	std::unordered_set<PedAppearance, PedAppearance::Hash>::iterator it = pedAppearanceSet.begin();

	while (it != pedAppearanceSet.end()) {
		auto myPedAppearance = (*it);
		Ped ped = myPedAppearance.createPed(pp.x, pp.y, pp.z);
		WAIT(100);
		it++;
		if (!PED::IS_PED_HUMAN(ped)) {
			pedAppearanceSet.erase(myPedAppearance);
			set_status_text("deleted non human ped");
		}
		PED::DELETE_PED(&ped);

	}

	savePedAppearanceSet();
}

void Creator::logFramerate() {
	
	if (recordInLoop) {
		frameRateLog << std::to_string(1.0f / GAMEPLAY::GET_FRAME_TIME()) 
			+ "," + std::to_string(GAMEPLAY::GET_FRAME_COUNT()) + "," + std::to_string(GAMEPLAY::GET_GAME_TIMER()) + "\n" ;
	}
	
}

void Creator::logPedSpawnProgress(int numberOfSpawnedPeds, int maxSpawnablePeds, std::shared_ptr<ScenarioGroup> group) {

	std::ofstream logPedSpawnProgress(output_path + "log_ped_spawn_progress.txt", std::ofstream::app);
	logPedSpawnProgress << "\n";
	logPedSpawnProgress << "Spawned ped " + std::to_string(numberOfSpawnedPeds) + " of "
		+ std::to_string(maxSpawnablePeds) << "\n";


	
	auto currentTime = std::chrono::system_clock::now();

	std::chrono::duration<double> elapsed_seconds = currentTime - startRecordingTime;

	std::time_t current_time_t = std::chrono::system_clock::to_time_t(currentTime);
	logPedSpawnProgress << "current time: " << std::ctime(&current_time_t);

	logPedSpawnProgress << "elapsed time: " << elapsed_seconds.count() << "s\n";

	logPedSpawnProgress << group->to_string();

	logPedSpawnProgress.close();
}

//Has to be looped by the update function because otherwise it will block other tasks. For example the possibility to set  
// isWalkingSceneRunning to false. Or checking if a ped has arrived at his goal.
void Creator::runWalkingScenes() {
	
	if (!isWalkingSceneRunning) {
		return;
	}
	
	
	loadPedAppearanceSet();
	loadPedTracks();
	loadPedScenario();
	

	if (pedTracks.empty()) {
		return;
	}

	int pedAppsItDistance = (int)std::distance(pedAppearanceVector.begin(), pedAppsIt);
	

	

	if (isWalkingSceneRunning && pedAppsIt != pedAppearanceVector.end()) {
		
		

		auto freeGroup = pedScenario.getFreeGroup((int)spawnedPeds.size(), pedAppsItDistance);

		if (freeGroup == nullptr) {
			
			return;
		}


		freeGroup->spawned_ped_count++;

		int trackIdx = freeGroup->track;
		

		if (pedTracks[trackIdx].empty()) {
			return;
		}
		
		logPedSpawnProgress(pedAppsItDistance, (int)pedAppearanceVector.size(), freeGroup);

		if (this->is_debug) {
			set_status_text("current ped spawned: " + std::to_string(pedAppsItDistance) + " of "
				+ std::to_string(pedAppearanceSet.size()));
		}
		

		PedAppearance pedAppearance = (*pedAppsIt);
		pedAppsIt++;
		

		std::vector<TrackPosition> choosenTrack = pedTracks[trackIdx];

		
		
		if (freeGroup->reverseTrack) {
			std::reverse(choosenTrack.begin(), choosenTrack.end());
		}
		

		std::ofstream spawnedPedTracks(output_path + "spawnedPedTracks.txt", std::ofstream::app);
		//log ped track of spawned ped
		spawnedPedTracks << "track_idx: " + std::to_string(trackIdx) << "\n";
		spawnedPedTracks << "appearance id: " + std::to_string(pedAppearance.identityNo) << "\n";
		for (auto trackPos : choosenTrack) {
			spawnedPedTracks << trackPos.to_csv() << "\n";
		}

		spawnedPedTracks.close();

		Ped ped = pedAppearance.createPed(choosenTrack);

		
		PedSpawned pedSpawned(ped, freeGroup->nodeTasks, pedAppearance, currentSpawnId);
		
		
		pedSpawned.followEntity(choosenTrack, freeGroup->speed, freeGroup->spawned_ped_count);
		spawnedPeds.insert(std::make_pair(ped,pedSpawned));
		currentSpawnId++; //A second id to identify spawned peds because we dont know if the pedId wont be reused after the ped is deleted.

	}

	

}




void Creator::viewCameraView(int increment) {

	loadCameraSettings();

	if (cameraSettings.empty()) {
		return;
	}
	
	int newIndex = (viewCameraIndex + increment);

	if (newIndex < 0) {
		newIndex = (int)(cameraSettings.size() - 1);
	}
	viewCameraIndex = newIndex % cameraSettings.size();
	set_status_text("Camera  " + std::to_string(viewCameraIndex));


	CameraSetting cameraView = cameraSettings[viewCameraIndex];

	setCamera(cameraView.position, cameraView.rotation);
	


}



void Creator::drawCircleInXY(float radius, float x, float y, float z, int steps, int r, int g, int b, int alpha) {
	const double pi = std::acos(-1);
	float stepWidth = 2 * (float)pi / (float)steps;

	float xStartCoord = radius * (float)std::cos(0) + x;
	float yStartCoord = radius * (float)std::sin(0) + y;
	float xEnd;
	float yEnd;
	for (float theta = stepWidth; theta < 2 * (float)pi; theta += stepWidth) {
		float xStart = radius * (float)std::cos(theta - stepWidth) + x;
		float yStart = radius * (float)std::sin(theta - stepWidth) + y;
		xEnd = radius * (float)std::cos(theta) + x;
		yEnd = radius * (float)std::sin(theta) + y;


		GRAPHICS::DRAW_LINE(xStart, yStart, z,
			xEnd, yEnd, z, r, g, b, alpha);
	}
	//Connect last point to start point
	GRAPHICS::DRAW_LINE(xEnd, yEnd, z,
		xStartCoord, yStartCoord, z, r, g, b, alpha);

}

void Creator::visualizeNetwork() {

	if (networkNodes.empty() || !showNetwork) {
		return;
	}

	for (int i = 0; i < networkNodes.size(); i++) {
		

		if (networkNodes[i]->type == PathNetworkNode::START_TYPE) {
			DrawableBox(networkNodes[i]->trackPosition.x,
				networkNodes[i]->trackPosition.y,
				networkNodes[i]->trackPosition.z, 0.3f, 255, 0, 0, 150).draw();

		}
		else {

			DrawableBox(networkNodes[i]->trackPosition.x,
				networkNodes[i]->trackPosition.y,
				networkNodes[i]->trackPosition.z, 0.3f, 0, 255, 0, 150).draw();
		}

		


		drawCircleInXY(networkNodes[i]->trackPosition.randomRadius, 
			networkNodes[i]->trackPosition.x,
			networkNodes[i]->trackPosition.y,
			networkNodes[i]->trackPosition.z, 10, 0, 0, 255, 255);

	}
	auto networkEdgesIt = networkEdges.begin();
	for (int i = 0; i < networkEdges.size(); i++) {

		
		GRAPHICS::DRAW_LINE((*networkEdgesIt).first_->trackPosition.x,
			(*networkEdgesIt).first_->trackPosition.y,
			(*networkEdgesIt).first_->trackPosition.z,
			(*networkEdgesIt).second_->trackPosition.x,
			(*networkEdgesIt).second_->trackPosition.y,
			(*networkEdgesIt).second_->trackPosition.z, 0, 255, 0, 255);

		networkEdgesIt++;
	}


}

void Creator::visualizeTrack() {
	
	if (!showTracks ||  pedTracks.empty()) {
		
		return;
	}
	std::vector<TrackPosition> trackToVisualize = pedTracks[currentTrack];

	for (int i = 0; i < trackToVisualize.size(); i++) {
		if (i == 0) {
			DrawableBox(trackToVisualize[i].x, trackToVisualize[i].y, trackToVisualize[i].z, 0.3f, 255, 0, 0, 150).draw();
		} 
		else if (i == trackToVisualize.size() - 1) {
			DrawableBox(trackToVisualize[i].x, trackToVisualize[i].y, trackToVisualize[i].z, 0.3f, 0, 255, 0, 150).draw();

		} else {
			DrawableBox(trackToVisualize[i].x, trackToVisualize[i].y, trackToVisualize[i].z, 0.3f, 0, 0, 255, 150).draw();

		}

		drawCircleInXY(trackToVisualize[i].randomRadius, trackToVisualize[i].x, trackToVisualize[i].y, trackToVisualize[i].z, 10, 0, 0, 255, 255);

		if (i >= 1) {
			GRAPHICS::DRAW_LINE(trackToVisualize[i - 1].x, trackToVisualize[i - 1].y, trackToVisualize[i - 1].z,
				trackToVisualize[i].x, trackToVisualize[i].y, trackToVisualize[i].z, 0, 255, 0, 255);
		}
			
	}


	
}

void Creator::moveNearestTrackPositionToPlayer() {
	int nearestTrackPositionIndex = getNearestTrackPosition();
	Vector3 pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), TRUE);

	if (pedTracks.size() == 0) {
		return;
	}

	if (pedTracks[currentTrack].size() == 0) {
		return;
	}

	pedTracks[currentTrack][nearestTrackPositionIndex].x = pp.x;
	pedTracks[currentTrack][nearestTrackPositionIndex].y = pp.y;
	pedTracks[currentTrack][nearestTrackPositionIndex].z = pp.z;
}


int Creator::getNearestTrackPosition() {
	if (pedTracks.size() == 0) {
		return 0;
	}
	

	std::vector<TrackPosition> track = pedTracks[currentTrack];

	if (track.size() == 0) {
		return 0;
	}

	Vector3 pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), TRUE);
	int nearestPos = 0;
	float currentMin = 10e10f;
	for (int i = 0; i < track.size(); i++) {
		
		
		float newMin = std::sqrt(std::pow((track[i].x - pp.x), 2) + std::pow((track[i].y - pp.y), 2) + std::pow((track[i].y - pp.y), 2));
		if (newMin < currentMin) {
			nearestPos = i;
			currentMin = newMin;
		}

	}

	return nearestPos;
}

float Creator::getTrackPosNearestRandomRadius() {

	int nearestTrackPositionIndex = getNearestTrackPosition();

	if (pedTracks.size() == 0) {
		return defaultRandomRadius;
	}

	if (pedTracks[currentTrack].size() == 0) {
		return defaultRandomRadius;
	}

	return pedTracks[currentTrack][nearestTrackPositionIndex].randomRadius;

}



float Creator::getNodePosNearestRandomRadius() {

	if (networkNodes.empty()) {

		return 0.0f;
	}

	Vector3 pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), TRUE);
	auto nn = findNearestNode(pp);


	return nn->trackPosition.randomRadius;

}


void Creator::incTrackPosNearestRandomRadius(float value) {

	int nearestTrackPositionIndex = getNearestTrackPosition();

	if (pedTracks.size() == 0) {
		return;
	}

	if (pedTracks[currentTrack].size() == 0) {
		return;
	}

	pedTracks[currentTrack][nearestTrackPositionIndex].randomRadius += value;

}

void Creator::incNodeNearestRandomRadius(float value) {
	
	if (networkNodes.empty()) {

		return;
	}

	Vector3 pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), TRUE);
	auto nn = findNearestNode(pp);

	nn->trackPosition.randomRadius += value;

}

void Creator::saveNetworkEdges() {

	std::ofstream networkEdgesFile(networkEdgesPath);

	for (auto edge : networkEdges) {
		networkEdgesFile << edge.to_csv() + "\n";
	}

	networkEdgesFile.close();
}



void Creator::loadNetworkEdges() {

	
	

	if (!networkEdges.empty()) {
		return;
	}

	loadNetworkNodes();

	auto networkEdgeStrings = readStringCSV(networkEdgesPath);

	auto compareNodeId = [](int nodeId) {
		return [nodeId](std::shared_ptr<PathNetworkNode> node) {
			return node->id == nodeId;
		};
	};

	for (auto edge : networkEdgeStrings) {
		int firstNodeId = std::stoi(edge[0]);
		int secondNodeId = std::stoi(edge[1]);

		auto firstNode = std::find_if(networkNodes.begin(), networkNodes.end(), compareNodeId(firstNodeId));
		auto secondNode = std::find_if(networkNodes.begin(), networkNodes.end(), compareNodeId(secondNodeId));

		if (firstNode == networkNodes.end() || secondNode == networkNodes.end()) {
			set_status_text("Edge ID not found in Node List");
			continue;
		}

		networkEdges.insert(PathNetworkEdge(*firstNode, *secondNode));

		(*firstNode)->connectedNodes.push_back(*secondNode);
		(*secondNode)->connectedNodes.push_back(*firstNode);

		
	}
}

void Creator::loadPedScenario() {
	
	loadPedTracks();
	loadNodeTasks();
	if (pedScenario.scenarioEntries.empty()) {
		std::vector<std::vector<std::string>> scenarioStrings = readStringCSV(scenarioPath);

		generateNodeToTrackMappings();
		pedScenario.nodeTasks = &nodeTasks;
		pedScenario.nodeToTrack = &nodesToTracks;
		pedScenario.loadScenario(scenarioStrings,(int)pedTracks.size());

	
	}
	
	

}

void Creator::loadNetworkNodes() {
	if (!networkNodes.empty()) {
		return;
	}
	std::vector<std::vector<std::string>> networkNodeStrings = readStringCSV(networkNodesPath);

	if (networkNodeStrings.empty()) {
		return;
	}


	for (std::vector<std::string> node : networkNodeStrings) {
		auto newNode = std::make_shared<PathNetworkNode>();
		newNode->id = std::stoi(node[0]);
		currentMaxNodeId = std::max<int>(newNode->id, currentMaxNodeId);
		newNode->type = std::stoi(node[1]);
		newNode->trackPosition = TrackPosition(std::stof(node[2]), std::stof(node[3]),std::stof(node[4]),std::stof(node[5]));
		networkNodes.push_back(newNode);
	}

}

void Creator::loadPedTracks() {

	if (!pedTracks.empty()) {
		return;
	}

	std::vector<std::vector<std::string>> pedStringTracks = readStringCSV(walkingPositionsPath);

	
	
	for (std::vector<std::string> pedTrack : pedStringTracks) {
		int _currentTrack = std::stoi(pedTrack[0]);
		
		if (pedTracks.size() <= _currentTrack) {
			pedTracks.push_back(std::vector<TrackPosition>());
		}
		
		pedTracks[_currentTrack].push_back(TrackPosition(std::stoi(pedTrack[1])
			, std::stof(pedTrack[2])
			, std::stof(pedTrack[3])
			, std::stof(pedTrack[4])
			, std::stof(pedTrack[5])));
	}

}

void Creator::deleteWallGroups() {
	while (wallGroups.size() > 0) {
		(*wallGroups.begin()).deleteWallElements();
		wallGroups.erase(wallGroups.begin());

	}

}


void Creator::saveNetworkNodes() {

	std::ofstream networkNodesFile(networkNodesPath);

	for (int i = 0; i < networkNodes.size(); i++) {
		networkNodesFile << networkNodes[i]->to_csv() + "\n";
	}
	networkNodesFile.close();
}

void Creator::loadWallGroups() {
	if (!wallGroups.empty()) {
		return;
	}
	
	Entity player = PLAYER::PLAYER_PED_ID();

	Vector3 currentPlayerPosition = ENTITY::GET_ENTITY_COORDS(player, true);

	deleteWallGroups();

	//To delete all spawned objects by going far away (Maze Bank)
	Vector3 deleteObjectPlace;
	deleteObjectPlace.x = -1378.818848f;
	deleteObjectPlace.y = -524.813354f;
	deleteObjectPlace.z = 31.031649f;
	ENTITY::SET_ENTITY_COORDS_NO_OFFSET(player, deleteObjectPlace.x, deleteObjectPlace.y, deleteObjectPlace.z, 0, 0, 1);
	WAIT(500);
	ENTITY::SET_ENTITY_COORDS_NO_OFFSET(player, currentPlayerPosition.x, currentPlayerPosition.y, currentPlayerPosition.z, 0, 0, 1);


	std::vector<std::vector<std::string>> wallGroupLinesCsv = readStringCSV(wallGroupsPath);

	if (wallGroupLinesCsv.empty()) {
		return;
	}

	currentWallGroup = 0;
	
	for (std::vector<std::string> wallGroupStrings : wallGroupLinesCsv) {
		

		WallGroup newWallGroup(wallGroupStrings);
		newWallGroup.isMovable = false;
		newWallGroup.setVisibility(false);
		wallGroups.push_back(newWallGroup);
		
	}

}

void Creator::saveWallGroups() {
	if (wallGroups.empty()) {
		return;
	}

	std::ofstream wallGroupsFile(wallGroupsPath);
	for (int i = 0; i < wallGroups.size(); i++) {
		wallGroupsFile << wallGroups[i].to_csv() + "\n";
	}

	wallGroupsFile.close();
}

void Creator::savePedTracks() {
	
	if (pedTracks.empty()) {
		return;
	}

	std::ofstream pedTracksFile(walkingPositionsPath);

	for (int trackNo = 0; trackNo < pedTracks.size(); trackNo++) {
		for (TrackPosition trackPos : pedTracks[trackNo]) {
			pedTracksFile << std::to_string(trackNo) + "," + trackPos.to_csv() + "\n";
		}


	}


	pedTracksFile.close();
}

void Creator::runSpawnedPedActions() {
	
	auto spawnedPedsIter = spawnedPeds.begin();
	

	while (!spawnedPeds.empty() && spawnedPedsIter != spawnedPeds.end()) {
		(*spawnedPedsIter).second.runActions();
		
			
		

		if ((*spawnedPedsIter).second.isPedDeleted) {
			spawnedPedsIter = spawnedPeds.erase(spawnedPedsIter);
		}
		else {
			spawnedPedsIter++;
		}
		
	}

}


std::shared_ptr<PathNetworkNode> Creator::findNearestNode(Vector3 pos) {
	std::shared_ptr<PathNetworkNode> currentNearestNode;
	float currentMin = 10e10f;

	for (int i = 0; i < networkNodes.size(); i++) {

		float newMin = std::sqrt(std::pow((networkNodes[i]->trackPosition.x - pos.x), 2)
			+ std::pow((networkNodes[i]->trackPosition.y - pos.y), 2)
			+ std::pow((networkNodes[i]->trackPosition.z - pos.z), 2));

		if (newMin < currentMin) {
			currentMin = newMin;
			currentNearestNode = networkNodes[i];
		}
	}

	return currentNearestNode;
}

std::tuple<int, int> Creator::findTwoNearestNodeIndices(Vector3 pos) {

	

	int firstNearestNodeIndex = -1;
	int secondNearestNodeIndex = -1;
	float firstMin = 10e10f;
	float secondMin = 10e10f;
	for (int i = 0; i < networkNodes.size(); i++) {

		float newMin = std::sqrt(std::pow((networkNodes[i]->trackPosition.x - pos.x), 2)
			+ std::pow((networkNodes[i]->trackPosition.y - pos.y), 2)
			+ std::pow((networkNodes[i]->trackPosition.z - pos.z), 2));

		if (newMin < firstMin) {
			secondMin = firstMin;
			firstMin = newMin;
			secondNearestNodeIndex = firstNearestNodeIndex;
			firstNearestNodeIndex = i;
		}
		else if (newMin < secondMin) {
			secondMin = newMin;
			secondNearestNodeIndex = i;
		}

		
	}
	return std::make_tuple(firstNearestNodeIndex, secondNearestNodeIndex);
}

void Creator::createNetworkMenu()
{
	const float lineWidth = 250.0;
	const int lineCount = 8;
	menuActive = true;
	networkMenuActive = true;
	loadNetworkNodes();
	loadNetworkEdges();

	std::string caption = "Create Network";

	char lines[lineCount][250] = {
		"Record Node",
		"Delete Nearest Node",
		"Connect Nearest Nodes",
		"Node Type",
		"Rand. Radius Nearest",
		"Visualize Network",
		"Move Nearest to Player",
		"Generate Tracks (Overwrite)"
	};

	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;

		std::string currentNodeTypeStr = "";
		if (currentNodeType == PathNetworkNode::START_TYPE) {
			currentNodeTypeStr = START_TYPE_STR;
		}
		else {

			currentNodeTypeStr = NO_START_TYPE_STR;

		}

		sprintf_s(lines[3], "Node Type	~y~[ %s ]", currentNodeTypeStr.c_str());

		sprintf_s(lines[4], "Rand. Radius Nearest	~y~[ %f ]", getNodePosNearestRandomRadius());

		sprintf_s(lines[5], "Visualize Network  ~y~[ %d ]", showNetwork);

		do
		{
			// draw menu
			draw_menu_line(caption, lineWidth, mHeight, mHeight * 2, mLeft, mTitle, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexCreateNetwork)
					draw_menu_line(lines[i], lineWidth, mHeight, mTopFlat + i * mTop, mLeft, mHeight, false, false);
			draw_menu_line(lines[activeLineIndexCreateNetwork], lineWidth, mHeight, mTopFlat + activeLineIndexCreateNetwork * mTop, mLeft, mHeight, true, false);

			if (!subMenuActive)
				updateNew();
			else
				return;
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		updateNew();
		// process buttons
		if (bEnter)
		{
			std::shared_ptr<PathNetworkNode> nearestNode;
			std::shared_ptr <PathNetworkNode> newNode = std::make_shared<PathNetworkNode>();
			Vector3 pp;
			std::vector<std::shared_ptr<PathNetworkNode>>::iterator toEraseIter;
			switch (activeLineIndexCreateNetwork)
			{
			case 0:
			{
				
				//create new node
				showNetwork = true;

				pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), TRUE);
				newNode->trackPosition = TrackPosition(pp.x, pp.y, pp.z, 1.0f);
				newNode->type = currentNodeType;

				currentMaxNodeId++;
				newNode->id = currentMaxNodeId;

				//connect to nearest network node if it exists
				if (!networkNodes.empty()) {
					std::shared_ptr<PathNetworkNode> nearestNode = findNearestNode(pp);
					networkEdges.insert(PathNetworkEdge(nearestNode, newNode));
					nearestNode->connectedNodes.push_back(newNode);
					newNode->connectedNodes.push_back(nearestNode);
				}

				networkNodes.push_back(newNode);
				break;
			}
			case 1:
				//Erase nearest node
				if (networkNodes.empty()) {
					break;
				}
				showNetwork = true;
				pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), TRUE);
				nearestNode = findNearestNode(pp);

				toEraseIter = std::find(networkNodes.begin(), networkNodes.end(), nearestNode);
				
				if (networkNodes.end() != toEraseIter) {
					networkNodes.erase(toEraseIter);
				}
				

				for (int i = 0; i < networkNodes.size(); i++) {

					auto nodeToDeleteIter = std::find(networkNodes[i]->connectedNodes.begin(), 
						networkNodes[i]->connectedNodes.end(), nearestNode);

					if (nodeToDeleteIter != networkNodes[i]->connectedNodes.end()) {
						networkNodes[i]->connectedNodes.erase(nodeToDeleteIter);
					}

				}

				{
					
					auto networkEdgesIter = networkEdges.begin();
					while (networkEdgesIter != networkEdges.end()) {

						if ((*networkEdgesIter).first_ == nearestNode || (*networkEdgesIter).second_ == nearestNode) {
							networkEdges.erase(networkEdgesIter);
						}
						
						networkEdgesIter++;
					}

				}
				break;
			
			case 2:

			{
				//Connect nearest nodes
				pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), TRUE);
				auto nearestNodes = findTwoNearestNodeIndices(pp);
				showNetwork = true;


				auto firstIndex = std::get<0>(nearestNodes);
				auto secondIndex = std::get<1>(nearestNodes);
				if (firstIndex == -1 || secondIndex == -1) {
					set_status_text("No two nearest nodes found.");
					break;
				}

				auto firstNode = networkNodes[firstIndex];
				auto secondNode = networkNodes[secondIndex];

				auto newEdge = PathNetworkEdge(firstNode, secondNode);
				
				/**
				//Check if such an edge exists
				for (int i = 0; i < networkEdges.size(); i++) {
					if ((networkEdges[i].first_ == firstNode && networkEdges[i].second_ == secondNode) ||
						(networkEdges[i].first_ == secondNode && networkEdges[i].second_ == firstNode)) {
						set_status_text("Edge already exists");
						break;
					}
				}

				**/

				if (networkEdges.find(newEdge) != networkEdges.end()) {
					set_status_text("Edge already exists");
					break;
				}

				networkEdges.insert(newEdge);
				firstNode->connectedNodes.push_back(secondNode);
				secondNode->connectedNodes.push_back(firstNode);
				break;

			}
			case 3:
				//Change Node Type

				if (networkNodes.empty()) {
					break;
				}
				showNetwork = true;
				pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), TRUE);
				nearestNode = findNearestNode(pp);

				

				currentNodeType = nearestNode->type;

				currentNodeType = (currentNodeType + 1) % nodeTypeCount;

				nearestNode->type = currentNodeType;

				break;

			case 4:
				//Change random radius
				incNodeNearestRandomRadius(0.1f);
				showNetwork = true;
				break;

			case 5:
				showNetwork = !showNetwork;

				break;

			case 6:
				//Move nearest Node to player
				if (networkNodes.empty()) {
					break;
				}

				showNetwork = true;
				pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), TRUE);
				nearestNode = findNearestNode(pp);

				nearestNode->trackPosition.x = pp.x;
				nearestNode->trackPosition.y = pp.y;
				nearestNode->trackPosition.z = pp.z;


				break;

			case 7:
				//Generate Tracks
				auto allPaths = getAllPathsFromAllStartEndCombinations();

				if (allPaths.empty()) {
					break;
				}

				set_status_text("Number of paths: " + std::to_string(allPaths.size()));
				pedTracks.clear();

				for (int i = 0; i < allPaths.size(); i++) {
					std::vector<TrackPosition> track;
					for (int j = 0; j < allPaths[i].size(); j++) {
						TrackPosition trackPos = allPaths[i][j]->trackPosition;
						trackPos.nodeId = allPaths[i][j]->id;
						track.push_back(trackPos);
					}
					pedTracks.push_back(track);
				}
				savePedTracks();
				break;
			}
			
			saveNetworkNodes();
			saveNetworkEdges();
			waitTime = 200;
		}
		if (bBack)
		{
			resetMenuCommands();
			break;
		}
		else if (bQuit)
		{
			resetMenuCommands();
			bQuit = true;
			break;
		}
		else if (bUp)
		{
			if (activeLineIndexCreateNetwork == 0)
				activeLineIndexCreateNetwork = lineCount;
			activeLineIndexCreateNetwork--;
			waitTime = 150;
		}
		else if (bDown)
		{
			activeLineIndexCreateNetwork++;
			if (activeLineIndexCreateNetwork == lineCount)
				activeLineIndexCreateNetwork = 0;
			waitTime = 150;
		}

		//Change random Radius Nearest
		if (activeLineIndexCreateNetwork == 4)
		{

			if (bLeft) {
				incNodeNearestRandomRadius(-0.1f);
			}
			else if (bRight) {
				incNodeNearestRandomRadius(0.1f);
			}
			
		}
		
		
		resetMenuCommands();
	}
}

void Creator::createTracksMenu()
{
	const float lineWidth = 250.0;
	const int lineCount = 8;
	menuActive = true;
	trackMenuActive = true;
	loadPedTracks();

	std::string caption = "Create Tracks";

	char lines[lineCount][200] = {
		"New Track",
		"Delete Track",
		"Current Track",
		"Rand. Radius Nearest",
		"Delete Last Position",
		"Record Ped Position",
		"Move Nearest Track Pos",
		"Visualize Tracks"
		
	};

	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;

		
		sprintf_s(lines[2], "Current Track	~y~[ %d ]", currentTrack);
		sprintf_s(lines[3], "Rand. Radius Nearest	~y~[ %f ]", getTrackPosNearestRandomRadius());
		sprintf_s(lines[7], "Visualize Tracks  ~y~[ %d ]", showTracks);

		do
		{
			// draw menu
			draw_menu_line(caption, lineWidth, mHeight, mHeight * 2, mLeft, mTitle, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexCreateTracks)
					draw_menu_line(lines[i], lineWidth, mHeight, mTopFlat + i * mTop, mLeft, mHeight, false, false);
			draw_menu_line(lines[activeLineIndexCreateTracks], lineWidth, mHeight, mTopFlat + activeLineIndexCreateTracks * mTop, mLeft, mHeight, true, false);

			if (!subMenuActive)
				updateNew();
			else
				return;
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		updateNew();
		// process buttons
		if (bEnter)
		{
			std::vector<TrackPosition> newTrack;
			
			switch (activeLineIndexCreateTracks)
			{
			case 0:
				//Create new track
				showTracks = true;
				pedTracks.push_back(newTrack);
				currentTrack = (int)pedTracks.size() - 1;
				break;
			case 1:
				//Delete track
				showTracks = true;
				if (pedTracks.size() > 0) {
					pedTracks.erase(pedTracks.begin()+currentTrack);
					currentTrack = 0;

				}
				break;
			case 2:
				//Switch current track
				showTracks = true;
				if (pedTracks.size() > 0) {
					currentTrack = (currentTrack + 1) % pedTracks.size();
				}
				
				break;
			case 3:
				//Random Radius Nearest
				showTracks = true;
				incTrackPosNearestRandomRadius(0.1f);
				

				break;

			case 4:
				//Delete Last Position
				showTracks = true;
				if (pedTracks.size() > 0) {
					if (!pedTracks[currentTrack].empty()) {
						pedTracks[currentTrack].pop_back();
					}

				}

				break;

			case 5:
				//Record Position
				showTracks = true;
				Vector3 pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), TRUE);
				

				if (pedTracks.size() == 0) {
					pedTracks.push_back(newTrack);
				}
				pedTracks[currentTrack].push_back(TrackPosition(pp.x, pp.y, pp.z, getTrackPosNearestRandomRadius()));


				break;
			case 6:
				showTracks = true;
				moveNearestTrackPositionToPlayer();
				break;
			case 7:
				showTracks = !showTracks;
				break;
			
			}
			savePedTracks();
			waitTime = 200;
		}
		if (bBack)
		{
			resetMenuCommands();
			break;
		}
		else if (bQuit)
		{
			resetMenuCommands();
			bQuit = true;
			break;
		}
		else if (bUp)
		{
			if (activeLineIndexCreateTracks == 0)
				activeLineIndexCreateTracks = lineCount;
			activeLineIndexCreateTracks--;
			waitTime = 150;
		}
		else if (bDown)
		{
			activeLineIndexCreateTracks++;
			if (activeLineIndexCreateTracks == lineCount)
				activeLineIndexCreateTracks = 0;
			waitTime = 150;
		}

		
		if (activeLineIndexCreateTracks == 2)
		{
			if (bLeft) {
				if (pedTracks.size() > 0) {
					currentTrack = (currentTrack - 1);
					if (currentTrack < 0) {
						currentTrack = (int)(pedTracks.size() - 1);
					}

				}
			}
			else if (bRight) {
				if (pedTracks.size() > 0) {
					currentTrack = (currentTrack + 1) % pedTracks.size();
				}
			}
		}
		//Change random Radius Nearest
		if (activeLineIndexCreateTracks == 3)
		{
			
			if (bLeft) {
				incTrackPosNearestRandomRadius(-0.1f);
			}
			else if (bRight) {
				incTrackPosNearestRandomRadius(0.1f);
			}
			savePedTracks();
		}
		resetMenuCommands();
	}
}

void Creator::createWallMenu()
{
	const float lineWidth = 250.0;
	const int lineCount = 8;
	menuActive = true;
	loadWallGroups();
	

	std::string caption = "Create Wall";

	char lines[lineCount][200] = {
		"New Wall Group",
		"Delete Wall Group",
		"Current Wall Group",
		"Wall Group Rows",
		"Wall Group Columns",
		"Show Wall Groups",
		"Movable",
		"Player Wall distance"

	};

	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;


		sprintf_s(lines[2], "Curr. Wall Gr.	~y~[ %d ]", currentWallGroup);
		if (!wallGroups.empty()) {
			sprintf_s(lines[3], "Wall Group Rows ~y~[ %d ]", wallGroups[currentWallGroup].rows);
			sprintf_s(lines[4], "Wall Group Columns  ~y~[ %d ]", wallGroups[currentWallGroup].columns);
			sprintf_s(lines[6], "Movable  ~y~[ %d ]", wallGroups[currentWallGroup].isMovable ? 1 : 0);
			sprintf_s(lines[7], "Player Wall distance  ~y~[ %f ]", wallGroups[currentWallGroup].playerWallDistance);
		}

		sprintf_s(lines[5], "Show Wall Groups  ~y~[ %d ]", showWallGroups ? 1 : 0 );


		do
		{
			// draw menu
			draw_menu_line(caption, lineWidth, mHeight, mHeight * 2, mLeft, mTitle, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexCreateWallGroup)
					draw_menu_line(lines[i], lineWidth, mHeight, mTopFlat + i * mTop, mLeft, mHeight, false, false);
			draw_menu_line(lines[activeLineIndexCreateWallGroup], lineWidth, mHeight, mTopFlat + activeLineIndexCreateWallGroup * mTop, mLeft, mHeight, true, false);

			if (!subMenuActive)
				updateNew();
			else
				return;
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		updateNew();
		// process buttons
		if (bEnter)
		{
			

			switch (activeLineIndexCreateWallGroup)
			{
			case 0:
			{
				//Create New Wall Group
				WallGroup wallGroup(2, 3);
				wallGroups.push_back(wallGroup);

				currentWallGroup = wallGroups.size() -1;
				break;
			}
			case 1:
			{
				//Delete current wall group
				if (!wallGroups.empty() && currentWallGroup >= 0) {
					wallGroups[currentWallGroup].deleteWallElements();
					wallGroups.erase(wallGroups.begin() + currentWallGroup);
					currentWallGroup = currentWallGroup % wallGroups.size();
				}
				break;
			}
			case 2:
			{
				//Change current wall group
				currentWallGroup = (currentWallGroup + 1) % wallGroups.size();

				break;
			}
			case 3:
			{
				//Add rows
				if (!wallGroups.empty() && currentWallGroup >= 0) {
					wallGroups[currentWallGroup].createWallElements(wallGroups[currentWallGroup].rows + 1, wallGroups[currentWallGroup].columns);
				}
				break;
			}
			case 4:
			{
				//Add columns
				if (!wallGroups.empty() && currentWallGroup >= 0) {
					wallGroups[currentWallGroup].createWallElements(wallGroups[currentWallGroup].rows, wallGroups[currentWallGroup].columns + 1);
				}
				
				break;
			}
			case 5:
			{
				//Set visibility of all wall groups
				showWallGroups = !showWallGroups;
				for (int i = 0; i < wallGroups.size(); i++) {
					wallGroups[i].setVisibility(showWallGroups);
				}

				break;
			}
			case 6:
				//
				if (!wallGroups.empty() && currentWallGroup != -1) {
					wallGroups[currentWallGroup].isMovable = !wallGroups[currentWallGroup].isMovable;
				}
				

				break;

			case 7:
				if (!wallGroups.empty() && currentWallGroup != -1) {
					wallGroups[currentWallGroup].playerWallDistance -= 1.0f;
				}
				break;
			}
			saveWallGroups();
			waitTime = 200;
		}
		if (bBack)
		{
			resetMenuCommands();
			break;
		}
		else if (bQuit)
		{
			resetMenuCommands();
			bQuit = true;
			break;
		}
		else if (bUp)
		{
			if (activeLineIndexCreateWallGroup == 0)
				activeLineIndexCreateWallGroup = lineCount;
			activeLineIndexCreateWallGroup--;
			waitTime = 150;
		}
		else if (bDown)
		{
			activeLineIndexCreateWallGroup++;
			if (activeLineIndexCreateWallGroup == lineCount)
				activeLineIndexCreateWallGroup = 0;
			waitTime = 150;
		}

		//change current wall group
		if (activeLineIndexCreateWallGroup == 2)
		{

			if (bLeft) {
				currentWallGroup = (currentWallGroup - 1);
				if (currentWallGroup < 0) {
					currentWallGroup = (int)(wallGroups.size() - 1);
				}
			}
			else if (bRight) {
				currentWallGroup = (currentWallGroup + 1) % wallGroups.size();
			}
			
		}

		if (activeLineIndexCreateWallGroup == 3)
		{
			if (wallGroups.empty()) {
				return;
			}

			if (bLeft) {
				wallGroups[currentWallGroup].createWallElements(wallGroups[currentWallGroup].rows - 1, wallGroups[currentWallGroup].columns);
			}
			else if (bRight) {
				wallGroups[currentWallGroup].createWallElements(wallGroups[currentWallGroup].rows + 1, wallGroups[currentWallGroup].columns);
			}
			saveWallGroups();
		}

		if (activeLineIndexCreateWallGroup == 4)
		{
			if (wallGroups.empty()) {
				return;
			}
			if (bLeft) {
				wallGroups[currentWallGroup].createWallElements(wallGroups[currentWallGroup].rows, wallGroups[currentWallGroup].columns - 1);
			}
			else if (bRight) {
				wallGroups[currentWallGroup].createWallElements(wallGroups[currentWallGroup].rows, wallGroups[currentWallGroup].columns + 1);
			}
			saveWallGroups();
		}


		if (activeLineIndexCreateWallGroup == 7)
		{
			if (wallGroups.empty()) {
				return;
			}
			if (bLeft) {
				if (currentWallGroup != -1) {
					wallGroups[currentWallGroup].playerWallDistance -= 1.0f;
				}
			}
			else if (bRight) {
				if (currentWallGroup != -1) {
					wallGroups[currentWallGroup].playerWallDistance += 1.0f;
				}
			}
			saveWallGroups();
		}
		
		resetMenuCommands();
	}
}


void Creator::createScenarioMenu()
{

	 


	const float lineWidth = 250.0;
	const int lineCount = 7;
	menuActive = true;
	scenarioMenuActive = true;
	loadNetworkEdges();
	loadNetworkNodes();
	loadPedScenario();
	loadNodeTasks();

	std::string caption = "Create Scenario";

	char lines[lineCount][200] = {
		"Current Spawn Stream",
		"Group Count",
		"Group Size Start",
		"Group Size End",
		"Wait Time",
		"Visualize Node Tasks",
		"Increment by"
		
	};

	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;


		sprintf_s(lines[0], "Current Spawn Stream	~y~[ %d ]", currentSpawnStream);
		sprintf_s(lines[1], "Group Count	~y~[ %d ]", pedScenario.scenarioEntries[currentSpawnStream].group_count);
		sprintf_s(lines[2], "Group Size Start	~y~[ %d ]", pedScenario.scenarioEntries[currentSpawnStream].group_size_start);
		sprintf_s(lines[3], "Group Size End	  ~y~[ %d ]", pedScenario.scenarioEntries[currentSpawnStream].group_size_end);
		sprintf_s(lines[5], "Visualize Node Tasks	~y~[ %s ]", std::to_string(showTaskNodes).c_str());
		sprintf_s(lines[6], "Increment by	~y~[ %f ]", incrementByScenarioMenu);


		auto nearestNode = findNearestNode(ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true));

		DrawableBox(nearestNode->trackPosition.x, nearestNode->trackPosition.y, nearestNode->trackPosition.z, 0.6f, 255, 0, 0, 155).draw();

		auto nearestTask = nodeTasks.find(NodeTask(nearestNode->id,currentSpawnStream, -1.0f));
		
		if (nearestTask != nodeTasks.end()) {
			sprintf_s(lines[4], "Wait time	~y~[ %f ]", (*nearestTask).waitTime_);
		}
		else {
			sprintf_s(lines[4], "Wait time	~y~[ %f ]", 0.0f);
			
		}

		auto incrementCurrentWaitTime = [nearestTask, this, nearestNode](float incrementVal) {
			if (nearestTask != nodeTasks.end()) {
				float taskWaitTime = (*nearestTask).waitTime_ + incrementVal;
				int taskNodeId = (*nearestTask).nodeId_;

				nodeTasks.erase(nearestTask);
				if (taskWaitTime > 0.001f) {
					nodeTasks.insert(NodeTask(taskNodeId, currentSpawnStream, taskWaitTime));
				}

			}
			else {
				auto newNodeTask = NodeTask(nearestNode->id, currentSpawnStream, 1000.0f);
				nodeTasks.insert(newNodeTask);
			}
		};

		do
		{
			// draw menu
			draw_menu_line(caption, lineWidth, mHeight, mHeight * 2, mLeft, mTitle, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexCreateScenario)
					draw_menu_line(lines[i], lineWidth, mHeight, mTopFlat + i * mTop, mLeft, mHeight, false, false);
			draw_menu_line(lines[activeLineIndexCreateScenario], lineWidth, mHeight, mTopFlat + activeLineIndexCreateScenario * mTop, mLeft, mHeight, true, false);

			if (!subMenuActive)
				updateNew();
			else
				return;
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		updateNew();
		// process buttons
		if (bEnter)
		{

			switch (activeLineIndexCreateScenario)
			{
			case 0:


				currentSpawnStream = (currentSpawnStream + 1) % pedScenario.scenarioEntries.size();
				break;


			case 1:
				break;

			case 2:

				break;

			case 3:

				break;
			case 4:
			{
				
				incrementCurrentWaitTime(1000.0f);
				break;
			}
			case 5:

				showTaskNodes = !showTaskNodes;
				break;

			case 6:
				incrementByScenarioMenu *= incrementByFactor;
				break;
			}
			saveNodeTasks();
			waitTime = 200;


		}
		if (bBack)
		{
			resetMenuCommands();
			break;
		}
		else if (bQuit)
		{
			resetMenuCommands();
			bQuit = true;
			break;
		}
		else if (bUp)
		{
			if (activeLineIndexCreateScenario == 0)
				activeLineIndexCreateScenario = lineCount;
			activeLineIndexCreateScenario--;
			waitTime = 150;
		}
		else if (bDown)
		{
			activeLineIndexCreateScenario++;
			if (activeLineIndexCreateScenario == lineCount)
				activeLineIndexCreateScenario = 0;
			waitTime = 150;
		}

		if (activeLineIndexCreateScenario == 4)
		{
			if (bLeft) {
				incrementCurrentWaitTime(-incrementByScenarioMenu);
			}
			else if (bRight) {
				incrementCurrentWaitTime(incrementByScenarioMenu);
			}
		}


		if (activeLineIndexCreateScenario == 6)
		{
			if (bLeft) {
				incrementByScenarioMenu /= incrementByFactor;
			}
			else if (bRight) {
				incrementByScenarioMenu *= incrementByFactor;

			}
		}


	
		resetMenuCommands();
	}
}

/**
	Needed for forcing peds through tracks that contain nodes which they will perform a waiting task.
**/
void Creator::generateNodeToTrackMappings() {
	loadPedTracks();

	if (!nodesToTracks.empty()) {
		return;
	}

	for (int trackId = 0; trackId < pedTracks.size(); trackId++) {
		for (int trackPosIdx = 0; trackPosIdx < pedTracks[trackId].size(); trackPosIdx++) {


			auto foundNode = nodesToTracks.find(pedTracks[trackId][trackPosIdx].nodeId);
			if (foundNode == nodesToTracks.end()) {

				auto newNodeTrackEntry = std::make_pair(pedTracks[trackId][trackPosIdx].nodeId, std::make_shared <std::vector<int>>());
				newNodeTrackEntry.second->push_back(trackId);
				nodesToTracks.insert(newNodeTrackEntry);
			}
			else {
				foundNode->second->push_back(trackId);
			}

		}
	}
	

}

void Creator::draw_rect(float A_0, float A_1, float A_2, float A_3, int A_4, int A_5, int A_6, int A_7)
{
	GRAPHICS::DRAW_RECT((A_0 + (A_2 * 0.5f)), (A_1 + (A_3 * 0.5f)), A_2, A_3, A_4, A_5, A_6, A_7);
}

void Creator::draw_menu_line(std::string _caption, float lineWidth, float lineHeight, float lineTop, float lineLeft, float textLeft, bool active, bool title, bool rescaleText)
{
	
	// default values
	int text_col[4] = { 255, 255, 255, 255 },
		rect_col[4] = { 0, 0, 0, 190 };
	float text_scale = 0.35f;
	int font = 0;

	// correcting values for active line
	if (active)
	{
		text_col[0] = 0;
		text_col[1] = 0;
		text_col[2] = 0;

		rect_col[0] = 0;
		rect_col[1] = 180;
		rect_col[2] = 205;
		rect_col[3] = 220;

		if (rescaleText) text_scale = 0.35f;
	}

	if (title)
	{
		rect_col[0] = 0;
		rect_col[1] = 0;
		rect_col[2] = 0;

		if (rescaleText) text_scale = 0.70f;
		font = 1;
	}

	int screen_w, screen_h;
	GRAPHICS::GET_SCREEN_RESOLUTION(&screen_w, &screen_h);

	textLeft += lineLeft;

	float lineWidthScaled = lineWidth / (float)screen_w; // line width
	float lineTopScaled = lineTop / (float)screen_h; // line top offset
	float textLeftScaled = textLeft / (float)screen_w; // text left offset
	float lineHeightScaled = lineHeight / (float)screen_h; // line height

	float lineLeftScaled = lineLeft / (float)screen_w;

	// this is how it's done in original scripts

	// text upper part
	UI::SET_TEXT_FONT(font);
	UI::SET_TEXT_SCALE(0.0, text_scale);
	UI::SET_TEXT_COLOUR(text_col[0], text_col[1], text_col[2], text_col[3]);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
	UI::SET_TEXT_EDGE(0, 0, 0, 0, 0);
	UI::_SET_TEXT_ENTRY((char*)"STRING");
	UI::_ADD_TEXT_COMPONENT_STRING((LPSTR)_caption.c_str());
	UI::_DRAW_TEXT(textLeftScaled, (((lineTopScaled + 0.00278f) + lineHeightScaled) - 0.005f));

	// text lower part
	UI::SET_TEXT_FONT(font);
	UI::SET_TEXT_SCALE(0.0, text_scale);
	UI::SET_TEXT_COLOUR(text_col[0], text_col[1], text_col[2], text_col[3]);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
	UI::SET_TEXT_EDGE(0, 0, 0, 0, 0);
	UI::_SET_TEXT_GXT_ENTRY((char*)"STRING");
	UI::_ADD_TEXT_COMPONENT_STRING((LPSTR)_caption.c_str());
	int num25 = UI::_0x9040DFB09BE75706(textLeftScaled, (((lineTopScaled + 0.00278f) + lineHeightScaled) - 0.005f));

	// rect
	draw_rect(lineLeftScaled, lineTopScaled + (0.00278f),
		lineWidthScaled, ((((float)(num25)* UI::_0xDB88A37483346780(text_scale, 0)) + (lineHeightScaled * 2.0f)) + 0.005f),
		rect_col[0], rect_col[1], rect_col[2], rect_col[3]);
}

void Creator::resetMenuCommands()
{
	bEnter = false;
	bBack = false;
	bUp = false;
	bLeft = false;
	bRight = false;
	bDown = false;
	bQuit = false;
	trackMenuActive = false;

}

void Creator::setNativePedsInvisible() {
	const int maxWorldPedCount = 1500;
	Ped worldPeds[maxWorldPedCount];
	int worldPedCount = worldGetAllPeds(worldPeds, maxWorldPedCount);



	
	for (int i = 0; i < worldPedCount; i++) {
		

		if (spawnedPeds.find(worldPeds[i]) == spawnedPeds.end()
			&& worldPeds[i] != PLAYER::PLAYER_PED_ID()
			&& !PED::IS_PED_IN_ANY_VEHICLE(worldPeds[i],true)) {
			
			
			

			if (!arePedsVisible) {
				PED::SET_PED_DENSITY_MULTIPLIER_THIS_FRAME(0.0f);
				ENTITY::SET_ENTITY_VISIBLE(worldPeds[i], arePedsVisible, true);
				ENTITY::SET_ENTITY_COLLISION(worldPeds[i], arePedsVisible, true);
				
			}
			else {
				PED::SET_PED_DENSITY_MULTIPLIER_THIS_FRAME(1.0f);
				ENTITY::SET_ENTITY_VISIBLE(worldPeds[i], arePedsVisible, true);
				ENTITY::SET_ENTITY_COLLISION(worldPeds[i], arePedsVisible, true);

			}
			

		}
		
	}
}

void Creator::main_menu()
{
	const float lineWidth = 250.0;
	const int lineCount = 18;
	menuActive = true;

	std::string caption = "MTMCT DS CREATOR";

	char lines[lineCount][200] = {
		"Show Camera view",
		"Add camera view",
		"Toggle SloMo",
		"Open Create Tracks Menu",
		"Ped Walking Scene",
		"Reset Camera",
		"Log Ped Appearances",
		"Open Create Network Menu",
		"Ped visibility",
		"Take mugshots",
		"Spawn specific thing",
		"Record current cam",
		"Open Scenario Menu",
		"Show Framerate",
		"Draw Ped 2d Box",
		"Delete Camera View",
		"Start combined recording",
		"Open Create Wall Menu"

	};

	


	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;
		
		sprintf_s(lines[0], "Show Camera view	~y~[ %d ]", viewCameraIndex);
		sprintf_s(lines[4], "Ped Walking Scene	~y~[ %s ]", std::to_string(isWalkingSceneRunning).c_str());
		sprintf_s(lines[8], "Ped visibility	~y~[ %s ]", std::to_string(arePedsVisible).c_str());
		sprintf_s(lines[14], "Draw Ped 2d Box	~y~[ %s ]", std::to_string(shouldDrawPed2dBox).c_str());

		do
		{
			// draw menu
			draw_menu_line(caption, lineWidth, mHeight, mHeight * 2, mLeft, mTitle, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexMain)
					draw_menu_line(lines[i], lineWidth, mHeight, mTopFlat + i * mTop, mLeft, mHeight, false, false);
			draw_menu_line(lines[activeLineIndexMain], lineWidth, mHeight, mTopFlat + activeLineIndexMain * mTop, mLeft, mHeight, true, false);

			updateNew();
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		updateNew();
		// process buttons
		if (bEnter)
		{
			resetMenuCommands();

			switch (activeLineIndexMain)
			{
			case 0:
				//process peds menu;
				viewCameraView(1);
				break;
			case 1:
				//process camera menu;
				addCurrentCameraView();
				break;
			case 2:
				//process place menu;

				
				if (this->currTimeScale < 0.9f) {
					this->currTimeScale = 1.0f;
				}
				else {
					this->currTimeScale = this->timeScale;
				}
				set_status_text(std::to_string(this->currTimeScale));
				break;
			case 3:
				createTracksMenu();
				
				break;
			case 4:
				isWalkingSceneRunning = !isWalkingSceneRunning;
				currentSpawnId = 0;
				break;

			case 5:
				
				resetPlayerCam();
				break;
			case 6:
				logPedAppearance();
				
				break;
			case 7:
				
				createNetworkMenu();
				
				break;
			case 8:
				
				arePedsVisible = !arePedsVisible;
				break;

			case 9:
				takeMugshots();
				break;
			case 10:
				spawnSpecificThing();
				break;
			case 11:
				recordCurrentCamera();
				break;
			case 12:
				createScenarioMenu();
				break;
			case 13:
				displayFramerate = !displayFramerate;
				break;
			case 14:
				shouldDrawPed2dBox = !shouldDrawPed2dBox;
				break;
			case 15:
				deleteCurrentCameraSetting();
				break;
			case 16:
				startCombinedRecording();
				break;
			case 17:
				createWallMenu();
				break;
			}
			
			
			waitTime = 200;
		}

		if (bBack || bQuit)
		{
			menuActive = false;
			resetMenuCommands();
			break;
		}
		else if (bUp)
		{
			if (activeLineIndexMain == 0)
				activeLineIndexMain = lineCount;
			activeLineIndexMain--;
			waitTime = 150;
		}
		else if (bDown)
		{
			activeLineIndexMain++;
			if (activeLineIndexMain == lineCount)
				activeLineIndexMain = 0;
			waitTime = 150;
		}

		if (activeLineIndexMain == 0)
		{

			if (bLeft) {
				viewCameraView(-1);
			}
			else if (bRight) {
				viewCameraView(1);
			}

		}

		resetMenuCommands();
	}
}

void Creator::listenKeyStrokes() 
{
	
	if (IsKeyJustUp(VK_F6)) {
		Player mainPlayer = PLAYER::PLAYER_ID();
		
		PLAYER::CLEAR_PLAYER_WANTED_LEVEL(mainPlayer);
		if (!menuActive)
			main_menu();
		else
			bQuit = true;
	}

	if (menuActive) {
		if (IsKeyJustUp(VK_NUMPAD5))							bEnter = true;
		if (IsKeyJustUp(VK_NUMPAD0) || IsKeyJustUp(VK_BACK))	bBack = true;
		if (IsKeyJustUp(VK_NUMPAD8))							bUp = true;
		if (IsKeyJustUp(VK_NUMPAD2))							bDown = true;
		if (IsKeyJustUp(VK_NUMPAD6))							bRight = true;
		if (IsKeyJustUp(VK_NUMPAD4))							bLeft = true;
	}


	if (IsKeyJustUp(VK_F9)) {
		
		

		recordAtCamSettingsLoop();
		
	}

	if (IsKeyDown(VK_F10)) {
		recordInLoop = false;
		Sleep(300);

		isWalkingSceneRunning = false;

		ENTITY::SET_ENTITY_COLLISION(PLAYER::PLAYER_PED_ID(), TRUE, TRUE);
		ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), TRUE, FALSE);
		this->currTimeScale = 1.0f;

		spawnedPeds.clear();
		arePedsVisible = true;

		resetPlayerCam();

		closeCamCoordFiles();
		frameRateLog.close();
		pedTasksLog.close();
	}

	
	

	
	
}

void Creator::deleteCurrentCameraSetting() {
	loadCameraSettings();

	if (cameraSettings.empty() || viewCameraIndex < 0 || viewCameraIndex >= cameraSettings.size()) {
		return;
	}

	auto it = cameraSettings.erase(cameraSettings.begin() + viewCameraIndex);

	if (cameraSettings.empty()) {
		resetPlayerCam();
	}
	else {
		viewCameraIndex = (int)std::distance(it, cameraSettings.begin());
		viewCameraView(0);
	}

	saveCameraSettings();
}

void Creator::closeCamCoordFiles() {
	for (int i = 0; i < camCoordsFiles.size(); i++) {
		if (camCoordsFiles[i]->is_open()) {
			camCoordsFiles[i]->close();
		}
	}
	camCoordsFiles.clear();
}

void Creator::showFrameRate() {

	if (displayFramerate) {
		//To show the framerate
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << 1.0f / GAMEPLAY::GET_FRAME_TIME();

		set_status_text(stream.str());
	}

}

void Creator::visualizeTaskNodes() {

	if (!showTaskNodes) {
		return;
	}

	loadNetworkNodes();

	auto cmp = [](int nodeId) {
		return [nodeId](std::shared_ptr<PathNetworkNode> node) {
			return nodeId == node->id;
		};
	};

	for (auto nodeTask : nodeTasks) {
		if (nodeTask.spawn_stream_id_ == currentSpawnStream) {
			
			auto foundNode = std::find_if(networkNodes.begin(), networkNodes.end(), cmp(nodeTask.nodeId_));
			if (foundNode != networkNodes.end()) {
				DrawableBox((*foundNode)->trackPosition.x, (*foundNode)->trackPosition.y, (*foundNode)->trackPosition.z+ 0.5f, 0.3f, 0, 155, 155, 255).draw();
			}
			
		}
	}


}

void Creator::spawnSpecificThing() {
	
	

	//testEntity = VEHICLE::CREATE_VEHICLE(modelHash, pp.x + 5.0f, pp.y, pp.z, 0.0f, false, true);

	//WEAPON::GIVE_WEAPON_TO_PED(PLAYER::PLAYER_PED_ID(), GAMEPLAY::GET_HASH_KEY((char*)"weapon_battleaxe"), 11000, true, true);
	
	
	

	Vector3 pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true);
	Ped myRandomPed = PED::CREATE_RANDOM_PED(pp.x,pp.y,pp.z);
	AI::TASK_STAND_STILL(myRandomPed, 100);

	

	Entity myEntityToGoTo = OBJECT::CREATE_OBJECT(0xe12b7c7c, pp.x + 11.0f, pp.y, pp.z,
		false, true, true);
	
	AI::TASK_FOLLOW_TO_OFFSET_OF_ENTITY(myRandomPed, myEntityToGoTo, 0.5f,
		0.5f, 0.5f, 1.0f, -1,
		10000.0f, true);

	PED::SET_PED_KEEP_TASK(myRandomPed, true);

	Object seq;

	AI::OPEN_SEQUENCE_TASK(&seq);
	AI::TASK_STAND_STILL(myRandomPed, 100);

	AI::CLOSE_SEQUENCE_TASK(seq);
	AI::TASK_PERFORM_SEQUENCE(myRandomPed, seq);
	AI::CLEAR_SEQUENCE_TASK(&seq);


	
	

	

	

	
}

void Creator::rotateAndMoveWallElements() {

	if (currentWallGroup != -1 && !wallGroups.empty() && showWallGroups) {
		wallGroups[currentWallGroup].rotateAndMove();
		wallGroups[currentWallGroup].markWallElements();
	}

}


std::vector<std::vector<std::shared_ptr<PathNetworkNode>>> Creator::getAllPaths(std::shared_ptr<PathNetworkNode> start, std::shared_ptr<PathNetworkNode> end) {
	std::vector<FindPathStackEntry> stackExpandVisited;
	std::vector<std::vector<std::shared_ptr<PathNetworkNode>>> paths;

	FindPathStackEntry stackEntry;
	stackEntry.toExpand = start;
	stackExpandVisited.push_back(stackEntry);

	while (!stackExpandVisited.empty()) {
		FindPathStackEntry entry = stackExpandVisited[stackExpandVisited.size() -1 ];
		stackExpandVisited.pop_back();
		auto visited = entry.visited;
		visited.push_back(entry.toExpand);
		entry.visited = visited;
		
		if (end == entry.toExpand) {
			paths.push_back(visited);
			continue;
		}

		for (auto connectedNode : entry.toExpand->connectedNodes) {
			
			auto visitedIterFound = std::find(visited.begin(), visited.end(), connectedNode);
			if (visitedIterFound == visited.end()) {
				FindPathStackEntry newEntry;
				newEntry.toExpand = connectedNode;
				newEntry.visited = visited;
				stackExpandVisited.push_back(newEntry);
			}
		}

	}
	
	return paths;
}

std::vector<std::vector<std::shared_ptr<PathNetworkNode>>> Creator::getAllPathsFromAllStartEndCombinations() {
	loadNetworkEdges();

	std::vector<std::vector<std::shared_ptr<PathNetworkNode>>> paths;
	std::vector<std::shared_ptr<PathNetworkNode>> startEndPoints;
	//Get start/end points
	for (auto pos : networkNodes) {
		if (pos->type == PathNetworkNode::START_TYPE) {
			startEndPoints.push_back(pos);
		}
	}

	for (int i = 0; i < startEndPoints.size(); i++) {
		for (int j = i + 1; j < startEndPoints.size(); j++) {
			auto pathsOfStartEnd = getAllPaths(startEndPoints[i], startEndPoints[j]);
			paths.insert(paths.end(), pathsOfStartEnd.begin(), pathsOfStartEnd.end());
		}
	}
	
	return paths;
}

void Creator::addwPed(Ped p, Vector3 from, Vector3 to, int stop, float spd)
{
	if (nwPeds > 299)
		return;

	wPeds[nwPeds].ped = p;
	wPeds[nwPeds].from = from;
	wPeds[nwPeds].to = to;
	wPeds[nwPeds].stopTime = stop;
	wPeds[nwPeds].speed = spd;

	nwPeds++;
}

void Creator::addwPed_scenario(Ped p)
{
	if (nwPeds_scenario > 299)
		return;

	wPeds_scenario[nwPeds_scenario].ped = p;
	nwPeds_scenario++;
}

void Creator::get_2D_from_3D(Vector3 v, float *x2d, float *y2d) {

	// translation
	float x = v.x - cam_coords.x;
	float y = v.y - cam_coords.y;
	float z = v.z - cam_coords.z;

	// rotation
	float cam_x_rad = cam_rot.x * (float)M_PI / 180.0f;
	float cam_y_rad = cam_rot.y * (float)M_PI / 180.0f;
	float cam_z_rad = cam_rot.z * (float)M_PI / 180.0f;

	// cos
	float cx = cos(cam_x_rad);
	float cy = cos(cam_y_rad);
	float cz = cos(cam_z_rad);

	// sin
	float sx = sin(cam_x_rad);
	float sy = sin(cam_y_rad);
	float sz = sin(cam_z_rad);	

	Vector3 d;
	d.x = cy*(sz*y + cz*x) - sy*z;
	d.y = sx*(cy*z + sy*(sz*y + cz*x)) + cx*(cz*y - sz*x);
	d.z = cx*(cy*z + sy*(sz*y + cz*x)) - sx*(cz*y - sz*x);

	float fov_rad = fov * (float)M_PI / 180;
	float f = (SCREEN_HEIGHT / 2.0f) * cos(fov_rad / 2.0f) / sin(fov_rad / 2.0f);

	*x2d = ((d.x * (f / d.y)) / SCREEN_WIDTH + 0.5f);
	*y2d = (0.5f - (d.z * (f / d.y)) / SCREEN_HEIGHT);
}

void Creator::save_frame() {
	StretchBlt(hCaptureDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hWindowDC, 0, 0, windowWidth, windowHeight, SRCCOPY | CAPTUREBLT);
	Gdiplus::Bitmap image(hCaptureBitmap, (HPALETTE)0);
	std::wstring ws;
	StringToWString(ws, output_path);

	image.Save((ws + L"\\" + std::to_wstring(nsample) + L".jpeg").c_str(), &pngClsid, NULL);
}

Gdiplus::Status Creator::saveScreenImage(std::string path) {
	StretchBlt(hCaptureDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hWindowDC, 0, 0, windowWidth, windowHeight, SRCCOPY | CAPTUREBLT);
	Gdiplus::Bitmap image(hCaptureBitmap, (HPALETTE)0);
	std::wstring widestr = std::wstring(path.begin(), path.end());
	Gdiplus::Status status = image.Save(widestr.c_str(), &pngClsid, NULL);


	return status;
}

void Creator::setCameraMoving(Vector3 A, Vector3 B, Vector3 C, int fov) {
	
	CAM::DESTROY_ALL_CAMS(TRUE);
	this->camera = CAM::CREATE_CAM((char *)"DEFAULT_SCRIPTED_CAMERA", TRUE);
	//this->ped_with_cam = PED::CREATE_RANDOM_PED(A.x, A.y, A.z);
	this->ped_with_cam = PLAYER::PLAYER_PED_ID();
	ENTITY::SET_ENTITY_COORDS_NO_OFFSET(ped_with_cam, A.x, A.y, A.z, 0, 0, 1);
	//AI::TASK_WANDER_IN_AREA(this->ped_with_cam, coords.x, coords.y, coords.z, WANDERING_RADIUS, 1.0, 1.0);
	float z_offset = ((float)((rand() % (6)) - 2)) / 10;
	CAM::ATTACH_CAM_TO_ENTITY(camera, this->ped_with_cam, 0, 0, z_offset, TRUE);
	CAM::SET_CAM_ACTIVE(camera, TRUE);
	CAM::SET_CAM_FOV(camera, (float)fov);
	CAM::RENDER_SCRIPT_CAMS(TRUE, FALSE, 0, TRUE, TRUE);
	//CAM::SET_CAM_MOTION_BLUR_STRENGTH(camera, 10.0);

	//ENTITY::SET_ENTITY_HEALTH(ped_with_cam, 0);
	WAIT(500);
	//AI::CLEAR_PED_TASKS_IMMEDIATELY(ped_with_cam);
	//PED::RESURRECT_PED(ped_with_cam);
	//PED::REVIVE_INJURED_PED(ped_with_cam);
	//PED::SET_PED_CAN_RAGDOLL(ped_with_cam, TRUE);

	ENTITY::SET_ENTITY_COLLISION(ped_with_cam, TRUE, TRUE);
	ENTITY::SET_ENTITY_VISIBLE(ped_with_cam, FALSE, FALSE);
	ENTITY::SET_ENTITY_ALPHA(ped_with_cam, 0, FALSE);
	ENTITY::SET_ENTITY_CAN_BE_DAMAGED(ped_with_cam, FALSE);
	PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped_with_cam, TRUE);
	PED::SET_PED_COMBAT_ATTRIBUTES(ped_with_cam, 1, FALSE);

	Object seq;
	AI::OPEN_SEQUENCE_TASK(&seq);
	//AI::TASK_USE_MOBILE_PHONE_TIMED(0, max_waiting_time + 10000);
	AI::TASK_STAND_STILL(0, max_waiting_time + 10000);
	AI::TASK_GO_TO_COORD_ANY_MEANS(0, A.x, A.y, A.z, 1.0, 0, 0, 786603, 0xbf800000);
	AI::TASK_GO_TO_COORD_ANY_MEANS(0, B.x, B.y, B.z, 1.0, 0, 0, 786603, 0xbf800000);
	AI::TASK_GO_TO_COORD_ANY_MEANS(0, C.x, C.y, C.z, 1.0, 0, 0, 786603, 0xbf800000);
	AI::TASK_GO_TO_COORD_ANY_MEANS(0, B.x, B.y, B.z, 1.0, 0, 0, 786603, 0xbf800000);
	AI::TASK_GO_TO_COORD_ANY_MEANS(0, A.x, A.y, A.z, 1.0, 0, 0, 786603, 0xbf800000);
	AI::TASK_GO_TO_COORD_ANY_MEANS(0, B.x, B.y, B.z, 1.0, 0, 0, 786603, 0xbf800000);
	AI::TASK_GO_TO_COORD_ANY_MEANS(0, C.x, C.y, C.z, 1.0, 0, 0, 786603, 0xbf800000);
	AI::TASK_GO_TO_COORD_ANY_MEANS(0, B.x, B.y, B.z, 1.0, 0, 0, 786603, 0xbf800000);
	AI::TASK_GO_TO_COORD_ANY_MEANS(0, A.x, A.y, A.z, 1.0, 0, 0, 786603, 0xbf800000);
	AI::CLOSE_SEQUENCE_TASK(seq);
	AI::TASK_PERFORM_SEQUENCE(ped_with_cam, seq);
	AI::CLEAR_SEQUENCE_TASK(&seq);

	// set the cam_coords used on update() function
	this->cam_coords = CAM::GET_CAM_COORD(camera);
	this->cam_rot = CAM::GET_CAM_ROT(camera, 2);
}

void Creator::setCameraFixed(Vector3 coords, Vector3 rot, float cam_z, int fov) {

	CAM::DESTROY_ALL_CAMS(TRUE);
	this->camera = CAM::CREATE_CAM((char *)"DEFAULT_SCRIPTED_CAMERA", TRUE);
	CAM::SET_CAM_COORD(camera, coords.x, coords.y, coords.z+cam_z);
	CAM::SET_CAM_ROT(camera, rot.x, rot.y, rot.z, 2);
	CAM::SET_CAM_ACTIVE(camera, TRUE);
	CAM::SET_CAM_FOV(camera, (float)fov);
	CAM::RENDER_SCRIPT_CAMS(TRUE, FALSE, 0, TRUE, TRUE);


	// set the cam_coords used on update() function
	this->cam_coords = CAM::GET_CAM_COORD(camera);
	this->cam_rot = CAM::GET_CAM_ROT(camera, 2);
	this->fov = (int)CAM::GET_CAM_FOV(camera);
}


void Creator::takeMugshots() {
	loadPedAppearanceSet();

	auto mugshotSettings = readFloatCSV(this->output_path + "mugshotSettings.csv");

	if (mugshotSettings.empty()) {
		return;
	}

	Vector3 pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), TRUE);
	std::unordered_set<PedAppearance, PedAppearance::Hash>::iterator it = pedAppearanceSet.begin();
	TIME::SET_CLOCK_TIME(13, 30, 0);
	WAIT(2000);

	


	while (it != pedAppearanceSet.end()) {
		
		TIME::SET_CLOCK_TIME(13, 30, 0);
		auto myPedAppearance = (*it);
		std::vector<float> spawnPos{ -304.326874f, -329.255646f, 30.280500f };

		Ped ped = myPedAppearance.createPed(spawnPos[0], spawnPos[1], spawnPos[2] - 0.5f);
		ENTITY::SET_ENTITY_HEADING(ped, 0.0f);
		WAIT(100);
		
		it++;
		

		std::string mugshotsFolder = "mugshots\\";
		std::string mugshotsFolderOutput = this->output_path + mugshotsFolder;
		_mkdir(mugshotsFolderOutput.c_str());

		std::string frontPath = mugshotsFolderOutput + "front\\";
		std::string leftPath = mugshotsFolderOutput + "left\\";
		std::string rightPath = mugshotsFolderOutput + "right\\";
		std::string backPath = mugshotsFolderOutput + "back\\";
		_mkdir(frontPath.c_str());
		_mkdir(leftPath.c_str());
		_mkdir(rightPath.c_str());
		_mkdir(backPath.c_str());

		
		
		Vector3 rot;
		Vector3 coords;
		std::string imageName = "image_" + std::to_string(myPedAppearance.identityNo)  + ".jpg";
		std::string imagePath = "";
		

		//front
		coords.x = mugshotSettings[0][0];
		coords.y = mugshotSettings[0][1];
		coords.z = mugshotSettings[0][2];
		rot.x = mugshotSettings[0][3];
		rot.y = mugshotSettings[0][4];
		rot.z = mugshotSettings[0][5];
		imagePath = frontPath + imageName;
		setCamera(coords, rot);
		WAIT(0);
		Sleep(250);
		saveScreenImage(imagePath);

		//left

		coords.x = mugshotSettings[1][0];
		coords.y = mugshotSettings[1][1];
		coords.z = mugshotSettings[1][2];
		rot.x = mugshotSettings[1][3];
		rot.y = mugshotSettings[1][4];
		rot.z = mugshotSettings[1][5];
		imagePath = leftPath + imageName;
		setCamera(coords, rot);
		WAIT(0);
		Sleep(250);
		saveScreenImage(imagePath);

		//right
		coords.x = mugshotSettings[2][0];
		coords.y = mugshotSettings[2][1];
		coords.z = mugshotSettings[2][2];
		rot.x = mugshotSettings[2][3];
		rot.y = mugshotSettings[2][4];
		rot.z = mugshotSettings[2][5];

		imagePath = rightPath + imageName;
		setCamera(coords, rot);
		WAIT(0);
		Sleep(250);
		saveScreenImage(imagePath);
		
		//back

		coords.x = mugshotSettings[3][0];
		coords.y = mugshotSettings[3][1];
		coords.z = mugshotSettings[3][2];
		rot.x = mugshotSettings[3][3];
		rot.y = mugshotSettings[3][4];
		rot.z = mugshotSettings[3][5];
		imagePath = backPath + imageName;
		setCamera(coords, rot);
		WAIT(0);
		Sleep(250);
		saveScreenImage(imagePath);



		PED::DELETE_PED(&ped);
		
	

	}

}

void Creator::spawnPed(Vector3 pos, int numPed) {

	int i = 0;
	Vector3 current;
	Vector3 ped_spawned_coord[1024];
	int n = (numPed % 2 == 0) ? numPed : numPed + 1;


	for (int i = 0; i<n; i++) {
		ped_spawned[i] = PED::CREATE_RANDOM_PED(pos.x + random_float(-6, 6), pos.y + random_float(-6, 6), pos.z - 0.4f);
	}
	/**
	WAIT(2000);
	for (int i = 0; i < n; i++) {
		PED::SET_PED_COMPONENT_VARIATION(ped_spawned[i], 0, 0, 0, 2);
		PED::SET_PED_COMPONENT_VARIATION(ped_spawned[i], 1, 0, 0, 2);
		PED::SET_PED_COMPONENT_VARIATION(ped_spawned[i], 2, 0, 0, 2);
		PED::SET_PED_COMPONENT_VARIATION(ped_spawned[i], 3, 0, 0, 2);
		PED::SET_PED_COMPONENT_VARIATION(ped_spawned[i], 4, 0, 0, 2);
		PED::SET_PED_COMPONENT_VARIATION(ped_spawned[i], 5, 0, 0, 2);
		PED::SET_PED_COMPONENT_VARIATION(ped_spawned[i], 6, 0, 0, 2);
		PED::SET_PED_COMPONENT_VARIATION(ped_spawned[i], 7, 0, 0, 2);
		PED::SET_PED_COMPONENT_VARIATION(ped_spawned[i], 8, 0, 0, 2);
		PED::SET_PED_COMPONENT_VARIATION(ped_spawned[i], 9, 0, 0, 2);
		PED::SET_PED_COMPONENT_VARIATION(ped_spawned[i], 10, 0, 0, 2);
		PED::SET_PED_COMPONENT_VARIATION(ped_spawned[i], 11, 0, 0, 2);
	}
	**/
	WAIT(2000);
	for (int i = 0; i<n; i++) {
		ENTITY::SET_ENTITY_HEALTH(ped_spawned[i], 0);
	}
	WAIT(2000);
	for (int i = 0; i<n; i++) {		
		AI::CLEAR_PED_TASKS_IMMEDIATELY(ped_spawned[i]);
		PED::RESURRECT_PED(ped_spawned[i]);
		PED::REVIVE_INJURED_PED(ped_spawned[i]);
		ENTITY::SET_ENTITY_COLLISION(ped_spawned[i], TRUE, TRUE);
		PED::SET_PED_CAN_RAGDOLL(ped_spawned[i], TRUE);
	}
	WAIT(2000);
	for (int i = 0; i<n; i++) {
		ped_spawned_coord[i] = ENTITY::GET_ENTITY_COORDS(ped_spawned[i], TRUE);
		//AI::TASK_USE_NEAREST_SCENARIO_TO_COORD(ped_spawned[i], current.x, current.y, current.z, 100.0, 1000 * 60 * 3);
		//AI::TASK_WANDER_IN_AREA(ped_spawned[i], pos.x, pos.y, pos.z, WANDERING_RADIUS, 0.0, 0.0);
		AI::TASK_WANDER_STANDARD(ped_spawned[i], 0x471c4000, 0);

	}
	WAIT(2000);
	for (int i = 0; i<n; i++) {
		current = ENTITY::GET_ENTITY_COORDS(ped_spawned[i], TRUE);
		float dist = sqrt(pow(ped_spawned_coord[i].x - current.x, 2) + pow(ped_spawned_coord[i].y - current.y, 2) + pow(ped_spawned_coord[i].z - current.z, 2));
		log_file << dist << std::endl;
		if (dist < 1.0)
			PED::DELETE_PED(&ped_spawned[i]);
	}
}

Vector3 Creator::teleportPlayer(Vector3 pos){
												
	// set the heading
	float heading = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 360));

	// teleport the player to the previously selected coordinates
	PLAYER::START_PLAYER_TELEPORT(this->player, pos.x, pos.y, pos.z, heading, 0, 0, 0);
	while (PLAYER::IS_PLAYER_TELEPORT_ACTIVE()) WAIT(0);

	return pos;
}

void Creator::draw_text(char *text, float x, float y, float scale) {
	UI::SET_TEXT_FONT(0);
	UI::SET_TEXT_SCALE(scale, scale);
	UI::SET_TEXT_COLOUR(255, 255, 255, 245);
	UI::SET_TEXT_WRAP(0.0, 1.0);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_DROPSHADOW(2, 2, 0, 0, 0);
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
	UI::_SET_TEXT_ENTRY((char *)"STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(text);
	UI::_DRAW_TEXT(y, x);
}

int Creator::myreadLine(FILE *f, Vector3 *pos, int *nPeds, int *ngroup, int *currentBehaviour, float *speed, Vector3 *goFrom, Vector3 *goTo, int *task_time, int *type, 
	int *radius, int *min_lenght, int *time_between_walks, int *spawning_radius)
{
	int result = fscanf_s(f, "%d %f %f %f %d %d %f %f %f %f %f %f %f %d %d %d %d %d %d \n", nPeds, &(*pos).x, &(*pos).y, &(*pos).z,
		ngroup, currentBehaviour, speed, 
		&(*goFrom).x, &(*goFrom).y, &(*goFrom).z, &(*goTo).x, &(*goTo).y, &(*goTo).z, 
		task_time, type, radius, min_lenght, time_between_walks, spawning_radius);

	return result;
}

Cam Creator::lockCam(Vector3 pos, Vector3 rot) {
	CAM::DESTROY_ALL_CAMS(true);
	Cam lockedCam = CAM::CREATE_CAM_WITH_PARAMS((char *)"DEFAULT_SCRIPTED_CAMERA", pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, 50, true, 2);
	CAM::SET_CAM_ACTIVE(lockedCam, true);
	/*
		 RENDER_SCRIPT_CAMS(BOOL render, BOOL ease, int easeTime, BOOL p3,
																			BOOL p4) 
		ease - smooth transition between the camera's positions
		easeTime - Time in milliseconds for the transition to happen

		If you have created a script (rendering) camera, and want to go back to the
		character (gameplay) camera, call this native with render set to 0.
		Setting ease to 1 will smooth the transition.

		p3 & p4 are always 0 in the scripts

	*/


	CAM::RENDER_SCRIPT_CAMS(1, 0, 3000, 1, 0);
	return lockedCam;
}

void Creator::loadScenario(const char* fname)
{
	FILE *f = fopen(fname, "r");
	Vector3 cCoords, cRot;
	Vector3 vTP1, vTP2, vTP1_rot, vTP2_rot;
	int stop;

	fscanf_s(f, "%d ", &moving);
	if (moving == 0) 
		fscanf_s(f, "%f %f %f %d %f %f %f\n", &cCoords.x, &cCoords.y, &cCoords.z, &stop, &cRot.x, &cRot.y, &cRot.z);
	else 
		fscanf_s(f, "%f %f %f %d %f %f %f %f %f %f\n", &A.x, &A.y, &A.z, &stop, &B.x, &B.y, &B.z, &C.x, &C.y, &C.z);

	fscanf_s(f, "%f %f %f %f %f %f\n", &vTP1.x, &vTP1.y, &vTP1.z, &vTP1_rot.x, &vTP1_rot.y, &vTP1_rot.z);
	fscanf_s(f, "%f %f %f %f %f %f\n", &vTP2.x, &vTP2.y, &vTP2.z, &vTP2_rot.x, &vTP2_rot.y, &vTP2_rot.z);

	Entity e = PLAYER::PLAYER_PED_ID();

	// teleport far away in order to load game scenarios
	ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, vTP1.x, vTP1.y, vTP1.z, 0, 0, 1);
	lockCam(vTP1, vTP1_rot);
	if (DEMO)
		WAIT(3000);
	else
		WAIT(10000);

	ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, vTP2.x, vTP2.y, vTP2.z, 0, 0, 1);
	lockCam(vTP2, vTP2_rot);
	if (DEMO)
		WAIT(3000);
	else
		WAIT(10000);

	/*if (moving == 0)
		Scenario::teleportPlayer(cCoords);
	else
		Scenario::teleportPlayer(A);*/

	if (moving == 0)
		ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, cCoords.x, cCoords.y, cCoords.z, 0, 0, 1);
	else
		ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, B.x, B.y, B.z, 0, 0, 1);
		
	//ENTITY::SET_ENTITY_COORDS_NO_OFFSET(this->player, cCoords.x, cCoords.y, cCoords.z, 0, 0, 1);
	//ENTITY::SET_ENTITY_HAS_GRAVITY(this->player, true);
	//ENTITY::SET_ENTITY_COLLISION(this->player, TRUE, TRUE);

	WAIT(100);

	//this->camera = CAM::GET_RENDERING_CAM();

	Vector3 pos, goFrom, goTo;
	int npeds, ngroup, currentBehaviour, task_time, type, radius, min_lenght, time_between_walks, spawning_radius;
	float speed;
	while (myreadLine(f, &pos, &npeds, &ngroup, &currentBehaviour, &speed, 
		&goFrom, &goTo, &task_time, &type, &radius, &min_lenght, 
		&time_between_walks, &spawning_radius) >= 0) {

		if (currentBehaviour == 8) {
			spawn_peds_flow(pos, goFrom, goTo, npeds, ngroup,
				currentBehaviour, task_time, type, radius,
				min_lenght, time_between_walks, spawning_radius, speed);
		}
		else {
			spawn_peds(pos, goFrom, goTo, npeds, ngroup,
				currentBehaviour, task_time, type, radius,
				min_lenght, time_between_walks, spawning_radius, speed);
		}
			
	}
	fclose(f);

	if (moving == 0)
		Creator::setCameraFixed(cCoords, cRot, 0, fov);
	else
		Creator::setCameraMoving(A, B, C, fov);
}

void Creator::spawn_peds_flow(Vector3 pos, Vector3 goFrom, Vector3 goTo, int npeds, int ngroup, int currentBehaviour, 
	int task_time, int type, int radius, int min_lenght, int time_between_walks, int spawning_radius, float speed) {

	Ped ped[100];
	Ped ped_specular[100];
	Vector3 current;
	int i = 0;

	float rnX, rnY;

	if (currentBehaviour == 8) {
		for (int i = 0; i < npeds; i++) {
			ped[i] = PED::CREATE_RANDOM_PED(goFrom.x, goFrom.y, goFrom.z);
			WAIT(100);
		}
		if (DEMO)
			WAIT(500);
		else
			WAIT(2000);
		for (int i = 0; i<npeds; i++) {
			ENTITY::SET_ENTITY_HEALTH(ped[i], 0);
			WAIT(50);
		}
		if (DEMO)
			WAIT(500);
		else
			WAIT(2000);
		for (int i = 0; i < npeds; i++) {
			AI::CLEAR_PED_TASKS_IMMEDIATELY(ped[i]);
			PED::RESURRECT_PED(ped[i]);
			PED::REVIVE_INJURED_PED(ped[i]);
			ENTITY::SET_ENTITY_COLLISION(ped[i], TRUE, TRUE);
			PED::SET_PED_CAN_RAGDOLL(ped[i], TRUE);
			PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped[i], TRUE);
			PED::SET_PED_COMBAT_ATTRIBUTES(ped[i], 1, FALSE);
		}
		if (DEMO)
			WAIT(500);
		else
			WAIT(2000);
		for (int i = 0; i < npeds; i++) {
			ped_specular[i] = PED::CREATE_RANDOM_PED(goTo.x, goTo.y, goTo.z);
			WAIT(100);
		}
		if (DEMO)
			WAIT(500);
		else
			WAIT(2000);
		for (int i = 0; i<npeds; i++) {
			ENTITY::SET_ENTITY_HEALTH(ped_specular[i], 0);
			WAIT(50);
		}
		if (DEMO)
			WAIT(500);
		else
			WAIT(2000);
		for (int i = 0; i<npeds; i++) {
			AI::CLEAR_PED_TASKS_IMMEDIATELY(ped_specular[i]);
			PED::RESURRECT_PED(ped_specular[i]);
			PED::REVIVE_INJURED_PED(ped_specular[i]);
			ENTITY::SET_ENTITY_COLLISION(ped_specular[i], TRUE, TRUE);
			PED::SET_PED_CAN_RAGDOLL(ped_specular[i], TRUE);
			PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped_specular[i], TRUE);
			PED::SET_PED_COMBAT_ATTRIBUTES(ped_specular[i], 1, FALSE);
		}

		
	}
	else if (currentBehaviour == 0) {
		for (int i = 0; i<npeds; i++) {
			ped[i] = PED::CREATE_RANDOM_PED(pos.x, pos.y, pos.z);
			WAIT(50);
		}
		for (int i = 0; i < npeds; i++) {
			ENTITY::SET_ENTITY_HEALTH(ped[i], 0);
			WAIT(50);
		}
		WAIT(500);
		for (int i = 0; i < npeds; i++) {
			AI::CLEAR_PED_TASKS_IMMEDIATELY(ped[i]);
			PED::RESURRECT_PED(ped[i]);
			PED::REVIVE_INJURED_PED(ped[i]);
			ENTITY::SET_ENTITY_COLLISION(ped[i], TRUE, TRUE);
			PED::SET_PED_CAN_RAGDOLL(ped[i], TRUE);
			PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped[i], TRUE);
			PED::SET_PED_COMBAT_ATTRIBUTES(ped[i], 1, FALSE);
		}
	}

	// resurrecting all pedestrians and assigning them a task
	for (int i = 0; i<npeds; i++) {

		WAIT(50);

		current = ENTITY::GET_ENTITY_COORDS(ped[i], TRUE);

		Ped targetPed = ped[0];
		if (npeds > 1)
			targetPed = ped[1];

		Vector3 pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_ID(), TRUE);
		
		if (spawning_radius == -1) {
			rnX = (float)(((rand() % 81) - 40) / 10.0);
			rnY = (float)(((rand() % 81) - 40) / 10.0);
		}
		else {
			rnX = (float)((rand() % (spawning_radius * 2)) - spawning_radius);
			rnY = (float)((rand() % (spawning_radius * 2)) - spawning_radius);
		}
		float speed_rnd = (float)(10 + rand() % 4) / 10;
		addwPed(ped[i], coordsToVector(goFrom.x + rnX, goFrom.y + rnY, goFrom.z), coordsToVector(goTo.x + rnX, goTo.y + rnY, goTo.z), time_between_walks, speed_rnd);
		Object seq;
		// waiting time proportional to distance
		float atob = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(goFrom.x, goFrom.y, goFrom.z, goTo.x, goTo.y, goTo.z, 1);
		int max_time = (int)((atob / 2.5) * 1000);

		if (max_time > max_waiting_time)
			max_waiting_time = max_time;

		AI::OPEN_SEQUENCE_TASK(&seq);
		AI::TASK_USE_MOBILE_PHONE_TIMED(0, rand() % max_time);
		AI::TASK_GO_TO_COORD_ANY_MEANS(0, goFrom.x + rnX, goFrom.y + rnY, goFrom.z, speed_rnd, 0, 0, 786603, 0xbf800000);
		AI::TASK_GO_TO_COORD_ANY_MEANS(0, goTo.x + rnX, goTo.y + rnY, goTo.z, speed_rnd, 0, 0, 786603, 0xbf800000);
		AI::TASK_GO_TO_COORD_ANY_MEANS(0, goFrom.x + rnX, goFrom.y + rnY, goFrom.z, speed_rnd, 0, 0, 786603, 0xbf800000);
		AI::TASK_GO_TO_COORD_ANY_MEANS(0, goTo.x + rnX, goTo.y + rnY, goTo.z, speed_rnd, 0, 0, 786603, 0xbf800000);
		AI::TASK_GO_TO_COORD_ANY_MEANS(0, goFrom.x + rnX, goFrom.y + rnY, goFrom.z, speed_rnd, 0, 0, 786603, 0xbf800000);
		AI::TASK_GO_TO_COORD_ANY_MEANS(0, goTo.x + rnX, goTo.y + rnY, goTo.z, speed_rnd, 0, 0, 786603, 0xbf800000);
		AI::TASK_GO_TO_COORD_ANY_MEANS(0, goFrom.x + rnX, goFrom.y + rnY, goFrom.z, speed_rnd, 0, 0, 786603, 0xbf800000);
		AI::TASK_GO_TO_COORD_ANY_MEANS(0, goTo.x + rnX, goTo.y + rnY, goTo.z, speed_rnd, 0, 0, 786603, 0xbf800000);
		AI::CLOSE_SEQUENCE_TASK(seq);
		AI::TASK_PERFORM_SEQUENCE(ped[i], seq);
		AI::CLEAR_SEQUENCE_TASK(&seq);

		if (spawning_radius != -1) {
			rnX = (float)((rand() % (spawning_radius * 2)) - spawning_radius);
			rnY = (float)((rand() % (spawning_radius * 2)) - spawning_radius);
			speed_rnd = (float)(10 + rand() % 4) / 10;

			WAIT(50);

			addwPed(ped_specular[i], coordsToVector(goTo.x + rnX, goTo.y + rnY, goTo.z), coordsToVector(goFrom.x + rnX, goFrom.y + rnY, goFrom.z), time_between_walks, speed_rnd);

			Object seq2;
			AI::OPEN_SEQUENCE_TASK(&seq2);
			AI::TASK_USE_MOBILE_PHONE_TIMED(0, rand() % max_time);
			AI::TASK_GO_TO_COORD_ANY_MEANS(0, goTo.x + rnX, goTo.y + rnY, goTo.z, speed_rnd, 0, 0, 786603, 0xbf800000);
			AI::TASK_GO_TO_COORD_ANY_MEANS(0, goFrom.x + rnX, goFrom.y + rnY, goFrom.z, speed_rnd, 0, 0, 786603, 0xbf800000);
			AI::TASK_GO_TO_COORD_ANY_MEANS(0, goTo.x + rnX, goTo.y + rnY, goTo.z, speed_rnd, 0, 0, 786603, 0xbf800000);
			AI::TASK_GO_TO_COORD_ANY_MEANS(0, goFrom.x + rnX, goFrom.y + rnY, goFrom.z, speed_rnd, 0, 0, 786603, 0xbf800000);
			AI::TASK_GO_TO_COORD_ANY_MEANS(0, goTo.x + rnX, goTo.y + rnY, goTo.z, speed_rnd, 0, 0, 786603, 0xbf800000);
			AI::TASK_GO_TO_COORD_ANY_MEANS(0, goFrom.x + rnX, goFrom.y + rnY, goFrom.z, speed_rnd, 0, 0, 786603, 0xbf800000);
			AI::TASK_GO_TO_COORD_ANY_MEANS(0, goTo.x + rnX, goTo.y + rnY, goTo.z, speed_rnd, 0, 0, 786603, 0xbf800000);
			AI::TASK_GO_TO_COORD_ANY_MEANS(0, goFrom.x + rnX, goFrom.y + rnY, goFrom.z, speed_rnd, 0, 0, 786603, 0xbf800000);
			AI::CLOSE_SEQUENCE_TASK(seq2);
			AI::TASK_PERFORM_SEQUENCE(ped_specular[i], seq2);
			AI::CLEAR_SEQUENCE_TASK(&seq2);
		}
	}
}

void Creator::spawn_peds(Vector3 pos, Vector3 goFrom, Vector3 goTo, int npeds, int ngroup, int currentBehaviour,
	int task_time, int type, int radius, int min_lenght, int time_between_walks, int spawning_radius, float speed) {

	Ped ped[100];
	Vector3 current;
	int i = 0;

	int rn;

	for (int i = 0; i < npeds; i++) {
		ped[i] = PED::CREATE_RANDOM_PED(pos.x, pos.y, pos.z);
		WAIT(50);
	}
	for (int i = 0; i < npeds; i++) {
		ENTITY::SET_ENTITY_HEALTH(ped[i], 0);
		WAIT(50);
	}
	WAIT(500);
	for (int i = 0; i < npeds; i++) {
		AI::CLEAR_PED_TASKS_IMMEDIATELY(ped[i]);
		PED::RESURRECT_PED(ped[i]);
		PED::REVIVE_INJURED_PED(ped[i]);
		ENTITY::SET_ENTITY_COLLISION(ped[i], TRUE, TRUE);
		PED::SET_PED_CAN_RAGDOLL(ped[i], TRUE);
		PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped[i], TRUE);
		PED::SET_PED_COMBAT_ATTRIBUTES(ped[i], 1, FALSE);
	}
	

	// resurrecting all pedestrians and assigning them a task
	for (int i = 0; i < npeds; i++) {

		WAIT(50);

		current = ENTITY::GET_ENTITY_COORDS(ped[i], TRUE);

		Ped targetPed = ped[0];
		if (npeds > 1)
			targetPed = ped[1];

		Vector3 pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_ID(), TRUE);
		switch (currentBehaviour)
		{
		case 0:
			rn = rand() % 12 + 2;
			if (type == 0) {
				AI::TASK_USE_NEAREST_SCENARIO_TO_COORD(ped[i], current.x, current.y, current.z, 100.0, task_time);
				addwPed_scenario(ped[i]);
			}

			else if (type == 1)
				AI::TASK_START_SCENARIO_IN_PLACE(ped[i], scenarioTypes[rn], 0, true);
			else
				AI::TASK_START_SCENARIO_IN_PLACE(ped[i], scenarioTypes[type], 0, true);
			break;
		case 1:
			AI::TASK_STAND_STILL(ped[i], task_time);
			break;
		case 2:
			AI::TASK_USE_MOBILE_PHONE_TIMED(ped[i], task_time);
			//AI::TASK_START_SCENARIO_AT_POSITION(ped[i], "PROP_HUMAN_SEAT_BENCH", current.x, current.y, current.z, 0, 100000, TRUE, TRUE);
			break;
		case 3:
			AI::TASK_COWER(ped[i], task_time);
			break;
		case 4:
			AI::TASK_WANDER_IN_AREA(ped[i], current.x, current.y, current.z, (float)radius, (float)min_lenght, (float)time_between_walks);
			break;
		case 5:
			if (i > 0)
				AI::TASK_CHAT_TO_PED(ped[i], ped[0], 16, 0.0, 0.0, 0.0, 0.0, 0.0);
			else
				AI::TASK_CHAT_TO_PED(ped[i], targetPed, 16, 0.0, 0.0, 0.0, 0.0, 0.0);
			break;
		case 6:
			if (i > 0)
				AI::TASK_COMBAT_PED(ped[i], ped[0], 0, 16);
			break;
		case 7:
			AI::TASK_STAY_IN_COVER(ped[i]);
			break;

		default:
			break;
		}

	}
}

void Creator::walking_peds()
{
	for (int i = 0; i < nwPeds; i++)
	{
		if (PED::IS_PED_STOPPED(wPeds[i].ped) && !AI::GET_IS_TASK_ACTIVE(wPeds[i].ped, 426))
		{
			int currentTime = (TIME::GET_CLOCK_HOURS()) * 60 + TIME::GET_CLOCK_MINUTES();
			if (wPeds[i].timeFix == -1)
				wPeds[i].timeFix = currentTime;
			if (wPeds[i].timeFix + wPeds[i].stopTime < currentTime)
			{
				wPeds[i].goingTo = !wPeds[i].goingTo;
				wPeds[i].timeFix = -1;
				if (wPeds[i].goingTo)
					AI::TASK_GO_TO_COORD_ANY_MEANS(wPeds[i].ped, wPeds[i].to.x, wPeds[i].to.y, wPeds[i].to.z, wPeds[i].speed, 0, 0, 786603, 0xbf800000);
				else
					AI::TASK_GO_TO_COORD_ANY_MEANS(wPeds[i].ped, wPeds[i].from.x, wPeds[i].from.y, wPeds[i].from.z, wPeds[i].speed, 0, 0, 786603, 0xbf800000);
			}
		}
	}
}


void Creator::setTimeScaleViaPerCamFPS(int fpsPerCam) {
	loadCameraSettings();
	int cameraCount = (int)cameraSettings.size();
	//The equation should be fps * cameraCount == FPS / timeScale
	//If the sleep time via sleep is too high or the computations per frame are to heavy the real framerate will be lower
	this->timeScale = (float)FPS / (float)(fpsPerCam * cameraCount);
}


void Creator::startCombinedRecording() {

	if (!menuActive)
		main_menu();
	else
		bQuit = true;

	Entity e = PLAYER::PLAYER_PED_ID();

	
	//"Rockford Plaza"
	Vector3 recordingPlace;
	recordingPlace.x = -243.421036f;
	recordingPlace.y = -339.233917f;
	recordingPlace.z = 30.871639f;
	ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, recordingPlace.x, recordingPlace.y, recordingPlace.z, 0, 0, 1);

	loadWallGroups();
	
	
	TIME::SET_CLOCK_TIME(12, 0, 0);
	GAMEPLAY::CLEAR_OVERRIDE_WEATHER();
	char wheaterName[] = "EXTRASUNNY";
	GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST(wheaterName);
	GAMEPLAY::CLEAR_WEATHER_TYPE_PERSIST();
	
	WAIT(10000);
	
	

	ENTITY::SET_ENTITY_COLLISION(PLAYER::PLAYER_PED_ID(), TRUE, TRUE);
	ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), FALSE, FALSE);

	startWalkingScene();
	arePedsVisible = false;
	
	//To spawn the peds a updateNew() call is needed then they should walk some time
	int timeToWait = 200;
	while (timeToWait > 0) {
		WAIT(1);
		updateNew();
		timeToWait--;
	}
	
	
	this->currTimeScale = this->timeScale;
	recordAtCamSettingsLoop();


}