#pragma once
#include <assert.h>
#include "GL/CAPI_GLE.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_GL.h"
#include "Shader.h"
#include "TextureBuffer.h"
#include <SOIL2/SOIL2.h>
#include "Model.h"

#define _SOLUTIONDIR R"($(SolutionDir))" 
using namespace OVR;

class Scene {
public:
	Scene(ovrSession _session);
	~Scene();

	void AddModel(Model* model);
	void Init();

	void Render(Matrix4f view, Matrix4f proj, Vector3f playerPos);

private:
	GLuint numModels;
	Model* models[10];
	ovrSession session;
	Vector3f playerPos;
};