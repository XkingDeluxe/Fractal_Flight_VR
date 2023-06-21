#pragma once
#include "GL/CAPI_GLE.h"
#include "Extras/OVR_Math.h"
#include "OculusTexBuffer.h"
#include "Scene.h"
using namespace OVR;

class RenderEye {
public:
	ovrPosef eyeRenderPose;
	ovrPosef hmdToEyePose;
	ovrHmdDesc hmdDesc;
	OculusTexBuffer* eyeRenderTexture;

	RenderEye(ovrSession _session, int eyeIndex);

	bool createBuffer();

	void Update();

	Quatf getOrientation(void) { return eyeRenderPose.Orientation; }

	Vector3f getPos(void) { return eyeRenderPose.Position; }
private:
	ovrSession session;
	
	ovrEyeRenderDesc eyeRenderDesc;
	

	int eye;


};

class Player {
public:
	RenderEye* eyes[2];

	Player(ovrSession);

	void Update(int frameIndex, Scene* scene);

	void inputHandler();

	double getSensorSampleTime(void) { return sensorSampleTime; }

	ovrTimewarpProjectionDesc getTWPD(void) { return posTimewarpProjectionDesc; }

private:
	ovrSession session;
	ovrTimewarpProjectionDesc posTimewarpProjectionDesc;

	
	Vector3f pos;
	Quatf orientation;
	double sensorSampleTime;
	float speed = 0.0f;
};