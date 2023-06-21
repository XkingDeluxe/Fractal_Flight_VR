#include "OGL.h"


//Handle keys
LRESULT CALLBACK OGL::WindowProc(_In_ HWND hWnd, _In_ UINT msg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	//Point to self
	OGL* self = reinterpret_cast<OGL*>(GetWindowLongPtr(hWnd, 0));

	switch (msg) {
	case WM_KEYDOWN:
		self->keys[wParam] = true;
		break;
	case WM_KEYUP:
		self->keys[wParam] = false;
		break;
	case WM_DESTROY:
		self->running = false;
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	if (self->keys['Q'] && self->keys[VK_ESCAPE]) {
		self->running = false;
	}
	return 0;
}

OGL::OGL() : Window(nullptr), hDC(nullptr), WglContext(nullptr), running(false), winWidth(0), winHeight(0), fboId(0), hInstance(nullptr) {
	for (int i = 0; i < 256; i++) {
		keys[i] = false;
	}
}

OGL::~OGL() {
	ReleaseDevice();
	CloseWindow();
}

bool OGL::InitWindow(HINSTANCE hInst, LPCWSTR name) {
	hInstance = hInst;
	running = true;

	WNDCLASSW wc;
	memset(&wc, 0, sizeof(wc));
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = WindowProc;
	wc.cbWndExtra = sizeof(struct OGL*);
	wc.hInstance = GetModuleHandleW(NULL);
	wc.lpszClassName = L"ORT";
	RegisterClassW(&wc);

	Window = CreateWindowW(wc.lpszClassName, name, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, 0, 0, hInstance, 0);
	if (!Window) return false;

	SetWindowLongPtr(Window, 0, LONG_PTR(this));

	hDC = GetDC(Window);

	return true;
}

void OGL::CloseWindow(){
	if (Window) {
		if (hDC) {
			ReleaseDC(Window, hDC);
			hDC = nullptr;
		}
		DestroyWindow(Window);
		Window = nullptr;
		UnregisterClassW(L"OGL", hInstance);
	}
}

bool OGL::InitDevice(int vpW, int vpH, const LUID*, bool windowed) {
	UNREFERENCED_PARAMETER(windowed);

	winWidth = vpW;
	winHeight = vpH;

	RECT size = { 0, 0, vpW, vpH };
	AdjustWindowRect(&size, WS_OVERLAPPEDWINDOW, false);
	const UINT flags = SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW;
	if (!SetWindowPos(Window, nullptr, 0, 0, size.right - size.left, size.bottom - size.top, flags))
		return false;

	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARBFunc = nullptr;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARBFunc = nullptr;
	{
		// First create a context for the purpose of getting access to wglChoosePixelFormatARB / wglCreateContextAttribsARB.
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 16;
		int pf = ChoosePixelFormat(hDC, &pfd);
		VALIDATE(pf, "Failed to choose pixel format.");

		VALIDATE(SetPixelFormat(hDC, pf, &pfd), "Failed to set pixel format.");

		HGLRC context = wglCreateContext(hDC);
		VALIDATE(context, "wglCreateContextfailed.");
		VALIDATE(wglMakeCurrent(hDC, context), "wglMakeCurrent failed.");

		wglChoosePixelFormatARBFunc = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
		wglCreateContextAttribsARBFunc = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
		
		assert(wglChoosePixelFormatARBFunc && wglCreateContextAttribsARBFunc);

		wglDeleteContext(context);
	}

	int iAttributes[] =
	{
		// WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 16,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
		0, 0
	};

	float fAttributes[] = { 0, 0 };
	int   pf = 0;
	UINT  numFormats = 0;

	VALIDATE(wglChoosePixelFormatARBFunc(hDC, iAttributes, fAttributes, 1, &pf, &numFormats),
		"wglChoosePixelFormatARBFunc failed.");

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));
	VALIDATE(SetPixelFormat(hDC, pf, &pfd), "SetPixelFormat failed.");

	GLint attribs[16];
	int   attribCount = 0;

	attribs[attribCount] = 0;

	WglContext = wglCreateContextAttribsARBFunc(hDC, 0, attribs);
	VALIDATE(wglMakeCurrent(hDC, WglContext), "wglMakeCurrent failed.");
	
	OVR::GLEContext::SetCurrentContext(&GLEContext);
	GLEContext.Init();


	//opengl init:	

	glGenFramebuffers(1, &fboId);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

void OGL::ReleaseDevice()
{
	if (fboId)
	{
		glDeleteFramebuffers(1, &fboId);
		fboId = 0;
	}
	if (WglContext)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(WglContext);
		WglContext = nullptr;
	}
	GLEContext.Shutdown();
}

bool OGL::HandleMessages(void) {
	MSG msg;
	while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return running;
}

void OGL::Run(bool (*MainLoop)(bool retryCreate)) {
	while (HandleMessages())
	{
		if (!MainLoop(true))
			break;
		Sleep(10); //Sleep for no CPU error
	}
}

void GLAPIENTRY OGL::DebugGLCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	UNREFERENCED_PARAMETER(source);
	UNREFERENCED_PARAMETER(type);
	UNREFERENCED_PARAMETER(id);
	UNREFERENCED_PARAMETER(severity);
	UNREFERENCED_PARAMETER(length);
	UNREFERENCED_PARAMETER(message);
	UNREFERENCED_PARAMETER(userParam);
	OVR_DEBUG_LOG(("Message from OpenGL: %s\n", message));
}


