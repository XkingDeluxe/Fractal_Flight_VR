#pragma once
#include "GL/CAPI_GLE.h"
#include <assert.h>

#ifndef VALIDATE
#define VALIDATE(x, msg) if (!x) {MessageBoxA(NULL, msg, "Fractal_Flight_VR", MB_ICONERROR | MB_OK); exit(-1);}
#endif

#ifndef OVR_DEBUG_LOG
#define OVR_DEBUG_LOG(x)
#endif
class OGL {
public:
	static LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

	OGL();
	~OGL();

	bool InitWindow(HINSTANCE hInst, LPCWSTR name);
	void CloseWindow();
	

	bool InitDevice(int vpW, int vpH, const LUID*, bool windowed = true);
	void ReleaseDevice();

	bool HandleMessages(void);

	void Run(bool (*MainLoop)(bool retryCreate));

	static void GLAPIENTRY DebugGLCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

	bool* get_keys(void) { return keys; }
	int get_winWidth(void) { return winWidth; }
	HDC getHDC(void) { return hDC; }


private:
	HWND Window;
	HDC hDC;
	HGLRC WglContext;
	OVR::GLEContext GLEContext;

	bool running;
	bool keys[256];
	int winWidth;
	int winHeight;
	GLuint fboId;
	HINSTANCE hInstance;

	
};

