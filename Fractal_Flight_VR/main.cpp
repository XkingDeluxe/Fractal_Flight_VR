#include "OVR_CAPI_GL.h"
#include "src/OGL.h"
#include "src/Scene.h"
#include "src/Player.h"
#include "src/OculusTexBuffer.h"
#if defined(_WIN32)
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")
#endif

static OGL Platform;

static ovrGraphicsLuid GetDefaultAdapterLuid()
{
	ovrGraphicsLuid luid = ovrGraphicsLuid();

#if defined(_WIN32)
	IDXGIFactory* factory = nullptr;

	if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory))))
	{
		IDXGIAdapter* adapter = nullptr;

		if (SUCCEEDED(factory->EnumAdapters(0, &adapter)))
		{
			DXGI_ADAPTER_DESC desc;

			adapter->GetDesc(&desc);
			memcpy(&luid, &desc.AdapterLuid, sizeof(luid));
			adapter->Release();
		}

		factory->Release();
	}
#endif

	return luid;
}

static int Compare(const ovrGraphicsLuid& lhs, const ovrGraphicsLuid& rhs)
{
	return memcmp(&lhs, &rhs, sizeof(ovrGraphicsLuid));
}

static bool MainLoop(bool retryCreate) {
	Player* player = nullptr;

	ovrMirrorTexture mirrorTexture = nullptr;
	GLuint          mirrorFBO = 0;
	Scene* scene = nullptr;
	long long frameIndex = 0;

	ovrSession session;

	ovrGraphicsLuid luid;
	ovrResult result = ovr_Create(&session, &luid);

	if (!OVR_SUCCESS(result))
		return retryCreate;

	

	ovrSizei windowSize = { ovr_GetHmdDesc(session).Resolution.w / 2, ovr_GetHmdDesc(session).Resolution.h / 2 };
	if (!Platform.InitDevice(windowSize.w, windowSize.h, reinterpret_cast<LUID*>(&luid))) {
		goto Done;
	}

	if (Compare(luid, GetDefaultAdapterLuid())) // If luid that the Rift is on is not the default adapter LUID...
	{
		VALIDATE(false, "OpenGL supports only the default graphics adapter.");
	}

	player = new Player(session);

	ovrMirrorTextureDesc desc;
	memset(&desc, 0, sizeof(desc));
	desc.Width = windowSize.w;
	desc.Height = windowSize.h;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

	result = ovr_CreateMirrorTextureWithOptionsGL(session, &desc, &mirrorTexture);
	if (!OVR_SUCCESS(result))
	{
		if (!retryCreate) goto Done;
		VALIDATE(false, "Failed to create mirror texture.");
	}

	GLuint texId;
	ovr_GetMirrorTextureBufferGL(session, mirrorTexture, &texId);


	glGenFramebuffers(1, &mirrorFBO);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
	glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);


	// Turn off vsync
	wglSwapIntervalEXT(0);

	scene = new Scene(session);

	ovr_SetTrackingOriginType(session, ovrTrackingOrigin_FloorLevel);

	while (Platform.HandleMessages()) {
		ovrSessionStatus sessionStatus;
		ovr_GetSessionStatus(session, &sessionStatus);
		if (sessionStatus.ShouldQuit)
		{
			// Because the application is requested to quit, should not request retry
			retryCreate = false;
			break;
		}
		if (sessionStatus.ShouldRecenter)
			ovr_RecenterTrackingOrigin(session);

		if (sessionStatus.IsVisible)
		{
			player->Update(frameIndex, scene);

			ovrLayerEyeFovDepth ld = {};
			ld.Header.Type = ovrLayerType_EyeFovDepth;
			ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.
			ld.ProjectionDesc = player->getTWPD();
			ld.SensorSampleTime = player->getSensorSampleTime();
			for (int eye = 0; eye < 2; ++eye)
			{
				ld.ColorTexture[eye] = player->eyes[eye]->eyeRenderTexture->ColorTextureChain;
				ld.DepthTexture[eye] = player->eyes[eye]->eyeRenderTexture->DepthTextureChain;
				ld.Viewport[eye] = Recti(player->eyes[eye]->eyeRenderTexture->GetSize());
				ld.Fov[eye] = player->eyes[eye]->hmdDesc.DefaultEyeFov[eye];
				ld.RenderPose[eye] = player->eyes[eye]->eyeRenderPose;
			}
			ovrLayerHeader* layers = &ld.Header;
			result = ovr_SubmitFrame(session, frameIndex, nullptr, &layers, 1);
			// exit the rendering loop if submit returns an error, will retry on ovrError_DisplayLost
			if (!OVR_SUCCESS(result))
				goto Done;
			frameIndex++;
		}

		// Blit mirror texture to back buffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		GLint w = windowSize.w;
		GLint h = windowSize.h;
		glBlitFramebuffer(0, h, w, 0,
			0, 0, w, h,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		SwapBuffers(Platform.getHDC());
	}


Done:
	delete scene;
	if (mirrorFBO) glDeleteFramebuffers(1, &mirrorFBO);
	if (mirrorTexture) ovr_DestroyMirrorTexture(session, mirrorTexture);
	delete player->eyes[0];
	delete player->eyes[1];
	Platform.ReleaseDevice();
	ovr_Destroy(session);

	// Retry on ovrError_DisplayLost
	return retryCreate || (result == ovrError_DisplayLost);
}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE, LPSTR, int){
	AllocConsole();//					| 
	freopen("CONIN$", "r", stdin);//	| Open console
	freopen("CONOUT$", "w", stdout);//	|
	freopen("CONOUT$", "w", stderr);//	|
	ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
	ovrResult result = ovr_Initialize(&initParams);

	VALIDATE(OVR_SUCCESS(result), "Failed to init libOVR");

	VALIDATE(Platform.InitWindow(hinst, L"Fractal Flight VR"), "Failed to open window");

	Platform.Run(MainLoop);

	ovr_Shutdown();
	
	return 0;
}