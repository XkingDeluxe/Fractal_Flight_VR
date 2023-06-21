#include "Scene.h"
Scene::Scene(ovrSession _session) : session(_session), playerPos(0.0f, 0.0f, 0.0f)
{
	Init();
}
Scene::~Scene() {

}

void Scene::AddModel(Model* n) {
	models[numModels++] = n;
}

void Scene::Init() {

	Shader woodShader("src/Shaders/Wood/trippyWood.vs", "src/Shaders/Wood/trippyWood.frag");

	int textureWidth, textureHeight;
	unsigned char* image;
	image = SOIL_load_image("src/Images/Wood/wood1.jpg", &textureWidth, &textureHeight, 0, SOIL_LOAD_RGB);
	TextureBuffer* diffuseBuffer = new TextureBuffer(Sizei(textureWidth, textureHeight), image);
	SOIL_free_image_data(image);

	image = SOIL_load_image("sec/Images/Wood/wood1_specular.jpg", &textureWidth, &textureHeight, 0, SOIL_LOAD_RGB);
	TextureBuffer* specularBuffer = new TextureBuffer(Sizei(textureWidth, textureHeight), image);
	SOIL_free_image_data(image);

	woodShader.Use();
	glUniform1i(glGetUniformLocation(woodShader.Program, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(woodShader.Program, "material.specular"), 1);

	ShaderFill* fill = new ShaderFill(woodShader.Program, diffuseBuffer->texId, specularBuffer->texId);



	/*Model* cube = new Model(Vector3f(5.0f, 0.0f, 0.0f), fill);
	
	cube->AddCube(0.0f, 0.0f, 0.0f);
	cube->RotateObj(Quatf(Vector3f(1.0f, 0.0f, 0.0f), 90));
	cube->ScaleObj(Vector3f(0.2f, 0.2f, 0.5f));
	
	cube->AllocateBuffers();
	AddModel(cube);*/

	Model* ape1 = new Model(Vector3f(0.0f, 0.0f, 0.0f), fill);
	ape1->LoadOBJ("src/Objects/test/zwaard.obj");
	ape1->ScaleObj(Vector3f(0.75f, 0.75f, 0.75f));
	ape1->AllocateBuffers();
	AddModel(ape1);

	Model* ape2 = new Model(Vector3f(0.0f, 0.0f, 0.0f), fill);
	ape2->LoadOBJ("src/Objects/test/zwaard.obj");
	ape1->ScaleObj(Vector3f(0.75f, 0.75f, 0.75f));
	ape2->AllocateBuffers();
	AddModel(ape2);

	/*Model* outside = new Model(Vector3f(0.0f, 0.0f, 0.0f), fill);*/
	/*outside->LoadOBJ("src/Objects/cabine/outside.obj");*/
	Model* throttle = new Model(Vector3f(0.002549f, 0.35759f, 0.70784f), fill);
	throttle->LoadOBJ("src/Objects/cabine/chair.obj");
	throttle->AllocateBuffers();
	AddModel(throttle);
}

bool hold = true;
Vector3f angVel;
Quatf rot;
Vector3f rotOffset;
Vector3f pos;
Vector3f vel;

float gravity;

float prevTime;

void Scene::Render(Matrix4f view, Matrix4f proj, Vector3f playerPos) {
	ovrTrackingState trackState = ovr_GetTrackingState(session, ovr_GetTimeInSeconds(), ovrTrue);
	ovrInputState inputstate;
	ovr_GetInputState(session, ovrControllerType_Touch, &inputstate);
	Vector3f righthandPos = (Vector3f)trackState.HandPoses[1].ThePose.Position;
	righthandPos.x *= -1.0f;
	righthandPos.z *= -1.0f;
	Quatf righthandRot = trackState.HandPoses[1].ThePose.Orientation;
	righthandRot.x *= -1.0f;
	righthandRot.z *= -1.0f;
	righthandRot.Normalize();
	
	
	models[0]->MoveToPos(righthandPos + playerPos);
	models[0]->RotateObj(righthandRot, Vector3f(3.14159/4.0f, ovr_GetTimeInSeconds()*10.0f, 0.0f));

	Vector3f lefthandPos = (Vector3f)trackState.HandPoses[0].ThePose.Position;
	lefthandPos.x *= -1.0f;
	lefthandPos.z *= -1.0f;
	Quatf lefthandRot = trackState.HandPoses[0].ThePose.Orientation;
	lefthandRot.x *= -1.0f;
	lefthandRot.z *= -1.0f;
	lefthandRot.Normalize();

	/*models[1]->MoveToPos(lefthandPos + playerPos);
	models[1]->RotateObj(lefthandRot, Vector3f(0.7f, ovr_GetTimeInSeconds() * 50.0f, 0.0f));*/

	

	std::cout << righthandRot.x << " " << righthandRot.y << " " << righthandRot.z << " " << righthandRot.w << std::endl;
	
	//https://karllewisdesign.com/how-to-improve-throwing-physics-in-vr/

	if (hold) {
		models[1]->MoveToPos(lefthandPos + playerPos);

		rotOffset = Vector3f(3.14159f/4.0f, 3.14159f/2.0f, 0.0f);
		models[1]->RotateObj(lefthandRot, rotOffset);
		

		if(inputstate.Buttons == 256){
			pos = lefthandPos + playerPos;
			rot = lefthandRot;
			
			vel = trackState.HandPoses[0].LinearVelocity;
			gravity = 0;
			angVel = trackState.HandPoses[0].AngularVelocity;
			hold = false;
		}
	}
	else {
		pos += vel * (ovr_GetTimeInSeconds() - prevTime) * Vector3f(-1, 1, -1);
		gravity += 0.4f * (ovr_GetTimeInSeconds() - prevTime);
		pos.y -= gravity * (ovr_GetTimeInSeconds() - prevTime);

		rotOffset += angVel * (ovr_GetTimeInSeconds() - prevTime) * Vector3f(-1, 1, -1);
		rot.Normalize();
		models[1]->MoveToPos(pos);
		models[1]->RotateObj(rot, rotOffset);

		if (inputstate.Buttons == 0) {
			hold = true;
		}
	}
	prevTime = ovr_GetTimeInSeconds();


	for (int i = 0; i < numModels; i++) {
		models[i]->Render(view, proj, playerPos);

	}
}