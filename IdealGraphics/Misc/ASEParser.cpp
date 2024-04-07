//#include "pch.h"
#include "Misc/ASEParser.h"
#include <fstream>
using namespace std;
using namespace DirectX;
std::vector<std::string> ASE_TOKEN_STRING_DATA
{
	"*3DSMAX_ASCIIEXPORT",
	"*COMMENT",
	"*SCENE",
	"*SCENE_FILENAME",
	"*SCENE_FIRSTFRAME",
	"*SCENE_LASTFRAME",
	"*SCENE_FRAMESPEED",
	"*SCENE_TICKSPERFRAME",
	"*SCENE_BACKGROUND_STATIC",
	"*SCENE_AMBIENT_STATIC",

	"*MATERIAL_LIST",

	"*HELPEROBJECT",
	"*BOUNDINGBOX_MIN",

	"*SHAPEOBJECT",
	"*SHAPE_LINECOUNT",

	"*TM_ANIMATION",

	"*GEOMOBJECT",
	"*NODE_NAME",	// 두 개네?
	"*NODE_PARENT",

	"*NODE_TM",
	"*NODE_NAME",
	"*INHERIT_POS",
	"*INHERIT_ROT",
	"*INHERIT_SCL",
	"*TM_ROW0",
	"*TM_ROW1",
	"*TM_ROW2",
	"*TM_ROW3",
	"*TM_POS",
	"*TM_ROTAXIS",
	"*TM_ROTANGLE",
	"*TM_SCALE",
	"*TM_SCALEAXIS",
	"*TM_SCALEAXISANG",
	"*MESH",
	"*TIMEVALUE",
	"*MESH_NUMVERTEX",
	"*MESH_NUMFACES",
	"*MESH_VERTEX_LIST",
	"*MESH_VERTEX",
	"*MESH_FACE_LIST",
	"*MESH_FACE",
	"*MESH_NORMALS",
	"*MESH_FACENORMAL",
	"*MESH_VERTEXNORMAL",

	"*MESH_NUMTVERTEX",
	"*MESH_TVERTLIST",
	"*MESH_TVERT",
	"*MESH_NUMTVFACES",
	"*MESH_TFACELIST",
	"*MESH_TFACE",


	"*PROP_RECVSHADOW",
	// BONE
	"*MESH_NUMBONE",
	"*BONE_LIST",
	"*BONE",
	"*BONE_NAME",

	"*MESH_WVERTEXS",
	"*MESH_WEIGHT",
	"*BONE_BLENGING_WEIGHT"
};
ASEData* ASEParser::LoadASE2(std::string path)
{
	ifstream ifs;
	ifs.open(path);

	// 새로운 ASE data를 만들어 반환
	ASEData* aseData = new ASEData();

	// 현재 만들어지고 있는 GeomObject를 가리킴
	int currentGeomObjectIdx = 0;

	// 현재 만들어지고 있는 HelperObject를 가리킴
	int currentHelperObjectIdx = 0;

	// 현재 만들어지고 있는 ShapeObject를 가리킴
	int currentShapeObjectIdx = 0;

	// 컬링 순서를 변경하기 위한 변수
	int currentFaceIdx = 0;
	int faceNextIdx = 0;

	// 현재 받아온 normal의 인덱스를 가르킴
	int currentNormalIdx = 0;

	// Animation
	int animFrameCount = 0;
	// int prevAnimIdx = 0;
	// int nextAnimIdx = -1;

	// 애니메이션의 쿼터니언을 앞에 인덱스와 곱해주기 위해 일단 가지고 있는다.
	int animQuaternionIdx = -1;

	// 처음부터 index를 관리하면서 string data와 비교


	//vertices, indices index
	int currentIdx = 0;

	/// Bone Info
	int currentBoneListIdx = 0;
	int currentMeshIdxForBoneWeight = 0;

	// Bone Weight Info Idx
	int currentBoneWeightCount = 0;

	// 현재 노드 타입
	ObjectType currentObjectType = ObjectType::None;
	ASEData::NodeTm* currentNode = nullptr;
	ASEData::Object* currentObject = nullptr;

	// 일단 무한 루프
	while (true)
	{
		std::string currentLine;
		getline(ifs, currentLine);

		if (currentLine == "")
			break;

		string token = ".";
		int DataIdx = 0;
		while (token != "")
		{
			token = GetToken(currentLine);
			if (token == "") break;
			if (token == "}") { token = ""; break; }
			for (DataIdx = 0; DataIdx < ASE_TOKEN_SIZE; DataIdx++)
			{
				if (token == ASE_TOKEN_STRING_DATA[DataIdx])
				{
					break;
				}
			}

			// 여기서 token에 따라 처리

			switch (DataIdx)
			{
			case _3DSMAX_ASCIIEXPORT:
			{
				// 이상한 숫자?
				int a = GetInt(currentLine);
				int b = 3;
			}
			break;
			case COMMENT:
			{
				// 주석
				GetString(currentLine);
			}
			break;
			case SCENE:
			{
				// "{" 를 지운다.
				GetToken(currentLine);
			}
			break;
			case SCENE_FILENAME:
			{
				aseData->scene.fileName = GetString(currentLine);
			}
			break;
			case SCENE_FIRSTFRAME:
			{
				aseData->scene.firstFrame = GetInt(currentLine);
			}
			break;
			case SCENE_LASTFRAME:
			{
				aseData->scene.lastFrame = GetInt(currentLine);
			}
			break;
			case SCENE_FRAMESPEED:
			{
				aseData->scene.frameSpeed = GetInt(currentLine);
			}
			break;
			case SCENE_TICKSPERFRAME:
			{
				aseData->scene.ticksPerFrame = GetInt(currentLine);

				animFrameCount = (aseData->scene.lastFrame + 1) * aseData->scene.ticksPerFrame;
				aseData->scene.frameCount = animFrameCount;
			}
			break;

			case HELPEROBJECT:
			{
				ASEData::HelperObject* helperObj = new ASEData::HelperObject();
				aseData->helperObjects.push_back(helperObj);

				Matrix I = DirectX::XMMatrixIdentity();
				XMStoreFloat4x4(&aseData->helperObjects[currentHelperObjectIdx]->node.tm, I);

				currentObjectType = ObjectType::HELPEROBJECT;
				currentNode = &aseData->helperObjects[currentHelperObjectIdx]->node;

				// 현재 오브젝트가 무엇인지
				currentObject = helperObj;
			}
			break;

			case BOUNDINGBOX_MIN:
			{
				currentHelperObjectIdx++;
			}
			break;

			case SHAPEOBJECT:
			{

				ASEData::ShapeObject* shapeObj = new ASEData::ShapeObject();
				aseData->shapeObjects.push_back(shapeObj);

				XMMATRIX I = XMMatrixIdentity();
				XMStoreFloat4x4(&aseData->shapeObjects[currentShapeObjectIdx]->node.tm, I);

				currentObjectType = ObjectType::SHAPEOBJECT;
				currentNode = &aseData->shapeObjects[currentShapeObjectIdx]->node;
				currentObject = shapeObj;
				/*int blockCount = 0;
				GetToken(currentLine);
				blockCount++;

				while (blockCount != 0)
				{
					std::string currentLine2;
					getline(ifs, currentLine2);

					string token2 = ".";
					while (token2 != "")
					{
						token2 = GetToken(currentLine2);
						if (token2 == "{")
							blockCount++;
						else if (token2 == "}")
							blockCount--;
					}
				}*/
			}
			break;
			case SHAPE_LINECOUNT:
			{
				currentShapeObjectIdx++;
			}
			break;
			case TM_ANIMATION:
			{
				int blockCount = 0;
				GetToken(currentLine);
				blockCount++;

				while (blockCount != 0)
				{
					std::string currentLine2;
					getline(ifs, currentLine2);

					string token2 = ".";
					while (token2 != "")
					{
						token2 = GetToken(currentLine2);
						if (token2 == "{")
							blockCount++;
						else if (token2 == "}")
							blockCount--;
						else if (token2 == "*CONTROL_POS_TRACK")
						{
							//currentObject->animInfo.controlPosTrack.resize(animFrameCount);
						}
						else if (token2 == "*CONTROL_POS_SAMPLE")
						{
							int frame = GetInt(currentLine2);
							SimpleMath::Vector3 pos = GetVectorXZY(currentLine2);
							//currentObject->animInfo.controlPosTrack[animIdx] = pos;

							ASEData::AnimIdxVector currentAnimInfo;
							currentAnimInfo.frame = frame;
							currentAnimInfo.vector3 = pos;

							currentObject->animInfo.controlPosTrack.push_back(currentAnimInfo);
						}
						else if (token2 == "*CONTROL_ROT_TRACK")
						{
							//currentObject->animInfo.controlRotTrack.resize(animFrameCount);
						}
						else if (token2 == "*CONTROL_ROT_SAMPLE")
						{
							int frame = GetInt(currentLine2);
							SimpleMath::Vector3 rotAxis = GetVectorXZY(currentLine2);
							float angle = GetFloat(currentLine2);

							RotationAxisAndAngle currentAnimRotInfo;
							currentAnimRotInfo.frame = frame;
							currentAnimRotInfo.axis = rotAxis;
							currentAnimRotInfo.angle = angle;
							if (currentObject->animInfo.controlRotTrack.size() == 0)
							{
								currentAnimRotInfo.quat = SimpleMath::Quaternion(rotAxis, angle);
							}
							else
							{
								// currentAnimRotInfo.quat = currentObject->animInfo.controlRotTrack.back().quat * SimpleMath::Quaternion(rotAxis, angle);
								currentAnimRotInfo.quat = currentObject->animInfo.controlRotTrack.back().quat * SimpleMath::Quaternion(rotAxis, angle);
							}
							currentObject->animInfo.controlRotTrack.push_back(currentAnimRotInfo);
							//currentObject->animInfo.controlRotTrack[animIdx] = { rotAxis,angle };
						}

						// scale은 없으니까 나중에 언젠가 반드시
					}
				}
			}
			break;
			case SCENE_BACKGROUND_STATIC:
			{
				aseData->scene.backgroundStatic.x = GetFloat(currentLine);
				aseData->scene.backgroundStatic.y = GetFloat(currentLine);
				aseData->scene.backgroundStatic.z = GetFloat(currentLine);
			}
			break;
			case SCENE_AMBIENT_STATIC:
			{
				aseData->scene.ambientStatic.x = GetFloat(currentLine);
				aseData->scene.ambientStatic.y = GetFloat(currentLine);
				aseData->scene.ambientStatic.z = GetFloat(currentLine);
			}
			break;

			case MATERIAL_LIST:
			{

			}
			break;

			case GEOMOBJECT:
			{
				currentObjectType = ObjectType::GEOMOBJECT;

				//"{" 삭제
				GetToken(currentLine);
				ASEData::GeomObject* currentGeomObject = new ASEData::GeomObject();
				aseData->geomObjects.push_back(currentGeomObject);

				currentFaceIdx = 0;
				faceNextIdx = 0;
				currentIdx = 0;
				XMMATRIX I = XMMatrixIdentity();
				XMStoreFloat4x4(&aseData->geomObjects[currentGeomObjectIdx]->node.tm, I);

				currentNode = &aseData->geomObjects[currentGeomObjectIdx]->node;
				currentObject = currentGeomObject;
			}
			break;
			case NODE_NAME:
			{
				//aseData->geomObject.nodeName = GetString(currentLine);
				//aseData->geomObjects[currentGeomObjectIdx]->node.nodeName = GetString(currentLine);
				currentNode->nodeName = GetString(currentLine);
				currentNode->tm = SimpleMath::Matrix::Identity;
			}
			break;
			case NODE_PARENT:
			{
				//aseData->geomObjects[currentGeomObjectIdx]->node.parentNodeName = GetString(currentLine);
				currentNode->parentNodeName = GetString(currentLine);
			}
			break;
			case NODE_TM:
			{
				// "{" 삭제
				GetToken(currentLine);
			}
			break;
			case NODE_NAME_2:
			{
				// 임시 용 
				//aseData->geomObject.nodeName = GetString(currentLine);
				//aseData->geomObjects[currentGeomObjectIdx]->node.nodeName = GetString(currentLine);
				currentNode->nodeName = GetString(currentLine);
			}
			break;
			case INHERIT_POS:
			{
				// 					aseData->geomObjects[currentGeomObjectIdx]->node.inheritPos.x = GetInt(currentLine);
				// 					aseData->geomObjects[currentGeomObjectIdx]->node.inheritPos.z = GetInt(currentLine);
				// 					aseData->geomObjects[currentGeomObjectIdx]->node.inheritPos.y = GetInt(currentLine);
				currentNode->inheritPos.x = GetInt(currentLine);
				currentNode->inheritPos.z = GetInt(currentLine);
				currentNode->inheritPos.y = GetInt(currentLine);
			}
			break;
			case INHERIT_ROT:
			{
				currentNode->inheritRot.x = GetInt(currentLine);
				currentNode->inheritRot.z = GetInt(currentLine);
				currentNode->inheritRot.y = GetInt(currentLine);
			}
			break;
			case INHERIT_SCL:
			{
				currentNode->inheritScl.x = GetInt(currentLine);
				currentNode->inheritScl.z = GetInt(currentLine);
				currentNode->inheritScl.y = GetInt(currentLine);
			}
			break;
			case TM_ROW0:
			{
				currentNode->tm._11 = GetFloat(currentLine);
				currentNode->tm._13 = GetFloat(currentLine);
				currentNode->tm._12 = GetFloat(currentLine);

				/*currentNode->tm._11 = GetFloat(currentLine);
				currentNode->tm._31 = GetFloat(currentLine);
				currentNode->tm._21 = GetFloat(currentLine);*/
			}
			break;
			case TM_ROW1:
			{
				currentNode->tm._31 = GetFloat(currentLine);
				currentNode->tm._33 = GetFloat(currentLine);
				currentNode->tm._32 = GetFloat(currentLine);

				/*currentNode->tm._13 = GetFloat(currentLine);
				currentNode->tm._33 = GetFloat(currentLine);
				currentNode->tm._23 = GetFloat(currentLine);*/
			}
			break;
			case TM_ROW2:
			{
				currentNode->tm._21 = GetFloat(currentLine);
				currentNode->tm._23 = GetFloat(currentLine);
				currentNode->tm._22 = GetFloat(currentLine);

				/*currentNode->tm._12 = GetFloat(currentLine);
				currentNode->tm._32 = GetFloat(currentLine);
				currentNode->tm._22 = GetFloat(currentLine);*/
			}
			break;
			case TM_ROW3:
			{
				//aseData->geomObjects[curretnGeomObjectIdx]->tm.r[3].m128_f32[0] = GetFloat(currentLine);
				//aseData->geomObjects[curretnGeomObjectIdx]->tm.r[3].m128_f32[1] = GetFloat(currentLine);
				//aseData->geomObjects[curretnGeomObjectIdx]->tm.r[3].m128_f32[2] = GetFloat(currentLine);

				currentNode->tm._41 = GetFloat(currentLine);
				currentNode->tm._43 = GetFloat(currentLine);
				currentNode->tm._42 = GetFloat(currentLine);
				/*currentNode->tm._14 = GetFloat(currentLine);
				currentNode->tm._34 = GetFloat(currentLine);
				currentNode->tm._24 = GetFloat(currentLine);*/
			}
			break;
			case TM_POS:
			{
				currentNode->pos.x = GetFloat(currentLine);
				currentNode->pos.z = GetFloat(currentLine);
				currentNode->pos.y = GetFloat(currentLine);
			}
			break;
			case TM_ROTAXIS:
			{
				currentNode->rotAxis.x = GetFloat(currentLine);
				currentNode->rotAxis.z = GetFloat(currentLine);
				currentNode->rotAxis.y = GetFloat(currentLine);


			}
			break;
			case TM_ROTANGLE:
			{
				currentNode->rotAngle = GetFloat(currentLine);
			}
			break;
			case TM_SCALE:
			{
				currentNode->scale.x = GetFloat(currentLine);
				currentNode->scale.z = GetFloat(currentLine);
				currentNode->scale.y = GetFloat(currentLine);
			}
			break;
			case TM_SCALEAXIS:
			{
				currentNode->scaleAxis.x = GetFloat(currentLine);
				currentNode->scaleAxis.z = GetFloat(currentLine);
				currentNode->scaleAxis.y = GetFloat(currentLine);
			}
			break;
			case TM_SCALEAXISANG:
			{
				currentNode->scaleAxisAng = GetFloat(currentLine);
			}
			break;
			case MESH:
			{
				//"{" 삭제
				GetToken(currentLine);
			}
			break;
			case TIMEVALUE:
			{
				aseData->geomObjects[currentGeomObjectIdx]->mesh.timeValue = GetInt(currentLine);
			}
			break;
			case MESH_NUMVERTEX:
			{
				aseData->geomObjects[currentGeomObjectIdx]->mesh.numVertex = GetInt(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.vertexList.resize(aseData->geomObjects[currentGeomObjectIdx]->mesh.numVertex);

				// bone의 정보를 들고있는 boneWeightInfo resize // vertexList와	같은 인덱스를 공유한다.
				//aseData->geomObjects[currentGeomObjectIdx]->mesh.boneWeightInfo.resize(aseData->geomObjects[currentGeomObjectIdx]->mesh.numVertex);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.boneWeightInfo.resize(aseData->geomObjects[currentGeomObjectIdx]->mesh.numVertex, std::vector<BoneWeightInfo>(4));
			}
			break;
			case MESH_NUMFACES:
			{
				int size = GetInt(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.numFaces = size;
				aseData->geomObjects[currentGeomObjectIdx]->mesh.faceList.resize(size);

				///aseData->mesh.faceList.resize(size * 3);
				aseData->geomObjects[currentGeomObjectIdx]->vertices.resize(size * 3);
				aseData->geomObjects[currentGeomObjectIdx]->vertices2.resize(size * 3);
				aseData->geomObjects[currentGeomObjectIdx]->indices.resize(size * 3);
				// mk2
			}
			break;
			case MESH_VERTEX_LIST:
			{
				//"{" 삭제
				GetToken(currentLine);
			}
			break;
			case MESH_VERTEX:
			{
				int vIdx = GetInt(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.vertexList[vIdx].x = GetFloat(currentLine);

				// 좌표계가 다르므로 바꿔서 읽는다.
				aseData->geomObjects[currentGeomObjectIdx]->mesh.vertexList[vIdx].z = GetFloat(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.vertexList[vIdx].y = GetFloat(currentLine);
			}
			break;
			case MESH_FACE_LIST:
			{
				// "{" 삭제
				GetToken(currentLine);
			}
			break;
			case MESH_FACE:
			{
				// index 받아옴
				int fIdx = GetInt(currentLine);
				// : 삭제
				GetToken(currentLine);

				// A: 삭제
				GetToken(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.faceList[fIdx].x = GetInt(currentLine);
				// B: 삭제
				GetToken(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.faceList[fIdx].y = GetInt(currentLine);
				// C: 삭제
				GetToken(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.faceList[fIdx].z = GetInt(currentLine);

				// MESH SMOOTHING, MESH_MTLID 처리 안해줌.
				token = "";
			}
			break;
			case MESH_NORMALS:
			{
				// "{" 삭제
				GetToken(currentLine);
			}
			break;
			case MESH_FACENORMAL:
			{
				currentNormalIdx = GetInt(currentLine);
				XMFLOAT3 currentNormal1;
				currentNormal1.x = GetFloat(currentLine);
				currentNormal1.z = GetFloat(currentLine);
				currentNormal1.y = GetFloat(currentLine);
				// 내부에 인덱스가 다 있음. 그럼 이 순서대로 인덱스 버퍼를 설정하
				aseData->geomObjects[currentGeomObjectIdx]->mesh.normals.push_back(currentNormal1);
			}
			break;
			case MESH_VERTEXNORMAL:
			{
				int vIdx = GetInt(currentLine);
				XMFLOAT3 currentNormal1;
				currentNormal1.x = GetFloat(currentLine);
				currentNormal1.z = GetFloat(currentLine);
				currentNormal1.y = GetFloat(currentLine);


				// temp ; 이 부분 일단 걍 대충 짬. 
				currentFaceIdx;
				//aseData->vertices.push_back({ aseData->mesh.vertexList[vIdx], currentNormal1 });
				aseData->geomObjects[currentGeomObjectIdx]->vertices[currentIdx].pos = aseData->geomObjects[currentGeomObjectIdx]->mesh.vertexList[vIdx];
				aseData->geomObjects[currentGeomObjectIdx]->vertices[currentIdx].normal = currentNormal1;

				//aseData->indices.push_back(currentFaceIdx);
				aseData->geomObjects[currentGeomObjectIdx]->indices[currentIdx] = (currentFaceIdx);


				// Vertex MK2
				{
					aseData->geomObjects[currentGeomObjectIdx]->vertices2[currentIdx].posIdx = vIdx;
					aseData->geomObjects[currentGeomObjectIdx]->vertices2[currentIdx].normal = currentNormalIdx;
					aseData->geomObjects[currentGeomObjectIdx]->indices[currentIdx] = currentFaceIdx;
				}


				if (faceNextIdx == 0)
				{
					currentFaceIdx += 2;
					faceNextIdx = 1;
				}
				else if (faceNextIdx == 1)
				{
					currentFaceIdx -= 1;
					faceNextIdx = 2;
				}
				else
				{
					currentFaceIdx += 2;
					faceNextIdx = 0;
				}

				currentIdx++;
			}
			break;

			case MESH_NUMTVERTEX:
			{
				int size = GetInt(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.numTVertex = size;
				aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList.resize(size);
			}
			break;

			case MESH_TVERTLIST:
			{
				GetToken(currentLine);
			}
			break;

			case MESH_TVERT:
			{
				int idx = GetInt(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[idx].x = GetFloat(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[idx].y = GetFloat(currentLine);

				// w는 안받아옴.
				GetFloat(currentLine);
			}
			break;

			case MESH_NUMTVFACES:
			{
				int size = GetInt(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.numTFaces = size;
			}
			break;

			case MESH_TFACELIST:
			{
				GetToken(currentLine);
			}
			break;

			case MESH_TFACE:
			{
				int idx = GetInt(currentLine);
				int uv0 = GetInt(currentLine);
				int uv2 = GetInt(currentLine);
				int uv1 = GetInt(currentLine);

				aseData->geomObjects[currentGeomObjectIdx]->vertices[idx * 3 + 0].uv = aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[uv0];
				aseData->geomObjects[currentGeomObjectIdx]->vertices[idx * 3 + 1].uv = aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[uv2];
				aseData->geomObjects[currentGeomObjectIdx]->vertices[idx * 3 + 2].uv = aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[uv1];

				aseData->geomObjects[currentGeomObjectIdx]->vertices2[idx * 3 + 0].uv = aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[uv0];
				aseData->geomObjects[currentGeomObjectIdx]->vertices2[idx * 3 + 1].uv = aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[uv2];
				aseData->geomObjects[currentGeomObjectIdx]->vertices2[idx * 3 + 2].uv = aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[uv1];
			}
			break;

			case PROP_RECVSHADOW:
			{
				switch (currentObjectType)
				{
				case ObjectType::HELPEROBJECT:
					currentHelperObjectIdx++;
					break;
				case ObjectType::GEOMOBJECT:
					currentGeomObjectIdx++;
					break;
				}
				//return aseData;
			}
			break;
			case MESH_NUMBONE:
			{
				// resize
				((ASEData::GeomObject*)currentObject)->mesh.boneList.resize(GetInt(currentLine));
			}
			break;
			case BONE_LIST:
			{
				GetToken(currentLine);
			}
			break;
			case BONE:
			{
				currentBoneListIdx = GetInt(currentLine);
				GetToken(currentLine);
			}
			break;
			
			case BONE_NAME:
			{
				// idx 고쳐줘야하는데 어디서 고치지.. // 이미 고쳤었네?
				//aseData->geomObjects[currentGeomObjectIdx]->mesh.boneList
				//((ASEData::GeomObject*)currentObject)->mesh.boneList.push_back(GetString(currentLine));
				((ASEData::GeomObject*)currentObject)->mesh.boneList[currentBoneListIdx] = GetString(currentLine);
			}
			break;
			case MESH_WVERTEXS:
			{
				GetToken(currentLine);
			}
			break;
			case MESH_WEIGHT:
			{
				currentMeshIdxForBoneWeight = GetInt(currentLine);
				GetToken(currentLine);
				currentBoneWeightCount = 0;
			}
			break;
			case BONE_BLENGING_WEIGHT:
			{
				int boneIdx = GetInt(currentLine);
				float boneWeight = GetFloat(currentLine);
				//((ASEData::GeomObject*)currentObject)->mesh.boneWeightInfo[currentMeshIdxForBoneWeight].boneWeight[boneIdx - 1] = boneWeight;
				//((ASEData::GeomObject*)currentObject)->mesh.boneWeightInfo[currentMeshIdxForBoneWeight].push_back({ boneIdx, boneWeight });
				((ASEData::GeomObject*)currentObject)->mesh.boneWeightInfo[currentMeshIdxForBoneWeight][currentBoneWeightCount] = {(BYTE)boneIdx, boneWeight };

				// 한개의 매쉬에 몇개가 저장돼있는지를 알기 위한 카운트
				currentBoneWeightCount++;
			}
			break;
			default:
				break;
			}
		}
	}

	return aseData;
}

std::shared_ptr<ASEData> ASEParser::LoadASE3(std::string path)
{
	ifstream ifs;
	ifs.open(path);

	// 새로운 ASE data를 만들어 반환
	std::shared_ptr<ASEData> aseData = std::make_shared<ASEData>();

	// 현재 만들어지고 있는 GeomObject를 가리킴
	int currentGeomObjectIdx = 0;

	// 현재 만들어지고 있는 HelperObject를 가리킴
	int currentHelperObjectIdx = 0;

	// 현재 만들어지고 있는 ShapeObject를 가리킴
	int currentShapeObjectIdx = 0;

	// 컬링 순서를 변경하기 위한 변수
	int currentFaceIdx = 0;
	int faceNextIdx = 0;

	// 현재 받아온 normal의 인덱스를 가르킴
	int currentNormalIdx = 0;

	// Animation
	int animFrameCount = 0;
	// int prevAnimIdx = 0;
	// int nextAnimIdx = -1;

	// 애니메이션의 쿼터니언을 앞에 인덱스와 곱해주기 위해 일단 가지고 있는다.
	int animQuaternionIdx = -1;

	// 처음부터 index를 관리하면서 string data와 비교


	//vertices, indices index
	int currentIdx = 0;

	/// Bone Info
	int currentBoneListIdx = 0;
	int currentMeshIdxForBoneWeight = 0;

	// Bone Weight Info Idx
	int currentBoneWeightCount = 0;

	// 현재 노드 타입
	ObjectType currentObjectType = ObjectType::None;
	ASEData::NodeTm* currentNode = nullptr;
	ASEData::Object* currentObject = nullptr;

	// 일단 무한 루프
	while (true)
	{
		std::string currentLine;
		getline(ifs, currentLine);

		if (currentLine == "")
			break;

		string token = ".";
		int DataIdx = 0;
		while (token != "")
		{
			token = GetToken(currentLine);
			if (token == "") break;
			if (token == "}") { token = ""; break; }
			for (DataIdx = 0; DataIdx < ASE_TOKEN_SIZE; DataIdx++)
			{
				if (token == ASE_TOKEN_STRING_DATA[DataIdx])
				{
					break;
				}
			}

			// 여기서 token에 따라 처리

			switch (DataIdx)
			{
			case _3DSMAX_ASCIIEXPORT:
			{
				// 이상한 숫자?
				int a = GetInt(currentLine);
				int b = 3;
			}
			break;
			case COMMENT:
			{
				// 주석
				GetString(currentLine);
			}
			break;
			case SCENE:
			{
				// "{" 를 지운다.
				GetToken(currentLine);
			}
			break;
			case SCENE_FILENAME:
			{
				aseData->scene.fileName = GetString(currentLine);
			}
			break;
			case SCENE_FIRSTFRAME:
			{
				aseData->scene.firstFrame = GetInt(currentLine);
			}
			break;
			case SCENE_LASTFRAME:
			{
				aseData->scene.lastFrame = GetInt(currentLine);
			}
			break;
			case SCENE_FRAMESPEED:
			{
				aseData->scene.frameSpeed = GetInt(currentLine);
			}
			break;
			case SCENE_TICKSPERFRAME:
			{
				aseData->scene.ticksPerFrame = GetInt(currentLine);

				animFrameCount = (aseData->scene.lastFrame + 1) * aseData->scene.ticksPerFrame;
				aseData->scene.frameCount = animFrameCount;
			}
			break;

			case HELPEROBJECT:
			{
				ASEData::HelperObject* helperObj = new ASEData::HelperObject();
				aseData->helperObjects.push_back(helperObj);

				Matrix I = DirectX::XMMatrixIdentity();
				XMStoreFloat4x4(&aseData->helperObjects[currentHelperObjectIdx]->node.tm, I);

				currentObjectType = ObjectType::HELPEROBJECT;
				currentNode = &aseData->helperObjects[currentHelperObjectIdx]->node;

				// 현재 오브젝트가 무엇인지
				currentObject = helperObj;
			}
			break;

			case BOUNDINGBOX_MIN:
			{
				currentHelperObjectIdx++;
			}
			break;

			case SHAPEOBJECT:
			{

				ASEData::ShapeObject* shapeObj = new ASEData::ShapeObject();
				aseData->shapeObjects.push_back(shapeObj);

				XMMATRIX I = XMMatrixIdentity();
				XMStoreFloat4x4(&aseData->shapeObjects[currentShapeObjectIdx]->node.tm, I);

				currentObjectType = ObjectType::SHAPEOBJECT;
				currentNode = &aseData->shapeObjects[currentShapeObjectIdx]->node;
				currentObject = shapeObj;
				/*int blockCount = 0;
				GetToken(currentLine);
				blockCount++;

				while (blockCount != 0)
				{
					std::string currentLine2;
					getline(ifs, currentLine2);

					string token2 = ".";
					while (token2 != "")
					{
						token2 = GetToken(currentLine2);
						if (token2 == "{")
							blockCount++;
						else if (token2 == "}")
							blockCount--;
					}
				}*/
			}
			break;
			case SHAPE_LINECOUNT:
			{
				currentShapeObjectIdx++;
			}
			break;
			case TM_ANIMATION:
			{
				int blockCount = 0;
				GetToken(currentLine);
				blockCount++;

				while (blockCount != 0)
				{
					std::string currentLine2;
					getline(ifs, currentLine2);

					string token2 = ".";
					while (token2 != "")
					{
						token2 = GetToken(currentLine2);
						if (token2 == "{")
							blockCount++;
						else if (token2 == "}")
							blockCount--;
						else if (token2 == "*CONTROL_POS_TRACK")
						{
							//currentObject->animInfo.controlPosTrack.resize(animFrameCount);
						}
						else if (token2 == "*CONTROL_POS_SAMPLE")
						{
							int frame = GetInt(currentLine2);
							SimpleMath::Vector3 pos = GetVectorXZY(currentLine2);
							//currentObject->animInfo.controlPosTrack[animIdx] = pos;

							ASEData::AnimIdxVector currentAnimInfo;
							currentAnimInfo.frame = frame;
							currentAnimInfo.vector3 = pos;

							currentObject->animInfo.controlPosTrack.push_back(currentAnimInfo);
						}
						else if (token2 == "*CONTROL_ROT_TRACK")
						{
							//currentObject->animInfo.controlRotTrack.resize(animFrameCount);
						}
						else if (token2 == "*CONTROL_ROT_SAMPLE")
						{
							int frame = GetInt(currentLine2);
							SimpleMath::Vector3 rotAxis = GetVectorXZY(currentLine2);
							float angle = GetFloat(currentLine2);

							RotationAxisAndAngle currentAnimRotInfo;
							currentAnimRotInfo.frame = frame;
							currentAnimRotInfo.axis = rotAxis;
							currentAnimRotInfo.angle = angle;
							if (currentObject->animInfo.controlRotTrack.size() == 0)
							{
								currentAnimRotInfo.quat = SimpleMath::Quaternion(rotAxis, angle);
							}
							else
							{
								// currentAnimRotInfo.quat = currentObject->animInfo.controlRotTrack.back().quat * SimpleMath::Quaternion(rotAxis, angle);
								currentAnimRotInfo.quat = currentObject->animInfo.controlRotTrack.back().quat * SimpleMath::Quaternion(rotAxis, angle);
							}
							currentObject->animInfo.controlRotTrack.push_back(currentAnimRotInfo);
							//currentObject->animInfo.controlRotTrack[animIdx] = { rotAxis,angle };
						}

						// scale은 없으니까 나중에 언젠가 반드시
					}
				}
			}
			break;
			case SCENE_BACKGROUND_STATIC:
			{
				aseData->scene.backgroundStatic.x = GetFloat(currentLine);
				aseData->scene.backgroundStatic.y = GetFloat(currentLine);
				aseData->scene.backgroundStatic.z = GetFloat(currentLine);
			}
			break;
			case SCENE_AMBIENT_STATIC:
			{
				aseData->scene.ambientStatic.x = GetFloat(currentLine);
				aseData->scene.ambientStatic.y = GetFloat(currentLine);
				aseData->scene.ambientStatic.z = GetFloat(currentLine);
			}
			break;

			case MATERIAL_LIST:
			{

			}
			break;

			case GEOMOBJECT:
			{
				currentObjectType = ObjectType::GEOMOBJECT;

				//"{" 삭제
				GetToken(currentLine);
				ASEData::GeomObject* currentGeomObject = new ASEData::GeomObject();
				aseData->geomObjects.push_back(currentGeomObject);

				currentFaceIdx = 0;
				faceNextIdx = 0;
				currentIdx = 0;
				XMMATRIX I = XMMatrixIdentity();
				XMStoreFloat4x4(&aseData->geomObjects[currentGeomObjectIdx]->node.tm, I);

				currentNode = &aseData->geomObjects[currentGeomObjectIdx]->node;
				currentObject = currentGeomObject;
			}
			break;
			case NODE_NAME:
			{
				//aseData->geomObject.nodeName = GetString(currentLine);
				//aseData->geomObjects[currentGeomObjectIdx]->node.nodeName = GetString(currentLine);
				currentNode->nodeName = GetString(currentLine);
				currentNode->tm = SimpleMath::Matrix::Identity;
			}
			break;
			case NODE_PARENT:
			{
				//aseData->geomObjects[currentGeomObjectIdx]->node.parentNodeName = GetString(currentLine);
				currentNode->parentNodeName = GetString(currentLine);
			}
			break;
			case NODE_TM:
			{
				// "{" 삭제
				GetToken(currentLine);
			}
			break;
			case NODE_NAME_2:
			{
				// 임시 용 
				//aseData->geomObject.nodeName = GetString(currentLine);
				//aseData->geomObjects[currentGeomObjectIdx]->node.nodeName = GetString(currentLine);
				currentNode->nodeName = GetString(currentLine);
			}
			break;
			case INHERIT_POS:
			{
				// 					aseData->geomObjects[currentGeomObjectIdx]->node.inheritPos.x = GetInt(currentLine);
				// 					aseData->geomObjects[currentGeomObjectIdx]->node.inheritPos.z = GetInt(currentLine);
				// 					aseData->geomObjects[currentGeomObjectIdx]->node.inheritPos.y = GetInt(currentLine);
				currentNode->inheritPos.x = GetInt(currentLine);
				currentNode->inheritPos.z = GetInt(currentLine);
				currentNode->inheritPos.y = GetInt(currentLine);
			}
			break;
			case INHERIT_ROT:
			{
				currentNode->inheritRot.x = GetInt(currentLine);
				currentNode->inheritRot.z = GetInt(currentLine);
				currentNode->inheritRot.y = GetInt(currentLine);
			}
			break;
			case INHERIT_SCL:
			{
				currentNode->inheritScl.x = GetInt(currentLine);
				currentNode->inheritScl.z = GetInt(currentLine);
				currentNode->inheritScl.y = GetInt(currentLine);
			}
			break;
			case TM_ROW0:
			{
				currentNode->tm._11 = GetFloat(currentLine);
				currentNode->tm._13 = GetFloat(currentLine);
				currentNode->tm._12 = GetFloat(currentLine);

				/*currentNode->tm._11 = GetFloat(currentLine);
				currentNode->tm._31 = GetFloat(currentLine);
				currentNode->tm._21 = GetFloat(currentLine);*/
			}
			break;
			case TM_ROW1:
			{
				currentNode->tm._31 = GetFloat(currentLine);
				currentNode->tm._33 = GetFloat(currentLine);
				currentNode->tm._32 = GetFloat(currentLine);

				/*currentNode->tm._13 = GetFloat(currentLine);
				currentNode->tm._33 = GetFloat(currentLine);
				currentNode->tm._23 = GetFloat(currentLine);*/
			}
			break;
			case TM_ROW2:
			{
				currentNode->tm._21 = GetFloat(currentLine);
				currentNode->tm._23 = GetFloat(currentLine);
				currentNode->tm._22 = GetFloat(currentLine);

				/*currentNode->tm._12 = GetFloat(currentLine);
				currentNode->tm._32 = GetFloat(currentLine);
				currentNode->tm._22 = GetFloat(currentLine);*/
			}
			break;
			case TM_ROW3:
			{
				//aseData->geomObjects[curretnGeomObjectIdx]->tm.r[3].m128_f32[0] = GetFloat(currentLine);
				//aseData->geomObjects[curretnGeomObjectIdx]->tm.r[3].m128_f32[1] = GetFloat(currentLine);
				//aseData->geomObjects[curretnGeomObjectIdx]->tm.r[3].m128_f32[2] = GetFloat(currentLine);

				currentNode->tm._41 = GetFloat(currentLine);
				currentNode->tm._43 = GetFloat(currentLine);
				currentNode->tm._42 = GetFloat(currentLine);
				/*currentNode->tm._14 = GetFloat(currentLine);
				currentNode->tm._34 = GetFloat(currentLine);
				currentNode->tm._24 = GetFloat(currentLine);*/
			}
			break;
			case TM_POS:
			{
				currentNode->pos.x = GetFloat(currentLine);
				currentNode->pos.z = GetFloat(currentLine);
				currentNode->pos.y = GetFloat(currentLine);
			}
			break;
			case TM_ROTAXIS:
			{
				currentNode->rotAxis.x = GetFloat(currentLine);
				currentNode->rotAxis.z = GetFloat(currentLine);
				currentNode->rotAxis.y = GetFloat(currentLine);


			}
			break;
			case TM_ROTANGLE:
			{
				currentNode->rotAngle = GetFloat(currentLine);
			}
			break;
			case TM_SCALE:
			{
				currentNode->scale.x = GetFloat(currentLine);
				currentNode->scale.z = GetFloat(currentLine);
				currentNode->scale.y = GetFloat(currentLine);
			}
			break;
			case TM_SCALEAXIS:
			{
				currentNode->scaleAxis.x = GetFloat(currentLine);
				currentNode->scaleAxis.z = GetFloat(currentLine);
				currentNode->scaleAxis.y = GetFloat(currentLine);
			}
			break;
			case TM_SCALEAXISANG:
			{
				currentNode->scaleAxisAng = GetFloat(currentLine);
			}
			break;
			case MESH:
			{
				//"{" 삭제
				GetToken(currentLine);
			}
			break;
			case TIMEVALUE:
			{
				aseData->geomObjects[currentGeomObjectIdx]->mesh.timeValue = GetInt(currentLine);
			}
			break;
			case MESH_NUMVERTEX:
			{
				aseData->geomObjects[currentGeomObjectIdx]->mesh.numVertex = GetInt(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.vertexList.resize(aseData->geomObjects[currentGeomObjectIdx]->mesh.numVertex);

				// bone의 정보를 들고있는 boneWeightInfo resize // vertexList와	같은 인덱스를 공유한다.
				//aseData->geomObjects[currentGeomObjectIdx]->mesh.boneWeightInfo.resize(aseData->geomObjects[currentGeomObjectIdx]->mesh.numVertex);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.boneWeightInfo.resize(aseData->geomObjects[currentGeomObjectIdx]->mesh.numVertex, std::vector<BoneWeightInfo>(4));
			}
			break;
			case MESH_NUMFACES:
			{
				int size = GetInt(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.numFaces = size;
				aseData->geomObjects[currentGeomObjectIdx]->mesh.faceList.resize(size);

				///aseData->mesh.faceList.resize(size * 3);
				aseData->geomObjects[currentGeomObjectIdx]->vertices.resize(size * 3);
				aseData->geomObjects[currentGeomObjectIdx]->vertices2.resize(size * 3);
				aseData->geomObjects[currentGeomObjectIdx]->indices.resize(size * 3);
				// mk2
			}
			break;
			case MESH_VERTEX_LIST:
			{
				//"{" 삭제
				GetToken(currentLine);
			}
			break;
			case MESH_VERTEX:
			{
				int vIdx = GetInt(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.vertexList[vIdx].x = GetFloat(currentLine);

				// 좌표계가 다르므로 바꿔서 읽는다.
				aseData->geomObjects[currentGeomObjectIdx]->mesh.vertexList[vIdx].z = GetFloat(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.vertexList[vIdx].y = GetFloat(currentLine);
			}
			break;
			case MESH_FACE_LIST:
			{
				// "{" 삭제
				GetToken(currentLine);
			}
			break;
			case MESH_FACE:
			{
				// index 받아옴
				int fIdx = GetInt(currentLine);
				// : 삭제
				GetToken(currentLine);

				// A: 삭제
				GetToken(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.faceList[fIdx].x = GetInt(currentLine);
				// B: 삭제
				GetToken(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.faceList[fIdx].y = GetInt(currentLine);
				// C: 삭제
				GetToken(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.faceList[fIdx].z = GetInt(currentLine);

				// MESH SMOOTHING, MESH_MTLID 처리 안해줌.
				token = "";
			}
			break;
			case MESH_NORMALS:
			{
				// "{" 삭제
				GetToken(currentLine);
			}
			break;
			case MESH_FACENORMAL:
			{
				currentNormalIdx = GetInt(currentLine);
				XMFLOAT3 currentNormal1;
				currentNormal1.x = GetFloat(currentLine);
				currentNormal1.z = GetFloat(currentLine);
				currentNormal1.y = GetFloat(currentLine);
				// 내부에 인덱스가 다 있음. 그럼 이 순서대로 인덱스 버퍼를 설정하
				aseData->geomObjects[currentGeomObjectIdx]->mesh.normals.push_back(currentNormal1);
			}
			break;
			case MESH_VERTEXNORMAL:
			{
				int vIdx = GetInt(currentLine);
				XMFLOAT3 currentNormal1;
				currentNormal1.x = GetFloat(currentLine);
				currentNormal1.z = GetFloat(currentLine);
				currentNormal1.y = GetFloat(currentLine);


				// temp ; 이 부분 일단 걍 대충 짬. 
				currentFaceIdx;
				//aseData->vertices.push_back({ aseData->mesh.vertexList[vIdx], currentNormal1 });
				aseData->geomObjects[currentGeomObjectIdx]->vertices[currentIdx].pos = aseData->geomObjects[currentGeomObjectIdx]->mesh.vertexList[vIdx];
				aseData->geomObjects[currentGeomObjectIdx]->vertices[currentIdx].normal = currentNormal1;

				//aseData->indices.push_back(currentFaceIdx);
				aseData->geomObjects[currentGeomObjectIdx]->indices[currentIdx] = (currentFaceIdx);


				// Vertex MK2
				{
					aseData->geomObjects[currentGeomObjectIdx]->vertices2[currentIdx].posIdx = vIdx;
					aseData->geomObjects[currentGeomObjectIdx]->vertices2[currentIdx].normal = currentNormalIdx;
					aseData->geomObjects[currentGeomObjectIdx]->indices[currentIdx] = currentFaceIdx;
				}


				if (faceNextIdx == 0)
				{
					currentFaceIdx += 2;
					faceNextIdx = 1;
				}
				else if (faceNextIdx == 1)
				{
					currentFaceIdx -= 1;
					faceNextIdx = 2;
				}
				else
				{
					currentFaceIdx += 2;
					faceNextIdx = 0;
				}

				currentIdx++;
			}
			break;

			case MESH_NUMTVERTEX:
			{
				int size = GetInt(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.numTVertex = size;
				aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList.resize(size);
			}
			break;

			case MESH_TVERTLIST:
			{
				GetToken(currentLine);
			}
			break;

			case MESH_TVERT:
			{
				int idx = GetInt(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[idx].x = GetFloat(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[idx].y = GetFloat(currentLine);

				// w는 안받아옴.
				GetFloat(currentLine);
			}
			break;

			case MESH_NUMTVFACES:
			{
				int size = GetInt(currentLine);
				aseData->geomObjects[currentGeomObjectIdx]->mesh.numTFaces = size;
			}
			break;

			case MESH_TFACELIST:
			{
				GetToken(currentLine);
			}
			break;

			case MESH_TFACE:
			{
				int idx = GetInt(currentLine);
				int uv0 = GetInt(currentLine);
				int uv2 = GetInt(currentLine);
				int uv1 = GetInt(currentLine);

				aseData->geomObjects[currentGeomObjectIdx]->vertices[idx * 3 + 0].uv = aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[uv0];
				aseData->geomObjects[currentGeomObjectIdx]->vertices[idx * 3 + 1].uv = aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[uv2];
				aseData->geomObjects[currentGeomObjectIdx]->vertices[idx * 3 + 2].uv = aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[uv1];

				aseData->geomObjects[currentGeomObjectIdx]->vertices2[idx * 3 + 0].uv = aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[uv0];
				aseData->geomObjects[currentGeomObjectIdx]->vertices2[idx * 3 + 1].uv = aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[uv2];
				aseData->geomObjects[currentGeomObjectIdx]->vertices2[idx * 3 + 2].uv = aseData->geomObjects[currentGeomObjectIdx]->mesh.tVertexList[uv1];
			}
			break;

			case PROP_RECVSHADOW:
			{
				switch (currentObjectType)
				{
				case ObjectType::HELPEROBJECT:
					currentHelperObjectIdx++;
					break;
				case ObjectType::GEOMOBJECT:
					currentGeomObjectIdx++;
					break;
				}
				//return aseData;
			}
			break;
			case MESH_NUMBONE:
			{
				// resize
				((ASEData::GeomObject*)currentObject)->mesh.boneList.resize(GetInt(currentLine));
			}
			break;
			case BONE_LIST:
			{
				GetToken(currentLine);
			}
			break;
			case BONE:
			{
				currentBoneListIdx = GetInt(currentLine);
				GetToken(currentLine);
			}
			break;

			case BONE_NAME:
			{
				// idx 고쳐줘야하는데 어디서 고치지.. // 이미 고쳤었네?
				//aseData->geomObjects[currentGeomObjectIdx]->mesh.boneList
				//((ASEData::GeomObject*)currentObject)->mesh.boneList.push_back(GetString(currentLine));
				((ASEData::GeomObject*)currentObject)->mesh.boneList[currentBoneListIdx] = GetString(currentLine);
			}
			break;
			case MESH_WVERTEXS:
			{
				GetToken(currentLine);
			}
			break;
			case MESH_WEIGHT:
			{
				currentMeshIdxForBoneWeight = GetInt(currentLine);
				GetToken(currentLine);
				currentBoneWeightCount = 0;
			}
			break;
			case BONE_BLENGING_WEIGHT:
			{
				int boneIdx = GetInt(currentLine);
				float boneWeight = GetFloat(currentLine);
				//((ASEData::GeomObject*)currentObject)->mesh.boneWeightInfo[currentMeshIdxForBoneWeight].boneWeight[boneIdx - 1] = boneWeight;
				//((ASEData::GeomObject*)currentObject)->mesh.boneWeightInfo[currentMeshIdxForBoneWeight].push_back({ boneIdx, boneWeight });
				((ASEData::GeomObject*)currentObject)->mesh.boneWeightInfo[currentMeshIdxForBoneWeight][currentBoneWeightCount] = { (BYTE)boneIdx, boneWeight };

				// 한개의 매쉬에 몇개가 저장돼있는지를 알기 위한 카운트
				currentBoneWeightCount++;
			}
			break;
			default:
				break;
			}
		}
	}

	return aseData;
}

void ASEParser::SkipWhitespace(std::string& line)
{
	// 앞에서부터 문자를 만날때 까지 공백 삭제시킴
	int idx = 0;
	while (idx < line.length() && line[idx] == ' ' || line[idx] == '\t')
	{
		idx++;
	}

	line.erase(0, idx);
}

std::string ASEParser::GetToken(std::string& line)
{
	// 공백을 비워줌 
	SkipWhitespace(line);

	int idx = 0;
	std::string token = "";

	while (!line.empty() && idx < line.length() && line[idx] != ' ' && line[idx] != '\t')
	{
		token += line[idx];
		idx++;
	}
	line.erase(0, idx);
	return token;
}

int ASEParser::GetInt(std::string& line)
{
	SkipWhitespace(line);

	int idx = 0;
	std::string intVal = "";
	while (!line.empty() && idx < line.length() && line[idx] != ' ' && line[idx] != '\t' && line[idx] != ':')
	{
		intVal += line[idx];
		idx++;
	}
	line.erase(0, idx);

	return stoi(intVal);
}

float ASEParser::GetFloat(std::string& line)
{
	SkipWhitespace(line);

	int idx = 0;
	std::string floatVal = "";
	while (!line.empty() && idx < line.length() && line[idx] != ' ' && line[idx] != '\t')
	{
		floatVal += line[idx];
		idx++;
	}
	line.erase(0, idx);

	return stof(floatVal);
}

std::string ASEParser::GetString(std::string& line)
{
	SkipWhitespace(line);

	int idx = 1;
	std::string stringDat = "\"";
	while (!line.empty() && idx < line.length() && line[idx] != '\"')
	{
		stringDat += line[idx];
		idx++;
	}
	line.erase(0, idx);

	// 따움표를 지워야함.
	line.erase(0, 1);
	// 따움표를 지워야함.
	stringDat.erase(0, 1);

	return stringDat;
}

DirectX::SimpleMath::Vector3 ASEParser::GetVectorXZY(std::string& line)
{
	SimpleMath::Vector3 ret;
	ret.x = GetFloat(line);
	ret.z = GetFloat(line);
	ret.y = GetFloat(line);
	return ret;
}
