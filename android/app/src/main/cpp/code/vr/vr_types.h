#ifndef __VR_TYPES
#define __VR_TYPES

#ifdef USE_LOCAL_HEADERS
#	include "SDL_opengl.h"
#	include "SDL_opengles2.h"
#else
#	include <SDL_opengl.h>
#	include <SDL_opengles2.h>
#endif

//OpenXR
#define XR_USE_GRAPHICS_API_OPENGL_ES 1
#define XR_USE_PLATFORM_ANDROID 1
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <jni.h>
#include <math.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#define MATH_PI 3.14159265358979323846f

#include <android/log.h>
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, "OpenXR", __VA_ARGS__);
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "OpenXR", __VA_ARGS__);

typedef union {
    XrCompositionLayerProjection Projection;
    XrCompositionLayerCylinderKHR Cylinder;
} ovrCompositorLayer_Union;

enum { ovrMaxLayerCount = 1 };
enum { ovrMaxNumEyes = 2 };

typedef struct {
	JavaVM* Vm;
	jobject ActivityObject;
	JNIEnv* Env;
} ovrJava;

typedef struct {
    XrSwapchain Handle;
    uint32_t Width;
    uint32_t Height;
} ovrSwapChain;

typedef struct {
    EGLint MajorVersion;
    EGLint MinorVersion;
    EGLDisplay Display;
    EGLConfig Config;
    EGLSurface TinySurface;
    EGLSurface MainSurface;
    EGLContext Context;
} ovrEgl;

typedef struct {
    int Width;
    int Height;
    uint32_t TextureSwapChainLength;
    uint32_t TextureSwapChainIndex;
    ovrSwapChain ColorSwapChain;
    XrSwapchainImageOpenGLESKHR* ColorSwapChainImage;
    GLuint* DepthBuffers;
    GLuint* FrameBuffers;
} ovrFramebuffer;

typedef struct {
    ovrFramebuffer FrameBuffer;
} ovrRenderer;

typedef struct {
    GLboolean Active;
    XrPosef Pose;
} ovrTrackedController;

typedef struct {
    ovrEgl Egl;
    GLboolean Focused;

    XrInstance Instance;
    XrSession Session;
    XrViewConfigurationProperties ViewportConfig;
    XrViewConfigurationView ViewConfigurationView[ovrMaxNumEyes];
    XrSystemId SystemId;
    XrSpace HeadSpace;
    XrSpace StageSpace;
    XrSpace FakeStageSpace;
    XrSpace CurrentSpace;
    GLboolean SessionActive;

    int SwapInterval;
    // These threads will be marked as performance threads.
    int MainThreadTid;
    int RenderThreadTid;
    ovrCompositorLayer_Union Layers[ovrMaxLayerCount];
    int LayerCount;

    GLboolean TouchPadDownLastFrame;
    ovrRenderer Renderer;
    ovrTrackedController TrackedController[2];
} ovrApp;


typedef struct {
    float M[4][4];
} ovrMatrix4f;

typedef enum ovrButton_ {
    ovrButton_A = 0x00000001, // Set for trigger pulled on the Gear VR and Go Controllers
    ovrButton_B = 0x00000002,
    ovrButton_RThumb = 0x00000004,
    ovrButton_RShoulder = 0x00000008,

    ovrButton_X = 0x00000100,
    ovrButton_Y = 0x00000200,
    ovrButton_LThumb = 0x00000400,
    ovrButton_LShoulder = 0x00000800,

    ovrButton_Up = 0x00010000,
    ovrButton_Down = 0x00020000,
    ovrButton_Left = 0x00040000,
    ovrButton_Right = 0x00080000,
    ovrButton_Enter = 0x00100000, //< Set for touchpad click on the Go Controller, menu
    // button on Left Quest Controller
    ovrButton_Back = 0x00200000, //< Back button on the Go Controller (only set when
    // a short press comes up)
    ovrButton_GripTrigger = 0x04000000, //< grip trigger engaged
    ovrButton_Trigger = 0x20000000, //< Index Trigger engaged
    ovrButton_Joystick = 0x80000000, //< Click of the Joystick

    ovrButton_EnumSize = 0x7fffffff
} ovrButton;

typedef struct {
	uint64_t frameIndex;
	ovrApp appState;
	ovrJava java;
	float predictedDisplayTime;
} engine_t;

typedef enum {
	WS_CONTROLLER,
	WS_HMD,
	WS_ALTKEY,
	WS_PREVNEXT
} weaponSelectorType_t;

typedef enum {
    VRFM_THIRDPERSON_1,		//Camera will auto move to keep up with player
	VRFM_THIRDPERSON_2,		//Camera is completely free movement with the thumbstick
    VRFM_FIRSTPERSON,		//Obvious isn't it?..
    VRFM_NUM_FOLLOWMODES,

	VRFM_QUERY		= 99	//Used to query which mode is active
} vrFollowMode_t;

void ovrApp_Clear(ovrApp* app);
void ovrApp_Destroy(ovrApp* app);
GLboolean ovrApp_HandleXrEvents(ovrApp* app);

void ovrEgl_CreateContext(ovrEgl* egl, const ovrEgl* shareEgl);
void ovrEgl_DestroyContext(ovrEgl* egl);

void ovrFramebuffer_Acquire(ovrFramebuffer* frameBuffer);
void ovrFramebuffer_Resolve(ovrFramebuffer* frameBuffer);
void ovrFramebuffer_Release(ovrFramebuffer* frameBuffer);
void ovrFramebuffer_SetCurrent(ovrFramebuffer* frameBuffer);
void ovrFramebuffer_SetNone();

void ovrRenderer_Create(
		XrSession session,
		ovrRenderer* renderer,
		int suggestedEyeTextureWidth,
		int suggestedEyeTextureHeight);
void ovrRenderer_Destroy(ovrRenderer* renderer);

void ovrTrackedController_Clear(ovrTrackedController* controller);

ovrMatrix4f ovrMatrix4f_Multiply(const ovrMatrix4f* a, const ovrMatrix4f* b);
ovrMatrix4f ovrMatrix4f_CreateRotation(const float radiansX, const float radiansY, const float radiansZ);
ovrMatrix4f ovrMatrix4f_CreateFromQuaternion(const XrQuaternionf* q);
ovrMatrix4f ovrMatrix4f_CreateProjectionFov(
		const float fovDegreesX,
		const float fovDegreesY,
		const float offsetX,
		const float offsetY,
		const float nearZ,
		const float farZ);

XrVector4f XrVector4f_MultiplyMatrix4f(const ovrMatrix4f* a, const XrVector4f* v);


/// THESE METHODS HAVE ORIGIN IN openxr_oculus_helpers.h

static inline double FromXrTime(const XrTime time) {
	return (time * 1e-9);
}

static inline XrTime ToXrTime(const double timeInSeconds) {
	return (timeInSeconds * 1e9);
}

static inline XrPosef XrPosef_Identity() {
	XrPosef r;
	r.orientation.x = 0;
	r.orientation.y = 0;
	r.orientation.z = 0;
	r.orientation.w = 1;
	r.position.x = 0;
	r.position.y = 0;
	r.position.z = 0;
	return r;
}

static inline float XrVector3f_LengthSquared(const XrVector3f v) {
	return v.x * v.x + v.y * v.y + v.z * v.z;;
}

static inline float XrVector3f_Length(const XrVector3f v) {
	return sqrtf(XrVector3f_LengthSquared(v));
}

static inline XrVector3f XrVector3f_ScalarMultiply(const XrVector3f v, float scale) {
	XrVector3f u;
	u.x = v.x * scale;
	u.y = v.y * scale;
	u.z = v.z * scale;
	return u;
}

static inline XrVector3f XrVector3f_Normalized(const XrVector3f v) {
	float rcpLen = 1.0f / XrVector3f_Length(v);
	return XrVector3f_ScalarMultiply(v, rcpLen);
}

static inline XrQuaternionf XrQuaternionf_CreateFromVectorAngle(
		const XrVector3f axis,
		const float angle) {
	XrQuaternionf r;
	if (XrVector3f_LengthSquared(axis) == 0.0f) {
		r.x = 0;
		r.y = 0;
		r.z = 0;
		r.w = 1;
		return r;
	}

	XrVector3f unitAxis = XrVector3f_Normalized(axis);
	float sinHalfAngle = sinf(angle * 0.5f);

	r.w = cosf(angle * 0.5f);
	r.x = unitAxis.x * sinHalfAngle;
	r.y = unitAxis.y * sinHalfAngle;
	r.z = unitAxis.z * sinHalfAngle;
	return r;
}

static inline XrQuaternionf XrQuaternionf_Multiply(const XrQuaternionf a, const XrQuaternionf b) {
	XrQuaternionf c;
	c.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	c.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
	c.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
	c.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
	return c;
}

static inline XrQuaternionf XrQuaternionf_Inverse(const XrQuaternionf q) {
	XrQuaternionf r;
	r.x = -q.x;
	r.y = -q.y;
	r.z = -q.z;
	r.w = q.w;
	return r;
}

static inline XrVector3f XrQuaternionf_Rotate(const XrQuaternionf a, const XrVector3f v) {
	XrVector3f r;
	XrQuaternionf q = {v.x, v.y, v.z, 0.0f};
	XrQuaternionf aq = XrQuaternionf_Multiply(a, q);
	XrQuaternionf aInv = XrQuaternionf_Inverse(a);
	XrQuaternionf aqaInv = XrQuaternionf_Multiply(aq, aInv);
	r.x = aqaInv.x;
	r.y = aqaInv.y;
	r.z = aqaInv.z;
	return r;
}

static inline XrVector3f XrVector3f_Add(const XrVector3f u, const XrVector3f v) {
	XrVector3f w;
	w.x = u.x + v.x;
	w.y = u.y + v.y;
	w.z = u.z + v.z;
	return w;
}

static inline XrVector3f XrPosef_Transform(const XrPosef a, const XrVector3f v) {
	XrVector3f r0 = XrQuaternionf_Rotate(a.orientation, v);
	return XrVector3f_Add(r0, a.position);
}

static inline XrPosef XrPosef_Multiply(const XrPosef a, const XrPosef b) {
	XrPosef c;
	c.orientation = XrQuaternionf_Multiply(a.orientation, b.orientation);
	c.position = XrPosef_Transform(a, b.position);
	return c;
}

int Pxr_SetEngineVersion(const char *version);

int Pxr_StartCVControllerThread(int headSensorState, int handSensorState);

int Pxr_StopCVControllerThread(int headSensorState, int handSensorState);

void InitializeGraphicDeivce(XrInstance mInstance);

#endif
