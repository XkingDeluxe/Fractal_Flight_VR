#pragma once
#include "GL/CAPI_GLE.h"
#include <Extras/OVR_Math.h>
#include "Shader.h"
#include <vector>
#include <string>
using namespace OVR;
struct Vertex {
	Vector3f Pos;
	Vector3f Normal;
	Vector2f UV;
	Vertex(){}
	Vertex(Vector3f _Pos, Vector3f _Normal, Vector2f _UV): Pos(_Pos), Normal(_Normal), UV(_UV){}
};
struct Index {
	GLuint PosInd;
	GLuint NormalInd;
	GLuint UVInd;
	Index() {}
	Index(GLuint _Pos, GLuint _Normal, GLuint _UV) : PosInd(_Pos), NormalInd(_Normal), UVInd(_UV) {}
};

class VertexBuffer {
private:
	GLuint buffer;
public:
	VertexBuffer(void* vertices, void* normals, void* uvs, size_t sizeV, size_t sizeN, size_t sizeU);
	~VertexBuffer();
	GLuint getBuffer(void) { return buffer; }
};
class IndexBuffer {
private:
	GLuint buffer;
public:
	IndexBuffer(void* indices, size_t size);
	~IndexBuffer();
	GLuint getBuffer(void) { return buffer; }
};

class Model {
private:
	std::vector<Vector3f> vertices;
	std::vector<Vector3f> normals;
	std::vector<Vector2f> UVs;
	//std::vector<GLuint> indices;

	Vector3f Pos;
	Vector3f Scale;
	Quatf Rot;
	Vector3f RotOffset;
	Matrix4f Mat;
	VertexBuffer* vertexBuffer;
	//IndexBuffer* indexBuffer;

	ShaderFill* fill;

	Vector3f lightPos;
public:
	Model(Vector3f pos, ShaderFill* s);
	~Model();

	void AllocateBuffers();
	void FreeBuffers();

	Matrix4f getMatrix();

	Vector3f getLightPos(void) { return lightPos; };
	void setLightPos(Vector3f lpos) { this->lightPos = lpos; }


	void AddCube(float xpos, float ypos, float zpos);

	void AddIndex(GLuint i);
	bool LoadOBJ(std::string filePath);

	void MoveToPos(Vector3f pos);
	void ScaleObj(Vector3f sc);
	void RotateObj(Quatf rotation, Vector3f rotOffset);

	void Render(Matrix4f view, Matrix4f proj, Vector3f playerPos);

};