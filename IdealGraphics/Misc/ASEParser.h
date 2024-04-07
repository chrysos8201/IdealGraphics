#pragma once
#include "Core/Core.h"
#include "RenderTest/D3D12ThirdParty.h"
struct RotationAxisAndAngle
{
	RotationAxisAndAngle() : frame(0), axis(Vector3()), angle(0), quat(Quaternion()) { }
	RotationAxisAndAngle(int frameP, Vector3 rotAxisP, float angleP, Quaternion quatP) : frame(frameP), axis(rotAxisP), angle(angleP), quat(quatP) {}
	int frame;
	Vector3 axis;
	float angle;
	Quaternion quat;
};
struct BoneWeightInfo
{
	BYTE boneIdx;
	float boneWeight;
};
struct ASEData
{
	

	struct Vertex
	{
		Vector3 pos;
		Vector3 normal;
		Vector2 uv;
	};
	struct Vertex2
	{
		int posIdx;
		int normal;	// 이것도 나중에 인덱스로 바꿔줄 것. 일단 나중에
		Vector2 uv; // 이것도
	};
	struct Scene
	{
		std::string fileName;
		int firstFrame;
		int lastFrame;
		int frameSpeed;
		int ticksPerFrame;
		Vector3 backgroundStatic;
		Vector3 ambientStatic;

		int frameCount;
	};

	struct Mesh
	{
		int timeValue;
		int numVertex;
		int numFaces;

		int numTVertex;
		int numTFaces;

		std::vector<Vector3> vertexList;


		// bone List
		std::vector<std::string> boneList;	//일단 string으로 무지성 검사

		// bone weight info
		//std::vector<std::vector<BoneWeightInfo>> boneWeightInfo;	// vertex와 같은 idx를 공유

		// bone weight info // vertex와 같은 idx를 공유 // 내부에 어떤 본의 가중치가 있는지 최소 1개를 가지고 있음
		std::vector<std::vector<BoneWeightInfo>> boneWeightInfo;

		// 일단 세개만 받는다.
		std::vector<Vector3> faceList;
		std::vector<Vector3> normals;

		std::vector<Vector2> tVertexList;
		//std::vector<unsigned int> 
	};

	struct NodeTm
	{
		std::string nodeName;
		std::string parentNodeName;

		Vector3 inheritPos;
		Vector3 inheritRot;
		Vector3 inheritScl;

		//XMMATRIX tm;
		Matrix tm;
		Vector3 pos;
		Vector3 rotAxis;
		float rotAngle;
		Vector3 scale;
		Vector3 scaleAxis;
		float scaleAxisAng;
	};

	struct AnimIdxVector
	{
		struct AnimIdxVector() : frame(0), vector3(Vector3(0.f, 0.f, 0.f)) { }
		struct AnimIdxVector(const AnimIdxVector& other) : frame(other.frame), vector3(other.vector3) { }
		int frame;
		Vector3 vector3;
	};

	struct AnimationInfo
	{
		//std::vector<SimpleMath::Vector3> controlPosTrack;
		std::vector<AnimIdxVector> controlPosTrack;
		std::vector<RotationAxisAndAngle> controlRotTrack;
		//std::vector<SimpleMath::Vector3> constrolSclTrack;
		std::vector<AnimIdxVector> constrolSclTrack;
	};

	struct Object
	{
		NodeTm node;
		AnimationInfo animInfo;
	};
	struct HelperObject : Object
	{
	};

	struct ShapeObject : Object
	{
	};

	struct GeomObject : Object
	{
		Mesh mesh;
		std::vector<ASEData::Vertex> vertices;
		std::vector<ASEData::Vertex2> vertices2;
		std::vector<unsigned int> indices;

	};




	//std::vector<ASEData::Vertex> vertices;
	//std::vector<unsigned int> indices;
	Scene scene;
	std::vector<ShapeObject*> shapeObjects;
	std::vector<HelperObject*> helperObjects;
	std::vector<GeomObject*> geomObjects;
	//Mesh mesh;

	// geomobject안에 mesh가 안에 있어야 한다.
};

enum ASE_TOKEN
{
	_3DSMAX_ASCIIEXPORT = 0,
	COMMENT,


	SCENE,
	SCENE_FILENAME,
	SCENE_FIRSTFRAME,
	SCENE_LASTFRAME,
	SCENE_FRAMESPEED,
	SCENE_TICKSPERFRAME,
	SCENE_BACKGROUND_STATIC,
	SCENE_AMBIENT_STATIC,

	// material list
	MATERIAL_LIST,

	// HELPEROBJEcT
	HELPEROBJECT,

	BOUNDINGBOX_MIN,

	// ShapeObject
	SHAPEOBJECT,
	SHAPE_LINECOUNT,

	// TMANimation
	TM_ANIMATION,

	// GEOMOBJECT
	GEOMOBJECT,

	NODE_NAME,
	NODE_PARENT,

	// NODE TM
	NODE_TM,
	NODE_NAME_2,
	INHERIT_POS,
	INHERIT_ROT,
	INHERIT_SCL,
	TM_ROW0,
	TM_ROW1,
	TM_ROW2,
	TM_ROW3,
	TM_POS,
	TM_ROTAXIS,
	TM_ROTANGLE,
	TM_SCALE,
	TM_SCALEAXIS,
	TM_SCALEAXISANG,

	// MESH
	MESH,
	TIMEVALUE,
	MESH_NUMVERTEX,
	MESH_NUMFACES,
	MESH_VERTEX_LIST,
	MESH_VERTEX,
	MESH_FACE_LIST,
	MESH_FACE,
	MESH_NORMALS,
	MESH_FACENORMAL,
	MESH_VERTEXNORMAL,

	MESH_NUMTVERTEX,
	MESH_TVERTLIST,
	MESH_TVERT,

	MESH_NUMTVFACES,
	MESH_TFACELIST,
	MESH_TFACE,

	PROP_RECVSHADOW,

	// Bone
	MESH_NUMBONE,
	BONE_LIST,
	BONE,
	BONE_NAME,

	MESH_WVERTEXS,
	MESH_WEIGHT,
	BONE_BLENGING_WEIGHT,


	ASE_TOKEN_SIZE,
};

enum class ObjectType : int
{
	None,
	HELPEROBJECT,
	GEOMOBJECT,
	SHAPEOBJECT
};

class ASEParser
{
public:
	static ASEData* LoadASE2(std::string path);
	static std::shared_ptr<ASEData> LoadASE3(std::string path);
	static void SkipWhitespace(std::string& line);

	// token을 가져온다. 현재 받아온 라인 정보에서 토큰을 빼서 넘긴다.
	// 아무것도 없을경우 빈 문자열 반환!
	// 그리고 현재 받아온 라인 정보에서 토큰은 삭제한다.
	static std::string GetToken(std::string& line);
	static int GetInt(std::string& line);
	static float GetFloat(std::string& line);
	static std::string GetString(std::string& line);
	static Vector3 GetVectorXZY(std::string& line);

private:
};

