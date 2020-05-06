#ifndef SCENARIO_H
#define SCENARIO_H

#include "script.h"
#include <string>
#include "dictionary.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <Windows.h>
#include <gdiplus.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include "keyboard.h"
#include <unordered_set>
#include <algorithm>
#include <set>
#include <chrono>
#include "ParameterReader.h"
#include <random>
#include <unordered_map>
#include <sstream> // show framerate precision
#include <iomanip> // setprecision

#pragma comment (lib,"Gdiplus.lib")

const int max_number_of_peds = 1024;					// size of the pedestrians array
const int number_of_joints = 21;							// size of the joint_ID subset
const float JOINT_DELTA = 0;
const int max_wpeds = 300;






typedef struct Helper {

	static std::string Vector3ToCsv(Vector3 vector) {
		return std::to_string(vector.x) + "," + std::to_string(vector.y) + "," + std::to_string(vector.z);
	}

	static int getRandInt(int min, int max) {

		std::random_device rd; // obtain a random number from hardware
		std::mt19937 eng(rd()); // seed the generator
		std::uniform_int_distribution<> distr(min, max); // define the range
		return distr(eng);
	}

	static float getRandFloat(float min, float max) {
		float r3 = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
		return r3;
	}

	static bool getRandBool() {

		if (getRandInt(0, 1) == 1) {
			return true;
		}
		else {
			return false;
		}

	}


	template<typename T>
	static std::vector<T> slice(std::vector<T> const &v, int m, int n)
	{
		auto first = v.cbegin() + m;
		auto last = v.cbegin() + n + 1;

		std::vector<T> vec(first, last);
		return vec;
	}

	static std::pair<float, float>  lineLineIntersection(std::pair<float, float>  A, std::pair<float, float>  B, std::pair<float, float>  C, std::pair<float, float>  D)
	{
		// Line AB represented as a1x + b1y = c1 
		float a1 = B.second - A.second;
		float b1 = A.first - B.first;
		float c1 = a1 * (A.first) + b1 * (A.second);

		// Line CD represented as a2x + b2y = c2 
		float a2 = D.second - C.second;
		float b2 = C.first - D.first;
		float c2 = a2 * (C.first) + b2 * (C.second);

		float determinant = a1 * b2 - a2 * b1;

		if (determinant == 0)
		{
			// The lines are parallel. This is simplified 
			// by returning a pair of FLT_MAX 
			return std::make_pair(FLT_MAX, FLT_MAX);
		}
		else
		{
			float x = (b2*c1 - b1 * c2) / determinant;
			float y = (a1*c2 - a2 * c1) / determinant;
			return std::make_pair(x, y);
		}
	}


	static Vector3 getOrthogonalVector(Vector3 u, Vector3 v) {

		Vector3 result;

		result.x = u.y*v.z - v.y*u.z;
		result.y = -(u.x*v.z - v.x*u.z);
		result.z = u.x*v.y - v.x*u.y;

		return Helper::normVector(result);

	}

	static void drawRedLine(Vector3 start, Vector3 end) {
		GRAPHICS::DRAW_LINE(start.x, start.y, start.z, end.x, end.y, end.z, 255, 0, 0, 155);
	}

	static Vector3 rotatePointAroundPoint(Vector3 toRotate, Vector3 around, Vector3 rot) {
		Vector3 result;
		Vector3 mulWithMat;


		const float pi = (float)std::acos(-1);

		rot.x = rot.x * pi / 180.0f;
		rot.y = rot.y * pi / 180.0f;
		rot.z = rot.z * pi / 180.0f;

		mulWithMat.x = toRotate.x - around.x;
		mulWithMat.y = toRotate.y - around.y;
		mulWithMat.z = toRotate.z - around.z;

		result.x = std::cos(rot.y)*std::cos(rot.z)*mulWithMat.x
			+ (-std::cos(rot.x)*std::sin(rot.z) + std::sin(rot.x)*std::sin(rot.y)*std::cos(rot.z))*mulWithMat.y
			+ (std::sin(rot.x)*std::sin(rot.z) + std::cos(rot.x)*std::sin(rot.y)*std::sin(rot.z))*mulWithMat.z;

		result.y = std::cos(rot.y)*std::sin(rot.z)*mulWithMat.x
			+ (std::cos(rot.x)*std::cos(rot.z) + std::sin(rot.x)*std::sin(rot.y)*std::sin(rot.z))*mulWithMat.y
			+ (-std::sin(rot.x)*std::cos(rot.z) + std::cos(rot.x)*std::sin(rot.y)*std::sin(rot.z))*mulWithMat.z;

		result.z = -std::sin(rot.y)*mulWithMat.x 
			+ std::sin(rot.x)*std::cos(rot.y)*mulWithMat.y 
			+ std::cos(rot.x) * std::cos(rot.y)*mulWithMat.z;


		result.x += around.x;
		result.y += around.y;
		result.z += around.z;

		return result;

	}

	static float dotProduct(Vector3 a, Vector3 b) {

		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static Vector3 add(Vector3 a, Vector3 b) {
		Vector3 result;

		result.x = a.x + b.x;
		result.y = a.y + b.y;
		result.z = a.z + b.z;
		return result;
	}

	static Vector3 subtract(Vector3 a, Vector3 b) {
		Vector3 result;

		result.x = a.x - b.x;
		result.y = a.y - b.y;
		result.z = a.z - b.z;
		return result;
	}

	static Vector3 normVector(Vector3 v) {

		return Helper::scalarTimesVector(1.0f / Helper::vectorLength(v), v);

	}

	static float vectorLength(Vector3 v) {
		return std::sqrt(std::pow(v.x, 2) + std::pow(v.y, 2) + std::pow(v.z, 2));
	}

	static float distance(Vector3 a, Vector3 b) {
		return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2) + std::pow(a.z - b.z, 2));
	}

	static Vector3 scalarTimesVector(float scalar, Vector3 v) {
		Vector3 result;

		result.x = scalar * v.x;
		result.y = scalar * v.y;
		result.z = scalar * v.z;
		return result;
	}


	/**
	 This is needed to find tracks where all nodes appear. As you know you can define tasks that will be executed if a pedestrian reaches
	 a node. If you activate enforce task tracks the pedestrians of one stream will walk
	 through only these tracks that contain the nodes that will trigger the tasks. Therefore the intersection of the track ids has to be
	 searched for that contain these nodes of the stream.
	**/
	static std::vector<int> getIntersectedTrackIds(std::vector<std::vector<int>> trackIds) {

		if (trackIds.empty()) {
			return std::vector<int>();
		}

		std::vector<int> result;

		std::copy(trackIds[0].begin(), trackIds[0].end(), std::back_inserter(result));

		for (int i = 1; i < trackIds.size(); i++) {
			std::vector<int> newIds;
			auto currentTrackIds = trackIds[i];
			std::sort(result.begin(), result.end());
			std::sort(currentTrackIds.begin(), currentTrackIds.end());

			std::set_intersection(result.begin(), result.end(),
				currentTrackIds.begin(), currentTrackIds.end(),
				back_inserter(newIds));

			result = newIds;
		}

		return result;

	}


	static void drawBox2D(int x1, int y1, int x2, int y2, float lineThickness) {
		
		int windowWidth;
		int windowHeight;

		GRAPHICS::_GET_SCREEN_ACTIVE_RESOLUTION(&windowWidth, &windowHeight);

		float relativeX1 = (float)x1 / (float)windowWidth;
		float relativeY1 = (float)y1 / (float)windowHeight;
		float relativeX2 = (float)x2 / (float)windowWidth;
		float relativeY2 = (float)y2 / (float)windowHeight;

		
		float relativeWidth = relativeX2 - relativeX1;
		float relativeHeight = relativeY2 - relativeY1;

		GRAPHICS::DRAW_RECT(relativeX1 + relativeWidth / 2.0f, relativeY1, relativeWidth, lineThickness / (float)windowHeight, 0, 255, 0, 155);
		GRAPHICS::DRAW_RECT(relativeX1 + relativeWidth / 2.0f, relativeY2, relativeWidth, lineThickness / (float)windowHeight, 0, 255, 0, 155);
		GRAPHICS::DRAW_RECT(relativeX1, relativeY1 + relativeHeight / 2.0f, lineThickness / (float)windowWidth, relativeHeight, 0, 255, 0, 155);
		GRAPHICS::DRAW_RECT(relativeX2, relativeY1 + relativeHeight / 2.0f, lineThickness / (float)windowWidth, relativeHeight, 0, 255, 0, 155);
	}

} Helper;

typedef struct JointPosition {
	Vector3 position;
	bool occluded_ped;
	bool occluded_self;
	bool occluded_object;

} JointPosition;

typedef struct Cuboid {

	Vector3 pos_;
	Vector3 rot_;
	float width_;
	float height_;
	float depth_;


	Vector3 frontLeftUpper;
	Vector3 frontLeftLower;
	Vector3 backLeftUpper;
	Vector3 backLeftLower;
	Vector3 frontRightUpper;
	Vector3 frontRightLower;
	Vector3 backRightUpper;
	Vector3 backRightLower;

	Cuboid(Vector3 pos, Vector3 rot, float width, float height, float depth):
															pos_(pos)
															, rot_(rot) 
															, width_(width)
															, height_(height) 
															,depth_(depth) {

		frontLeftUpper.x = pos.x + depth / 2.0f;
		frontLeftUpper.y = pos.y + width / 2.0f;
		frontLeftUpper.z = pos.z + height / 2.0f;

		frontLeftLower.x = pos.x + depth / 2.0f;
		frontLeftLower.y = pos.y + width / 2.0f;
		frontLeftLower.z = pos.z - height / 2.0f;

		backLeftUpper.x = pos.x - depth / 2.0f;
		backLeftUpper.y = pos.y + width / 2.0f;
		backLeftUpper.z = pos.z + height / 2.0f;

		backLeftLower.x = pos.x - depth / 2.0f;
		backLeftLower.y = pos.y + width / 2.0f;
		backLeftLower.z = pos.z - height / 2.0f;

		frontRightUpper.x = pos.x + depth / 2.0f;
		frontRightUpper.y = pos.y - width / 2.0f;
		frontRightUpper.z = pos.z + height / 2.0f;

		frontRightLower.x = pos.x + depth / 2.0f;
		frontRightLower.y = pos.y - width / 2.0f;
		frontRightLower.z = pos.z - height / 2.0f;

		backRightUpper.x = pos.x - depth / 2.0f;
		backRightUpper.y = pos.y - width / 2.0f;
		backRightUpper.z = pos.z + height / 2.0f;

		backRightLower.x = pos.x - depth / 2.0f;
		backRightLower.y = pos.y - width / 2.0f;
		backRightLower.z = pos.z - height / 2.0f;



		frontLeftUpper = Helper::rotatePointAroundPoint(frontLeftUpper, pos, rot);
		frontLeftLower = Helper::rotatePointAroundPoint(frontLeftLower, pos, rot);
		backLeftUpper = Helper::rotatePointAroundPoint(backLeftUpper, pos, rot);
		backLeftLower = Helper::rotatePointAroundPoint(backLeftLower, pos, rot);
		frontRightUpper = Helper::rotatePointAroundPoint(frontRightUpper, pos, rot);
		frontRightLower = Helper::rotatePointAroundPoint(frontRightLower, pos, rot);
		backRightUpper = Helper::rotatePointAroundPoint(backRightUpper, pos, rot);
		backRightLower = Helper::rotatePointAroundPoint(backRightLower, pos, rot);


	}

	void draw() {
		//Left side
		Helper::drawRedLine(frontLeftUpper, frontLeftLower);
		Helper::drawRedLine(frontLeftUpper, backLeftUpper);
		Helper::drawRedLine(backLeftUpper, backLeftLower);
		Helper::drawRedLine(backLeftLower, frontLeftLower);

		//Width lines
		Helper::drawRedLine(frontLeftUpper, frontRightUpper);
		Helper::drawRedLine(frontLeftLower, frontRightLower);
		Helper::drawRedLine(backLeftLower, backRightLower);
		Helper::drawRedLine(backLeftUpper, backRightUpper);

		//Right side
		Helper::drawRedLine(frontRightUpper, frontRightLower);
		Helper::drawRedLine(frontRightUpper, backRightUpper);
		Helper::drawRedLine(backRightUpper, backRightLower);
		Helper::drawRedLine(backRightLower, frontRightLower);
	}

	bool isPointInside(Vector3 point) {
		
		float x_length = Helper::distance(frontLeftLower, backLeftLower);
		float y_length = Helper::distance(frontLeftLower, frontRightLower);
		float z_length = Helper::distance(frontLeftLower, frontLeftUpper);

		Vector3 x_local = Helper::scalarTimesVector(1.0f / x_length, Helper::subtract(backLeftLower, frontLeftLower));
		Vector3 y_local = Helper::scalarTimesVector(1.0f / y_length, Helper::subtract(frontRightLower, frontLeftLower));
		Vector3 z_local = Helper::scalarTimesVector(1.0f / z_length, Helper::subtract(frontLeftUpper, frontLeftLower));

		Vector3 center = Helper::scalarTimesVector(1.0f/2.0f, Helper::add(frontLeftLower, backRightUpper));

		Vector3 V = Helper::subtract(point, center);

		float px = std::abs(Helper::dotProduct(V, x_local));
		float py = std::abs(Helper::dotProduct(V, y_local));
		float pz = std::abs(Helper::dotProduct(V, z_local));

		bool x_cond = 2.0f * px < x_length;
		bool y_cond = 2.0f * py < y_length;
		bool z_cond = 2.0f * pz < z_length;

		return x_cond && y_cond && z_cond;

	}

} Cuboid;

typedef struct DrawableBox {
	float xPos;
	float yPos;
	float zPos;
	float size;
	int rColor;
	int gColor;
	int bColor;
	int alphaColor;

	DrawableBox(float xPos, float yPos, float zPos, float size, int rColor, int gColor, int bColor, int alphaColor) {
		this->xPos = xPos;
		this->yPos = yPos;
		this->zPos = zPos;
		this->size = size;
		this->rColor = rColor;
		this->gColor = gColor;
		this->bColor = bColor;
		this->alphaColor = alphaColor;
	}

	


	void draw() {

		//+size / 2.0f because it will be started from the edge
		GRAPHICS::DRAW_BOX(xPos - size / 2.0f, yPos - size / 2.0f, zPos - size / 2.0f,
			xPos + size - size / 2.0f, yPos + size - size / 2.0f, zPos + size - size / 2.0f,
			rColor, gColor, bColor, alphaColor);
	}


} DrawableBox;

typedef struct wPed {
	Ped ped;
	bool goingTo = true;
	Vector3 from, to;
	int stopTime;
	int timeFix = -1;
	float speed;
} wPed;

typedef struct TrackPosition {

	int nodeId; //needed to identify the tracks original node id to execute actions like waiting at that node

	float xRand;
	float yRand;

	float x;
	float y;
	float z;
	float randomRadius; //Radius around x and y where the actual position can be

	TrackPosition() {

	} 

	TrackPosition(float x, float y, float z, float randomRadius) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->randomRadius = randomRadius;
	}

	TrackPosition(int nodeId, float x, float y, float z, float randomRadius) {
		this->nodeId = nodeId;
		this->x = x;
		this->y = y;
		this->z = z;
		this->randomRadius = randomRadius;
	}

	void createRandomXY() {
		xRand = x + Helper::getRandFloat(-randomRadius, randomRadius);
		yRand = y + Helper::getRandFloat(-randomRadius, randomRadius);
	}

public:

	std::string to_csv() {
		return std::to_string(this->nodeId) + ","
			+ std::to_string(this->x) + ","
			+ std::to_string(this->y) + ","
			+ std::to_string(this->z) + ","
			+ std::to_string(randomRadius);
	}

	std::string to_coord_and_radius_csv() {
		return std::to_string(this->x) + ","
			+ std::to_string(this->y) + ","
			+ std::to_string(this->z) + ","
			+ std::to_string(randomRadius);
	}


} TrackPosition;

typedef struct NodeTask {
	int nodeId_;
	int spawn_stream_id_;
	float waitTime_;


	NodeTask(int nodeId, int spawn_stream_id, float waitTime) {
		this->nodeId_ = nodeId;
		this->spawn_stream_id_ = spawn_stream_id;
		this->waitTime_ = waitTime;
	}


	std::string to_csv() {
		return std::to_string(this->nodeId_) + "," + std::to_string(this->spawn_stream_id_) + "," + std::to_string(this->waitTime_);
	}

	bool operator==(const NodeTask& other) const
	{

		return other.nodeId_ == this->nodeId_ && other.spawn_stream_id_ == this->spawn_stream_id_;
	}

	struct Hash
	{
		std::size_t operator()(NodeTask const& node) const
		{

			return (size_t)node.nodeId_ ^ (size_t)node.spawn_stream_id_;

		}
	};

	struct EqualNodeId {
		bool operator()(const NodeTask& a, const NodeTask& b) {

			return a.nodeId_ == b.nodeId_;
		}
	};


} NodeTask;

typedef struct PedAppearance {


	struct AppearanceFeature {
		int ped_drawable_variation;
		int ped_texture_variation;
		int ped_palette_variation;
		int ped_prop_texture_index;
		int ped_prop_index;

		std::string getAsCSVLine() const {
			return std::to_string(ped_drawable_variation)
				+ "," + std::to_string(ped_texture_variation)
				+ "," + std::to_string(ped_palette_variation)
				+ "," + std::to_string(ped_prop_texture_index)
				+ "," + std::to_string(ped_prop_index);

		}



		bool operator==(const AppearanceFeature &other) const
		{

			return ped_drawable_variation == other.ped_drawable_variation
				&& ped_texture_variation == other.ped_texture_variation
				&& ped_palette_variation == other.ped_palette_variation
				&& ped_prop_texture_index == other.ped_prop_texture_index
				&& ped_prop_index == other.ped_prop_index;

		}

		struct AFHash
		{
			std::size_t operator()(AppearanceFeature const& af) const
			{

				std::size_t ped_drawable_variation_hash = std::hash<int>{}(af.ped_drawable_variation);
				std::size_t ped_texture_variation_hash = std::hash<int>{}(af.ped_texture_variation);
				std::size_t ped_palette_variation_hash = std::hash<int>{}(af.ped_palette_variation);
				std::size_t ped_prop_texture_index_hash = std::hash<int>{}(af.ped_prop_texture_index);
				std::size_t ped_prop_index_hash = std::hash<int>{}(af.ped_prop_index);
				return ped_drawable_variation_hash
					^ (ped_texture_variation_hash << 1)
					^ (ped_palette_variation_hash << 1)
					^ (ped_prop_texture_index_hash << 1)
					^ (ped_prop_index_hash << 1);

			}
		};

	};



	bool isSpawnable = true;
	const int MAX_COMPONENTS = 12;
	const int MAX_PROPS = 3;
	Hash modelHash;
	int identityNo;

	std::vector<AppearanceFeature> appearanceFeatures;

	PedAppearance& operator=(const PedAppearance& other)
	{
		if (this != &other) { // protect against invalid self-assignment

			modelHash = other.modelHash;
			identityNo = other.identityNo;

			appearanceFeatures = other.appearanceFeatures;

		}
		// by convention, always return *this
		return *this;
	}


	PedAppearance(Ped ped, int identityNo) {
		this->identityNo = identityNo;
		modelHash = ENTITY::GET_ENTITY_MODEL(ped);

		//Actually there are only 3 prop components. It will be -1 for the other.
		for (int componentId = 0; componentId < MAX_COMPONENTS; componentId++) {
			AppearanceFeature appearanceFeature;
			appearanceFeature.ped_drawable_variation = PED::GET_PED_DRAWABLE_VARIATION(ped, componentId);
			appearanceFeature.ped_texture_variation = PED::GET_PED_TEXTURE_VARIATION(ped, componentId);
			appearanceFeature.ped_palette_variation = PED::GET_PED_PALETTE_VARIATION(ped, componentId);
			appearanceFeature.ped_prop_texture_index = PED::GET_PED_PROP_TEXTURE_INDEX(ped, componentId);
			appearanceFeature.ped_prop_index = PED::GET_PED_PROP_INDEX(ped, componentId);
			appearanceFeatures.push_back(appearanceFeature);
		}
	}

	PedAppearance() {

	}

	PedAppearance(std::vector<std::vector<std::string>> pedAppearance) {

		identityNo = std::stoi(pedAppearance[0][0]);
		modelHash = std::stoul(pedAppearance[0][1]);

		if (pedAppearance[0].size() > 7) {
			this->isSpawnable = (std::stoi(pedAppearance[0][7]) == 1) ? true : false;
		}

		for (int componentId = 0; componentId < MAX_COMPONENTS; componentId++) {
			AppearanceFeature appearanceFeature;
			appearanceFeature.ped_drawable_variation = std::stoi(pedAppearance[componentId][2]);
			appearanceFeature.ped_texture_variation = std::stoi(pedAppearance[componentId][3]);
			appearanceFeature.ped_palette_variation = std::stoi(pedAppearance[componentId][4]);
			appearanceFeature.ped_prop_texture_index = std::stoi(pedAppearance[componentId][5]);
			appearanceFeature.ped_prop_index = std::stoi(pedAppearance[componentId][6]);
			appearanceFeatures.push_back(appearanceFeature);
		}
	}


	struct Hash
	{
		std::size_t operator()(PedAppearance const& pa) const
		{
			size_t hashResult = (size_t)pa.modelHash;
			for (int componentId = 0; componentId < pa.appearanceFeatures.size(); componentId++) {

				PedAppearance::AppearanceFeature appearanceFeature = pa.appearanceFeatures[componentId];

				std::size_t appearanceFeatureHash = PedAppearance::AppearanceFeature::AFHash{}(appearanceFeature);

				hashResult ^= appearanceFeatureHash << componentId;
			}

			return hashResult;
		}
	};





public:
	void appendToOfstream(std::ofstream &ofstream) {

		for (int componentId = 0; componentId < appearanceFeatures.size(); componentId++) {
			ofstream << std::to_string(identityNo) + "," + std::to_string(modelHash) + "," + appearanceFeatures[componentId].getAsCSVLine() + "," + std::to_string(isSpawnable ?  1 : 0) << "\n";
		}
	}

	std::string toString() const {
		std::string result = "";
		for (int componentId = 0; componentId < appearanceFeatures.size(); componentId++) {
			result += std::to_string(modelHash) + "," + appearanceFeatures[componentId].getAsCSVLine() + "\n";
		}

		return result;
	}

	bool operator==(const PedAppearance &other) const
	{

		if (modelHash != other.modelHash) {
			return false;
		}

		for (int componentId = 0; componentId < appearanceFeatures.size(); componentId++) {
			AppearanceFeature otherAppearanceFeature = other.appearanceFeatures[componentId];
			AppearanceFeature appearanceFeature = appearanceFeatures[componentId];
			if (!(otherAppearanceFeature == appearanceFeature)) {
				return false;
			}
		}

		return true;
	}

	Ped createPed(float x, float y, float z) {


		STREAMING::REQUEST_MODEL(modelHash);
		while (!STREAMING::HAS_MODEL_LOADED(modelHash)) {
			WAIT(1);
		}

		//z-0.3f because the recorded position is too high and the ped will fall down.
		Ped ped = PED::CREATE_PED(1, modelHash, x, y, z - 0.5f, 0.0f, false, true);

		for (int componentId = 0; componentId < MAX_COMPONENTS; componentId++) {


			AppearanceFeature af = appearanceFeatures[componentId];
			
			PED::SET_PED_PROP_INDEX(ped, componentId, af.ped_prop_index, af.ped_prop_texture_index, true);
			

			PED::SET_PED_COMPONENT_VARIATION(ped,
				componentId,
				af.ped_drawable_variation,
				af.ped_texture_variation,
				af.ped_palette_variation);

		}
		ENTITY::SET_ENTITY_AS_MISSION_ENTITY(ped, true, true);
		ENTITY::SET_ENTITY_CAN_BE_DAMAGED(ped, FALSE);
		return ped;


	}

	Ped createPed(std::vector<TrackPosition> track) {

		return createPed(track[0].x + Helper::getRandFloat(-track[0].randomRadius, track[0].randomRadius),
			track[0].y + Helper::getRandFloat(-track[0].randomRadius, track[0].randomRadius),
			track[0].z + Helper::getRandFloat(-track[0].randomRadius, track[0].randomRadius));
	}


} PedAppearance;

typedef struct VectorHash {
	size_t operator()(const std::vector<int>& v) const {
		std::hash<int> hasher;
		size_t seed = 0;
		for (int i : v) {
			seed ^= hasher(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
} VectorHash;

typedef struct PedSpawned {
	
	Ped ped;
	Object entityToFollow;
	int spawnId;
	int groupMemberNo;

	bool isWaiting = false;
	int waitStartTime;
	int timeToWait;

	bool isPedDeleted = false;
	float xTarget;
	float yTarget;
	float zTarget;

	float walkingSpeed;

	float deleteDistance;
	int currentPosition;

	std::vector<TrackPosition> trackToFollow;

	std::shared_ptr<std::vector<NodeTask>> nodeTasks = nullptr;
	PedAppearance pedAppearance;

	std::unordered_set<std::vector<int>,VectorHash> wrongTaskCombinations;

	PedSpawned() {

	}

	PedSpawned(Ped ped, std::shared_ptr<std::vector<NodeTask>> nodeTasks, PedAppearance pedAppearance, int spawnId) {
		this->ped = ped;
		this->nodeTasks = nodeTasks;
		this->pedAppearance = pedAppearance;
		this->spawnId = spawnId;
		std::vector<int> oneWrongTaskCombi { 35, 38, 120, 221 };
		wrongTaskCombinations.insert(oneWrongTaskCombi);

	}

	//Will be called in a loop for all peds in every updateNew Cycle
	void runActions() {
		deleteIfTargetPositionReached();
		checkVehiclePedIntersections();
		checkIfTaskRunStop();
		goToNewPositionIfPositionHasBeenReached();
	}

	void checkIfTaskRunStop() {

		if (nodeTasks == nullptr) {
			return;
		}

		if (trackToFollow.empty()) {
			return;
		}

		if (currentPosition >= trackToFollow.size()) {
			return;
		}



		Vector3 pedCoords = ENTITY::GET_ENTITY_COORDS(ped, TRUE);
		float distanceTargetPed = std::sqrt(std::pow((trackToFollow[currentPosition].xRand - pedCoords.x), 2)
			+ std::pow((trackToFollow[currentPosition].yRand - pedCoords.y), 2)
			+ std::pow((trackToFollow[currentPosition].z - pedCoords.z), 2));

		if (distanceTargetPed < (3.5f) && !isWaiting) {

			for (int i = 0; i < nodeTasks->size(); i++) {

				if ((*nodeTasks)[i].nodeId_ == trackToFollow[currentPosition].nodeId) {
					timeToWait = (int)(*nodeTasks)[i].waitTime_;
					isWaiting = true;
					waitStartTime = GAMEPLAY::GET_GAME_TIMER();
					 
				}

			}
			
		}

		if (isWaiting) {
			if ((int)GAMEPLAY::GET_GAME_TIMER() - waitStartTime >= timeToWait) {
				isWaiting = false;
			}
		}



	}

	void checkVehiclePedIntersections() {
		const int maxVehicles = 200;

		struct NearbyEnts {
			int size;//still 32 bit integer
			int64_t entities[maxVehicles];
		};

		NearbyEnts nearbyVehicles;
		int foundVehicles = PED::GET_PED_NEARBY_VEHICLES(ped, (int*)&nearbyVehicles);
		nearbyVehicles.size = maxVehicles;
		int64_t* vehicles = nearbyVehicles.entities;
		
		Vector3 pedPos1 = ENTITY::GET_ENTITY_COORDS(ped, true);


		for (int i = 0; i < foundVehicles; i++) {
			
			Vector3 vehiclePos1 = ENTITY::GET_ENTITY_COORDS((Entity)vehicles[i], true);
			Vector3 vehicleRot = ENTITY::GET_ENTITY_ROTATION((Entity)vehicles[i], 2);



			Vector3 minDim;
			Vector3 maxDim;
			GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL((Entity)vehicles[i]), &minDim, &maxDim);

			
			if (!VEHICLE::IS_VEHICLE_SEAT_FREE((Entity)vehicles[i], -1)) {
				
				Cuboid safetyBox(vehiclePos1, vehicleRot, maxDim.y - minDim.y + 5.0f, maxDim.z - minDim.z + 2.0f,maxDim.x - minDim.x + 2.0f);

				
				//safetyBox.draw();
				if (safetyBox.isPointInside(pedPos1)) {
					//DrawableBox(pedPos1.x, pedPos1.y, pedPos1.z, 0.5f, 255, 0, 0, 155).draw();
					VEHICLE::SET_VEHICLE_FORWARD_SPEED((Entity)vehicles[i], 0.0f);
				}
			}

		}
		
		
	}

	void deleteIfTargetPositionReached() {
		Vector3 pedCoords = ENTITY::GET_ENTITY_COORDS(ped, TRUE);
		float distanceTargetPed = std::sqrt(std::pow((xTarget - pedCoords.x), 2) 
			+ std::pow((yTarget - pedCoords.y), 2) 
			+ std::pow((zTarget - pedCoords.z), 2));

		if (distanceTargetPed < (3.8f)) {
			
			AI::CLEAR_PED_TASKS_IMMEDIATELY(ped);
			PED::DELETE_PED(&ped);
			OBJECT::DELETE_OBJECT(&entityToFollow);
			isPedDeleted = true;
		}

	}



	/**

	Maximum of 8 Points. Otherwise the peds will stop. People can get stuck at obstacles. Maybe a go back strategie can be a solution.
	**/
	void followPointRoute(std::vector<TrackPosition> points) {
		AI::TASK_FLUSH_ROUTE();

	

		for (int i = 0; i < points.size(); i++) {
			AI::TASK_EXTEND_ROUTE(points[i].x, points[i].y, points[i].z);


		}
		AI::TASK_FOLLOW_POINT_ROUTE(ped, 5.0f, 0);
	}

	
	void followNavmesh(std::vector<TrackPosition> track) {
		if (track.empty()) {
			return;
		}
		AI::CLEAR_PED_TASKS_IMMEDIATELY(ped);
		AI::CLEAR_PED_TASKS(ped);
		AI::CLEAR_PED_SECONDARY_TASK(ped);
		
		

		TrackPosition startPosition = track[0];
		TrackPosition endPosition = track[track.size() - 1];

		float distance = std::sqrt(std::pow(startPosition.x - endPosition.x, 2) +
			std::pow(startPosition.y - endPosition.y, 2) +
			std::pow(startPosition.z - endPosition.z, 2));

		PATHFIND::ADD_NAVMESH_REQUIRED_REGION(startPosition.x, startPosition.y, distance * 1.5f);

		while (!PATHFIND::ARE_ALL_NAVMESH_REGIONS_LOADED()) {
			WAIT(0);
			Sleep(1);
		}


		AI::TASK_FOLLOW_NAV_MESH_TO_COORD(ped, endPosition.x, endPosition.y, endPosition.z,
			5.0f, -1,  1.5f, true,
			0.0f);

		Any distRemaining = 0;
		Any isRouteCalculated = false;

		
		while (isRouteCalculated <= 0) {
			AI::GET_NAVMESH_ROUTE_DISTANCE_REMAINING(ped, &distRemaining, &isRouteCalculated);
			WAIT(0);
			Sleep(1);
		}

	}

	
	void goToNewPositionIfPositionHasBeenReached() {

		
		
		if (trackToFollow.empty()) {
			return;
		}

		if (currentPosition >= trackToFollow.size()) {
			return;
		}

		
		
		Vector3 pedCoords = ENTITY::GET_ENTITY_COORDS(ped, TRUE);
		float distanceTargetPed = std::sqrt(std::pow((trackToFollow[currentPosition].xRand - pedCoords.x), 2)
			+ std::pow((trackToFollow[currentPosition].yRand - pedCoords.y), 2)
			+ std::pow((trackToFollow[currentPosition].z - pedCoords.z), 2));

		if (distanceTargetPed < (3.5f) && !isWaiting) {

			goToPosition(trackToFollow[currentPosition + 1]);
			currentPosition++;
		}
		
	}

	
	void goToPosition(TrackPosition trackPos) {
		
		ENTITY::SET_ENTITY_COORDS(entityToFollow, trackPos.xRand, trackPos.yRand, trackPos.z, 1, 0, 0, 1);
		

	}

	void resetFollowEntity() {

		AI::CLEAR_PED_TASKS_IMMEDIATELY(ped);

		AI::TASK_FOLLOW_TO_OFFSET_OF_ENTITY(ped, entityToFollow, 0.5f,
			0.5f, 0.5f, walkingSpeed, -1,
			10000.0f, true);
	}

	//Seems to set the same task combination and therefore the people will be locked at one place
	void checkForWrongTasks() {

		const unsigned int maxTaskNumber = 528;
		std::vector<int> runningTasks;
		for (int taskNo = 0; taskNo < maxTaskNumber; taskNo++) {
			if (AI::GET_IS_TASK_ACTIVE(ped, taskNo)) {
				runningTasks.push_back(taskNo);
			}
		}

		if (wrongTaskCombinations.find(runningTasks) != wrongTaskCombinations.end()) {
			resetFollowEntity();
		}

	}

	

	void followEntity(std::vector<TrackPosition> track, float walkingSpeed, int currentGroupSize) {
		
		if (track.size() < 2) {
			return;
		}

		if (!trackToFollow.empty()) {
			return;
		}

		groupMemberNo = currentGroupSize;
		currentPosition = 0;
		trackToFollow = track;
		

		for (int i = 0; i < trackToFollow.size(); i++) {
			trackToFollow[i].createRandomXY();
		}

		xTarget = trackToFollow[trackToFollow.size() - 1].xRand;
		yTarget = trackToFollow[trackToFollow.size() - 1].yRand;
		zTarget = trackToFollow[trackToFollow.size() - 1].z;

		deleteDistance = trackToFollow[trackToFollow.size() - 1].randomRadius;

		AI::CLEAR_PED_SECONDARY_TASK(ped);
		AI::CLEAR_PED_TASKS_IMMEDIATELY(ped);
		PED::RESURRECT_PED(ped);
		PED::REVIVE_INJURED_PED(ped);
		ENTITY::SET_ENTITY_COLLISION(ped, TRUE, TRUE);
		PED::SET_PED_CAN_RAGDOLL(ped, TRUE);
		PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped, TRUE);
		PED::SET_PED_COMBAT_ATTRIBUTES(ped, 1, FALSE);
		

		//0xe12b7c7c is just a hash for the name of a paper box called hei_prop_heist_box 
		//found on a website then hashed with jooat php md5hashing.net
		entityToFollow = OBJECT::CREATE_OBJECT(0xe12b7c7c, trackToFollow[1].xRand, trackToFollow[1].yRand, trackToFollow[1].z,
			false, true, true);
		ENTITY::SET_ENTITY_VISIBLE(entityToFollow,false, false);
		ENTITY::SET_ENTITY_COLLISION(entityToFollow, false, true);

		
		AI::TASK_FOLLOW_TO_OFFSET_OF_ENTITY(ped, entityToFollow, 0.5f,
			0.5f, 0.5f, walkingSpeed, -1,
			10000.0f, true);
		this->walkingSpeed = walkingSpeed;
		PED::SET_PED_KEEP_TASK(ped, true);
		
	}


} PedSpawned;







typedef struct PathNetworkNode {
	

public:
	static const int START_TYPE = 0;
	static const int NO_START_TYPE = 1;
	
	
	std::vector<std::shared_ptr<PathNetworkNode>> connectedNodes;
	TrackPosition trackPosition;

	int type;
	int id;


	PathNetworkNode() {

	}

	PathNetworkNode(float x, float y, float z, float randomRadius): trackPosition(x,y,z, randomRadius) {
		
	}

	std::string to_csv() {
		return std::to_string(id) + "," + std::to_string(type) + "," + trackPosition.to_coord_and_radius_csv();
	}

} PathNetworkNode;

typedef struct PathNetworkEdge {
	std::shared_ptr<PathNetworkNode> first_;
	std::shared_ptr<PathNetworkNode> second_;

	struct Hash
	{
		std::size_t operator()(PathNetworkEdge const& pathNetworkEdge) const
		{
			return pathNetworkEdge.first_->id ^ pathNetworkEdge.second_->id;
		}
	};

	bool operator==(const PathNetworkEdge &other) const
	{


		return ((other.first_->id == this->first_->id) &&  (other.second_->id == this->second_->id))
			|| ((other.first_->id == this->second_->id) && (other.second_->id == this->first_->id));
	}


	PathNetworkEdge() {

	};



	PathNetworkEdge(std::shared_ptr<PathNetworkNode> first, std::shared_ptr<PathNetworkNode> second): first_(first), second_(second) {

	}

	std::string to_csv() {

		if (first_ == nullptr || second_ == nullptr) {
			return "Null,Null";
		}

		return std::to_string(first_->id)+ "," + std::to_string(second_->id);
	}
	
} PathNetworkEdge;


typedef struct FindPathStackEntry {
	std::shared_ptr<PathNetworkNode> toExpand;
	std::vector<std::shared_ptr<PathNetworkNode>> visited;

	FindPathStackEntry() {}
} FindPathStackEntry;




typedef struct ScenarioGroup {
	
	int group_size;
	int spawned_ped_count = 0;
	int spawn_ped_no; //The number at which this group should be spawned
	float speed;
	int track;
	bool reverseTrack;
	std::shared_ptr<std::vector<NodeTask>> nodeTasks = nullptr;
	
	std::string to_string() {

		return "group_size:" + std::to_string(group_size)  + "\n"
			+ "spawned_ped_count:" +  std::to_string(spawned_ped_count) + "\n"
			+ "spawn_ped_no:" + std::to_string(spawn_ped_no) + "\n"
			+ "speed:" + std::to_string(speed) + "\n"
			+ "trackId:" + std::to_string(track) + "\n"
			+ "reverseTrack:" + std::to_string(reverseTrack) + "\n";
	}

	ScenarioGroup() {}

} ScenarioGroup;

typedef struct ScenarioEntry {
	int pedTracksSize;
	
	std::shared_ptr<std::vector<NodeTask>> nodeTasks = nullptr;
	std::unordered_map<int, std::shared_ptr<std::vector<int>>> * nodeToTrack;
	std::vector<int> intersectedTracks; //tracks that contain all nodes that have a task

	int spawn_stream_id;
	int max_spawned_world;
	int spawn_at_ped_no_start;
	int spawn_at_ped_no_end;
	int group_count;
	int group_size_start;
	int group_size_end;
	float ped_speed_start;
	float ped_speed_end;
	int track_no_start;
	int track_no_end;
	
	int currentNotFullGroup = 0;

	


	std::vector<std::shared_ptr<ScenarioGroup>> groups;

	ScenarioEntry() { }

	void init() {

		loadIntersectedTracks();

		for (int i = 0; i < group_count; i++) {
			auto newGroup = std::make_shared<ScenarioGroup>();
			newGroup->group_size = Helper::getRandInt(group_size_start, group_size_end);
			newGroup->spawn_ped_no = Helper::getRandInt(spawn_at_ped_no_start, spawn_at_ped_no_end);
			newGroup->speed = Helper::getRandFloat(ped_speed_start, ped_speed_end);
			int trackRandStart = track_no_start;
			int trackRandEnd = track_no_end;

			if (track_no_start < 0) {
				trackRandStart = 0;
			}

			if (track_no_start >= pedTracksSize) {
				trackRandStart = pedTracksSize - 1;
			}

			if (track_no_end >= pedTracksSize) {
				trackRandEnd = pedTracksSize - 1;
			}

			if (track_no_end < 0) {
				trackRandEnd = 0;
			}

			if (!intersectedTracks.empty()) {
				newGroup->track = intersectedTracks[(size_t)Helper::getRandInt(0, (int)(intersectedTracks.size()-1))];
			}
			else {
				newGroup->track = Helper::getRandInt(trackRandStart, trackRandEnd);
			}
			
			newGroup->nodeTasks = this->nodeTasks;
			

			newGroup->reverseTrack = Helper::getRandBool();
			


			groups.push_back(newGroup);
		}
		

		
	}

	void loadIntersectedTracks() {
		
		
		if (nodeTasks == nullptr) {
			return;
		}
		std::vector<std::vector<int>> allTracks;
		for (auto task : *nodeTasks) {
			auto tracks = nodeToTrack->find(task.nodeId_);
			if (tracks != nodeToTrack->end()) {
				std::shared_ptr<std::vector<int>> tracksContainingNode = (*tracks).second;

				allTracks.push_back(*tracksContainingNode);
			}
		}

		intersectedTracks = Helper::getIntersectedTrackIds(allTracks);
	
	}

	std::shared_ptr<ScenarioGroup> getFreeGroup(int current_ped_no) {
		
		
		while(currentNotFullGroup < groups.size()) {


			if (groups[currentNotFullGroup]->group_size > groups[currentNotFullGroup]->spawned_ped_count
				&& current_ped_no >= groups[currentNotFullGroup]->spawn_ped_no) {
				return groups[currentNotFullGroup];
			}

			currentNotFullGroup++;
		}

		return nullptr;
	}


} ScenarioEntry;

typedef struct PedScenario {


	std::unordered_set<NodeTask,NodeTask::Hash> * nodeTasks;
	std::unordered_map<int, std::shared_ptr<std::vector<NodeTask>>> spawnStreamIdToNodeTask;
	std::unordered_map<int, std::shared_ptr<std::vector<int>>> * nodeToTrack;
	std::vector<ScenarioEntry> scenarioEntries;

	PedScenario() {}


	void loadSpawnStreamIdToNodeTask() {
		for (auto task : *nodeTasks) {

			auto foundStreamToNodeTask = spawnStreamIdToNodeTask.find(task.spawn_stream_id_);

			if (foundStreamToNodeTask == spawnStreamIdToNodeTask.end()) {

				auto newNodeTaskVectorPair = std::make_pair(task.spawn_stream_id_, std::make_shared<std::vector<NodeTask>>());
				newNodeTaskVectorPair.second->push_back(task);
				spawnStreamIdToNodeTask.insert(newNodeTaskVectorPair);

			}
			else {

				(*foundStreamToNodeTask).second->push_back(task);
			}


		}
	}

	void loadScenario(std::vector<std::vector<std::string>> scenarioStrings, int pedTracksSize) {

		loadSpawnStreamIdToNodeTask();


		//start with one because of the header
		for (int i = 1; i < scenarioStrings.size(); i++) {
			ScenarioEntry newEntry;
			newEntry.pedTracksSize = pedTracksSize;
			newEntry.spawn_stream_id = std::stoi(scenarioStrings[i][0]);
			newEntry.max_spawned_world = std::stoi(scenarioStrings[i][1]);
			newEntry.spawn_at_ped_no_start = std::stoi(scenarioStrings[i][2]);
			newEntry.spawn_at_ped_no_end = std::stoi(scenarioStrings[i][3]);
			newEntry.group_count = std::stoi(scenarioStrings[i][4]);
			newEntry.group_size_start = std::stoi(scenarioStrings[i][5]);
			newEntry.group_size_end = std::stoi(scenarioStrings[i][6]);
			newEntry.ped_speed_start = std::stof(scenarioStrings[i][7]);
			newEntry.ped_speed_end = std::stof(scenarioStrings[i][8]);
			newEntry.track_no_start = std::stoi(scenarioStrings[i][9]);
			newEntry.track_no_end = std::stoi(scenarioStrings[i][10]);

			auto possibleNodeTasks = spawnStreamIdToNodeTask.find(newEntry.spawn_stream_id);
			if (spawnStreamIdToNodeTask.end() != possibleNodeTasks) {
				newEntry.nodeTasks = (*possibleNodeTasks).second;
			}

			newEntry.nodeToTrack = nodeToTrack;
			
			newEntry.init();
			scenarioEntries.push_back(newEntry);

			//std::ofstream scenarioLoadLog("scenarioLoadLog.txt",std::ofstream::app);
			//scenarioLoadLog << "scenario loaded "  << std::to_string(i) << "\n";
			//scenarioLoadLog.close();
		}

	}

	std::shared_ptr<ScenarioGroup> getFreeGroup(int spawnedPedCount, int currentPedIndex) {
		

		for (int i = 0; i < scenarioEntries.size(); i++) {
			if (scenarioEntries[i].max_spawned_world > spawnedPedCount) {
				auto possibleFreeGroup = scenarioEntries[i].getFreeGroup(currentPedIndex);
				if (possibleFreeGroup != nullptr) {
					return possibleFreeGroup;
				}
			}
			
		}

		return nullptr;
	}


} Scenario;


typedef struct CameraSetting {

	Vector3 position;
	Vector3 rotation;

	CameraSetting() {

	}

	CameraSetting(Vector3 position, Vector3 rotation) {
		this->position = position;
		this->rotation = rotation;
	}


	CameraSetting(std::vector<std::string> settingsStrings) {
		position.x = std::stof(settingsStrings[0]);
		position.y = std::stof(settingsStrings[1]);
		position.z = std::stof(settingsStrings[2]);
		rotation.x = std::stof(settingsStrings[3]);
		rotation.y = std::stof(settingsStrings[4]);
		rotation.z = std::stof(settingsStrings[5]);

	}

	std::string getCSV() {
		return std::to_string(position.x)
			+ "," + std::to_string(position.y)
			+ "," + std::to_string(position.z)
			+ "," + std::to_string(rotation.x)
			+ "," + std::to_string(rotation.y)
			+ "," + std::to_string(rotation.z);
	}

} CameraSetting;




typedef struct BoundingBox {
	float x_min;
	float y_min;

	float x_max;
	float y_max;

	BoundingBox() {}

	BoundingBox(float x_min, float y_min, float x_max, float y_max) {
		this->x_min = x_min;
		this->y_min = y_min;
		this->x_max = x_max;
		this->y_max = y_max;
	}


} BoundingBox;

typedef struct WallElement {
	float xOffset;
	float zOffset;

	Object element;


} WallElement;

typedef struct WallGroup {
	std::vector<WallElement> wallElements;
	int rows;
	int columns;
	bool isMovable = true;
	float playerWallDistance = -2.0f;
	Vector3 cameraPosition;
	Vector3 playerPosition;
	Vector3 cameraRotation;
	static const Hash modelHash = -1694943621;

	void setVisibility(bool visible) {
		for (int i = 0; i < wallElements.size(); i++) {
			
			
			ENTITY::SET_ENTITY_VISIBLE(wallElements[i].element, visible, false);
			
		}
	}

	std::string to_csv() {
		return std::to_string(rows)
			+ "," + std::to_string(columns)
			+ "," + std::to_string(playerWallDistance)
			+ "," + Helper::Vector3ToCsv(cameraPosition)
			+ "," + Helper::Vector3ToCsv(playerPosition)
			+ "," + Helper::Vector3ToCsv(cameraRotation);
	}

	WallGroup(std::vector<std::string> wallGroupStrings) {
		this->rows = std::stoi(wallGroupStrings[0]);
		this->columns = std::stoi(wallGroupStrings[1]);
		this->playerWallDistance = std::stof(wallGroupStrings[2]);
		
		cameraPosition.x = std::stof(wallGroupStrings[3]);
		cameraPosition.y = std::stof(wallGroupStrings[4]);
		cameraPosition.z = std::stof(wallGroupStrings[5]);

		playerPosition.x = std::stof(wallGroupStrings[6]);
		playerPosition.y = std::stof(wallGroupStrings[7]);
		playerPosition.z = std::stof(wallGroupStrings[8]);

		cameraRotation.x = std::stof(wallGroupStrings[9]);
		cameraRotation.y = std::stof(wallGroupStrings[10]);
		cameraRotation.z = std::stof(wallGroupStrings[11]);

		createWallElements(rows, columns, cameraPosition, playerPosition, cameraRotation);
	}

	void markWallElements() {
		
		for (int i = 0; i < wallElements.size(); i++) {
			Vector3 wallElementPosition = ENTITY::GET_ENTITY_COORDS(wallElements[i].element, false);
			DrawableBox(wallElementPosition.x, wallElementPosition.y, wallElementPosition.z, 0.4f, 0, 255, 0, 155).draw();
		}

	}

	void createWallElements(int rows, int columns, Vector3 cameraPosition, Vector3 playerPosition, Vector3 cameraRotation) {
		

		if (rows <= 0 || columns <= 0) {
			return;
		}

		this->cameraPosition = cameraPosition;
		this->playerPosition = playerPosition;
		this->cameraRotation = cameraRotation;

		deleteWallElements();
		Vector3 pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true);
		this->rows = rows;
		this->columns = columns;


		

		STREAMING::REQUEST_MODEL(modelHash);
		while (!STREAMING::HAS_MODEL_LOADED(modelHash)) {
			WAIT(1);

		}

		for (int row = 0; row < rows; row++) {
			for (int column = 0; column < columns; column++) {
				Object plane = OBJECT::CREATE_OBJECT(modelHash, pp.x, pp.y, pp.z + 1.0f, false, true, false);

				ENTITY::SET_ENTITY_VISIBLE(plane, true, true);

				WallElement wallElement2;
				Vector3 minDim;
				Vector3 maxDim;
				GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL((Entity)plane), &minDim, &maxDim);


				wallElement2.element = plane;
				float zWidth = maxDim.z - minDim.z - 0.1f;
				float xWidth = maxDim.x - minDim.x - 0.1f;

				wallElement2.xOffset = xWidth * column;
				wallElement2.zOffset = zWidth * row;
				Vector3 wallPoint = calculateWallPoint(cameraPosition, playerPosition, cameraRotation, wallElement2.xOffset, wallElement2.zOffset);
				
				ENTITY::SET_ENTITY_COORDS(plane, wallPoint.x, wallPoint.y, wallPoint.z, 1, 0, 0, 1);
				ENTITY::SET_ENTITY_ROTATION(plane, cameraRotation.x, cameraRotation.y, cameraRotation.z, 2, true);

				wallElements.push_back(wallElement2);
			}
		}
	}

	void createWallElements(int rows, int columns) {
		Vector3 pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true);
		Vector3 camPos = CAM::GET_GAMEPLAY_CAM_COORD();
		Vector3 camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);
		createWallElements(rows, columns, camPos, pp, camRot);
	}

	void deleteWallElements() {
		
		while (wallElements.size() > 0) {
			ENTITY::DELETE_ENTITY(&(*wallElements.begin()).element);
			wallElements.erase(wallElements.begin());
			
		}
	}

	Vector3 calculateWallPoint(Vector3 camPos, Vector3 playerPos, Vector3 camRot, float xOffset, float zOffset) {
		Vector3 camToPlayer = Helper::subtract(cameraPosition, playerPosition);

		//To Put it in front of the player a vector through the camera position and the player position is used
		Vector3 wallPoint = Helper::add(Helper::scalarTimesVector(playerWallDistance, camToPlayer), playerPosition);
		//Little offset to get it up
		wallPoint.z += 1.0f;

		Vector3 zAxis;
		zAxis.x = 0;
		zAxis.y = 0;
		zAxis.z = 1;
		Vector3 leftRightAxis = Helper::getOrthogonalVector(camToPlayer, zAxis);

		wallPoint = Helper::add(Helper::scalarTimesVector(xOffset, leftRightAxis), wallPoint);
		wallPoint.z += zOffset;
		return wallPoint;
		
	}

	void rotateAndMove() {

		if (!isMovable) {
			return;
		}
		this->cameraPosition = CAM::GET_GAMEPLAY_CAM_COORD();
		Entity player = PLAYER::PLAYER_PED_ID();
		this->playerPosition = ENTITY::GET_ENTITY_COORDS(player, true);
		this->cameraRotation = CAM::GET_GAMEPLAY_CAM_ROT(2);
		for (int i = 0; i < wallElements.size(); i++) {
			Vector3 wallPoint = calculateWallPoint(cameraPosition, playerPosition, cameraRotation, wallElements[i].xOffset, wallElements[i].zOffset);
			ENTITY::SET_ENTITY_ROTATION(wallElements[i].element, cameraRotation.x, cameraRotation.y, cameraRotation.z, 2, true);
			ENTITY::SET_ENTITY_COORDS_NO_OFFSET(wallElements[i].element, wallPoint.x, wallPoint.y, wallPoint.z, 0, 0, 1);
		}
	}

	

	WallGroup(int rows, int columns) {
		createWallElements(rows, columns);
	}

	

} WallGroup;

class Creator
{
public:
	Creator(std::string output_path, const char* file_scenario, int max_samples, int is_night);
	Creator(std::string _output_path, std::string config_file);
	int update();
	void listenKeyStrokes();
	void updateNew();
	~Creator();



private:

	int SCREEN_WIDTH;
	int SCREEN_HEIGHT;

	const int head_component_id = 0;
	const int beard_component_id = 1;
	const int hair_component_id = 2;
	const int torso_1_component_id = 3;
	const int legs_component_id = 4;
	const int hands_component_id = 5;
	const int foot_component_id = 6;
	const int eyes_component_id = 7;
	const int accessories_1_component_id = 8;
	const int accessories_2_component_id = 9;
	const int decals_component_id = 10;
	const int torso_2_component_id = 11;

	const std::string START_TYPE_STR = "START";
	const std::string NO_START_TYPE_STR = "NO_START";

	const std::string screen_width_param_name_ = "screen_width";
	const std::string screen_height_param_name_ = "screen_height";
	const std::string wait_ms_after_set_camera_param_name_ = "wait_ms_after_set_camera";
	const std::string time_scale_param_name_ = "time_scale";
	const std::string fps_per_cam_param_name_ = "fps_per_cam";
	const std::string is_debug_param_name_ = "is_debug";


	std::string output_path;
	int sequence_index;
	Player player;
	Ped playerPed;
	std::string line;								// string use the safe the fram data-line
	std::string log;
	Cam camera;										// camera
	bool showingGameplayCam = true;
	Vector3 cam_coords;								// coordinates of the camera
	Vector3 cam_rot;
	Vector3 wanderer;
	Ped entity_cam;
	Vector3 ped_spawn_pos;
	bool SHOW_JOINT_RECT;							// bool used to switch the rectangle drawing around the joint
	Ped ped_spawned[max_number_of_peds];
	int n_peds;
	int max_samples;
	std::vector<const char*> bad_scenarios; // 36
	int ped_with_cam;
	const char* file_scenario;
	wPed wPeds[max_wpeds];
	wPed wPeds_scenario[max_wpeds];
	int nwPeds = 0;
	int nwPeds_scenario = 0;
	bool recordInLoop = false;
	int moving;
	Vector3 A, B, C;
	std::unordered_set<PedAppearance, PedAppearance::Hash> pedAppearanceSet;
	std::vector<PedAppearance> pedAppearanceVector; 

	std::vector<std::shared_ptr<PathNetworkNode>> networkNodes;
	std::vector<CameraSetting> cameraSettings;

	std::unordered_set<PathNetworkEdge, PathNetworkEdge::Hash> networkEdges;
	PedScenario pedScenario;
	std::unordered_set<NodeTask,NodeTask::Hash> nodeTasks;

	std::unordered_map<int, std::shared_ptr<std::vector<int>>> nodesToTracks;

	bool networkMenuActive;
	int currentNodeType = 1;
	const int nodeTypeCount = 2;
	bool showNetwork = false;
	bool showTracks = false;
	bool showTaskNodes = false;
	int currentMaxNodeId = -1;
	const int incrementByFactor = 10;
	float incrementByScenarioMenu = 1000.0f;

	bool shouldDrawPed2dBox = false;

	int windowWidth;
	int windowHeight;
	int secondsBeforeSaveImages;
	int captureFreq;
	int joint_int_codes[number_of_joints];
	int fov;
	int max_waiting_time = 0;
	int is_night;
	float timeScale = 0.8f;
	int fpsPerCam;
	float currTimeScale = 1.0f;
	
	HWND hWnd;
	HDC hWindowDC;
	HDC hCaptureDC;
	HBITMAP hCaptureBitmap;
	bool displayFramerate;

	float recordingPeriod;
	std::clock_t lastRecordingTime;
	int nsample;
	std::vector<std::shared_ptr<std::ofstream>> camCoordsFiles;
	std::ofstream frameRateLog;
	std::ofstream pedTasksLog;
	std::ofstream log_file;									// file used to log things

	int waitTimeAfterSetCamera;
	bool is_debug;
	int defaultPedsInWorld;
	int imageCountPerCam = 0;

	CLSID pngClsid;

	std::string pedAppearancesPath;
	std::string walkingPositionsPath;
	std::string cameraSettingsPath;
	std::string networkNodesPath;
	std::string scenarioPath;
	std::string networkEdgesPath;
	std::string nodeTasksPath;
	std::string wallGroupsPath;

	ParameterReader<int> int_params_;
	ParameterReader<float> float_params_;
	ParameterReader<std::string > string_params_;
	ParameterReader<bool> bool_params_;

	void get_2D_from_3D(Vector3 v, float *x, float *y);
	void save_frame();																		// function used to capture frames internally, then private
	void setCameraFixed(Vector3 coords, Vector3 rot, float cam_z, int fov);
	void setCameraMoving(Vector3 A, Vector3 B, Vector3 C, int fov);							// function used to set the camera stuff
	void spawnPed(Vector3 spawnAreaCenter, int numPed);										// function used to spawn pedestrians at the beginning of the scenario
	Vector3 teleportPlayer(Vector3 pos);													// function used to teleport the player to a random position returned by the function
	void draw_text(char *text, float x, float y, float scale);
	void addwPed(Ped p, Vector3 from, Vector3 to, int stop, float spd);
	void spawn_peds_flow(Vector3 pos, Vector3 goFrom, Vector3 goTo, int npeds, int ngroup, int currentBehaviour, int task_time, int type, int radius,
		int min_lenght, int time_between_walks, int spawning_radius, float speed);
	void spawn_peds(Vector3 pos, Vector3 goFrom, Vector3 goTo, int npeds, int ngroup, int currentBehaviour,
		int task_time, int type, int radius, int min_lenght, int time_between_walks, int spawning_radius, float speed);
	void loadScenario(const char* fname);
	void walking_peds();
	int myreadLine(FILE *f, Vector3 *pos, int *nPeds, int *ngroup, int *currentBehaviour, float *speed, Vector3 *goFrom, Vector3 *goTo, int *task_time,
		int *type, int *radius, int *min_lenght, int *time_between_walks, int *spawning_radius);

	void addwPed_scenario(Ped p);
	Cam lockCam(Vector3 pos, Vector3 rot);
	void addCurrentCameraView();
	void appendLineToFile(std::string fname, std::string line);
	std::vector<std::vector<float>> readFloatCSV(std::string filePath);
	Gdiplus::Status saveScreenImage(std::string filename);
	void recordAtCamSettingsLoop();
	void logPedestrians(int imageCountPerCam, int frameCount, int camId, std::shared_ptr<std::ofstream>);
	void setCamera(Vector3 coords, Vector3 rots);
	void appendCSVLinesToFile(std::shared_ptr<std::ofstream>, std::vector<std::vector<std::string>> stringVector);
	void logPedAppearance();
	std::vector<std::vector<std::string>> readStringCSV(std::string filePath);
	
	void loadPedAppearanceSet();
	void savePedAppearanceSet();
	void deletePedAppearanceSetNonHumans();
	void runWalkingScenes();
	void viewCameraView(int increment);
	void resetMenuCommands();
	void main_menu();
	void createTracksMenu();
	void draw_menu_line(std::string caption, float lineWidth, float lineHeight, float lineTop, float lineLeft, float textLeft, bool active, bool title, bool rescaleText = true);
	void draw_rect(float A_0, float A_1, float A_2, float A_3, int A_4, int A_5, int A_6, int A_7);
	void visualizeTrack();
	void drawCircleInXY(float radius, float x, float y, float z, int steps, int r, int g, int b, int alpha);
	int getNearestTrackPosition();
	float getTrackPosNearestRandomRadius();
	void incTrackPosNearestRandomRadius(float value);
	void savePedTracks();
	void loadPedTracks();
	void moveNearestTrackPositionToPlayer();
	void runSpawnedPedActions();
	void setNativePedsInvisible();
	void takeMugshots();
	void spawnSpecificThing();
	void recordCurrentCamera();
	void registerParams();
	void createNetworkMenu();
	std::shared_ptr<PathNetworkNode> findNearestNode(Vector3 pos);
	void visualizeNetwork();
	std::tuple<int, int> findTwoNearestNodeIndices(Vector3 pos);

	void incNodeNearestRandomRadius(float value);
	float getNodePosNearestRandomRadius();
	void saveNetworkNodes();
	void loadNetworkNodes();
	void saveNetworkEdges();
	void loadNetworkEdges();
	void loadPedScenario();
	std::vector<std::vector<std::shared_ptr<PathNetworkNode>>> getAllPaths(std::shared_ptr<PathNetworkNode> start, std::shared_ptr<PathNetworkNode> end);
	std::vector<std::vector<std::shared_ptr<PathNetworkNode>>> getAllPathsFromAllStartEndCombinations();

	void recordAllCamsOnce();
	void loadCameraSettings();
	void createScenarioMenu();

	void loadNodeTasks();
	void saveNodeTasks();
	void generateNodeToTrackMappings();
	void logPedSpawnProgress(int numberOfSpawnedPeds, int maxSpawnablePeds, std::shared_ptr<ScenarioGroup> group);
	void showFrameRate();
	void visualizeTaskNodes();
	void closeCamCoordFiles();
	void setTimeScaleViaPerCamFPS(int fps);
	void resetPlayerCam();
	void logFramerate();
	void drawPedBox3D();
	std::vector<JointPosition> getPedJointPoints(Ped ped);
	void drawPed2dBoxViaJoints();
	void saveCameraSettings();
	void deleteCurrentCameraSetting();
	void startCombinedRecording();
	BoundingBox getPaddedBoundingBox(BoundingBox occludedBox, BoundingBox nonOccludedBox);

	void logTasksPed(Ped ped);
	void rotateAndMoveWallElements();
	void createWallMenu();
	void saveWallGroups();
	void loadWallGroups();
	void deleteWallGroups();
	void startWalkingScene();
};


	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

#endif // !SCENARIO_H
