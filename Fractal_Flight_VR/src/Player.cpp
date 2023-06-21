#include "Player.h"

RenderEye::RenderEye(ovrSession _session, int eyeIndex) : session(_session), hmdDesc(ovr_GetHmdDesc(session)), eye(eyeIndex) {
	eyeRenderTexture = { nullptr };
}

bool RenderEye::createBuffer() {
	ovrSizei idealTextureSize = ovr_GetFovTextureSize(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1);
	eyeRenderTexture = new OculusTexBuffer(session, idealTextureSize, 1);
	if (!eyeRenderTexture->ColorTextureChain || !eyeRenderTexture->DepthTextureChain) {
		return false;
	}
	return true;
}

void RenderEye::Update() {
	eyeRenderDesc = ovr_GetRenderDesc(session, (ovrEyeType)eye, hmdDesc.DefaultEyeFov[eye]);
	hmdToEyePose = eyeRenderDesc.HmdToEyePose;
}




Player::Player(ovrSession _session) : session(_session), pos(0.0f, 0.0f, 0.0f) {
	eyes[0] = new RenderEye(session, ovrEye_Left);
	eyes[1] = new RenderEye(session, ovrEye_Right);

	eyes[0]->createBuffer();
	eyes[1]->createBuffer();
}

void Player::Update(int frameIndex, Scene* scene) {
	eyes[0]->Update();
	eyes[1]->Update();


	ovrPosef b[2];
	ovrPosef a[] = { eyes[0]->hmdToEyePose, eyes[1]->hmdToEyePose };

	ovr_GetEyePoses(session, frameIndex, ovrTrue, a, b, &sensorSampleTime);
	eyes[0]->eyeRenderPose = b[0];
	eyes[1]->eyeRenderPose = b[1];

	//inputHandler();
	// Render Scene to Eye Buffers
	for (int eye = 0; eye < 2; ++eye)
	{
		// Switch to eye render target
		eyes[eye]->eyeRenderTexture->SetAndClearRenderSurface();


		// Get view and projection matrices
		Matrix4f rollPitchYaw = Matrix4f::RotationY(3.141592f);
		Matrix4f finalRollPitchYaw = rollPitchYaw * Matrix4f(eyes[eye]->getOrientation());
		Vector3f finalUp = finalRollPitchYaw.Transform(Vector3f(0, 1, 0));
		Vector3f finalForward = finalRollPitchYaw.Transform(Vector3f(0, 0, -1));
		Vector3f shiftedEyePos = pos + rollPitchYaw.Transform(eyes[eye]->getPos());

		Matrix4f view = Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);
		Matrix4f proj = ovrMatrix4f_Projection(eyes[eye]->hmdDesc.DefaultEyeFov[eye], 0.2f, 1000.0f, ovrProjection_None);
		posTimewarpProjectionDesc = ovrTimewarpProjectionDesc_FromProjection(proj, ovrProjection_None);

		scene->Render(view, proj, pos);//-----------------------------------------------------------------------------------------------

		eyes[eye]->eyeRenderTexture->UnsetRenderSurface();

		eyes[eye]->eyeRenderTexture->Commit();
	}
}

/*void Player::inputHandler() {
	ovrInputState inputState;
	if (OVR_SUCCESS(ovr_GetInputState(session, ovrControllerType_Touch, &inputState))) {
		Quatf rotation = eyes[1]->getOrientation();
		rotation.x = 0.0f;
		rotation.z = 0.0f;
		rotation.Normalize();
		pos += Matrix4f(rotation).Transform(Vector3f(-inputState.Thumbstick[0].x / 20.0f, 0.0f, inputState.Thumbstick[0].y / 20.0f) );
	}
}*/