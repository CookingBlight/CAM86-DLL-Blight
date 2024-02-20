// Calling convention is cdecl with ANSI charset.

#pragma once

// DLL export macrom for the cam86 driver.
#ifdef CAM86_DLL_EXPORT
	#define CAM86_DLL_DIR extern "C" __declspec(dllexport)
#else
	#define CAM86_DLL_DIR extern "C" __declspec(dllimport)
#endif

enum {
	kCamera_Idle = 0,
	kCamera_Waiting = 1,
	kCamera_Exposing = 2,
	kCamera_Reading = 3,
	kCamera_Download = 4,
	kCamera_Error = 5
};

CAM86_DLL_DIR BOOL cameraSetGain(int val); // Always returns true.
CAM86_DLL_DIR BOOL cameraSetOffset(int val); // Always returns true.
CAM86_DLL_DIR BOOL cameraConnect(); // Returns true if connected.
CAM86_DLL_DIR BOOL cameraDisconnect(); // Returns true if disconnected.
CAM86_DLL_DIR BOOL cameraIsConnected(); // Returns true if connected.
CAM86_DLL_DIR BOOL cameraStartExposure(int bin, int start_x, int start_y, int num_x, int num_y, double duration, BOOL light); // Always returns true.
CAM86_DLL_DIR BOOL cameraStopExposure(); // Always returns true.
CAM86_DLL_DIR int cameraGetCameraState(); // Returns camera state.
CAM86_DLL_DIR BOOL cameraGetImageReady(); // Returns true if the image is ready.
CAM86_DLL_DIR int* cameraGetImage(); // Returns a pointer to a int array of size 3000*2000.
CAM86_DLL_DIR int cameraGetError(); // Returns 0 of no errors have occured.
CAM86_DLL_DIR double cameraGetTemp(); // Returns temperature in C.
CAM86_DLL_DIR BOOL cameraSetTemp(double temp); // Always returns true.
CAM86_DLL_DIR double cameraGetSetTemp(); // Returns set temperature in C.
CAM86_DLL_DIR BOOL cameraCoolingOn(); // Always returns true.
CAM86_DLL_DIR BOOL cameraCoolingOff(); // Always returns true.
CAM86_DLL_DIR BOOL cameraGetCoolerOn(); // Returns true if cooler is on.
CAM86_DLL_DIR double cameraGetCoolerPower(); // Returns cooler power.
CAM86_DLL_DIR BOOL cameraSetReadingTime(int val); // Set delay per line in 0.1ms.
CAM86_DLL_DIR BYTE cameraGetFirmwareVersion();
CAM86_DLL_DIR BOOL cameraSetCoolerDuringReading(BYTE val); // Always returns true.
CAM86_DLL_DIR BYTE cameraGetLLDriverVersion();
CAM86_DLL_DIR BOOL cameraSetBiasBeforeExposure(BYTE val); // Always returns true.
CAM86_DLL_DIR double cameraGetTempDHT(); // Returns DHT temperature in C.
CAM86_DLL_DIR double cameraGetHumidityDHT();
CAM86_DLL_DIR BOOL cameraSetCoolingStartingPowerPercentage(int val); // Always returns true.
CAM86_DLL_DIR BOOL cameraSetCoolingMaximumPowerPercentage(int val); // Always returns true.
CAM86_DLL_DIR int cameraGetCoolingStartingPowerPercentage();
CAM86_DLL_DIR int cameraGetCoolingMaximumPowerPercentage();
CAM86_DLL_DIR BOOL cameraSetPIDproportionalGain(double val); // Always returns true.
CAM86_DLL_DIR BOOL cameraSetPIDintegralGain(double val); // Always returns true.
CAM86_DLL_DIR BOOL cameraSetPIDderivativeGain(double val); // Always returns true.
CAM86_DLL_DIR BOOL cameraGetPIDGainLow(BYTE cmd);
CAM86_DLL_DIR BOOL cameraGetPIDGainHigh(BYTE cmd);
CAM86_DLL_DIR double cameraGetPIDGain(double cache_var, BYTE cmd_low, BYTE cmd_high); // Returns 4 bytes cast as a double.
CAM86_DLL_DIR double cameraGetPIDproportionalGain(); // Return P.
CAM86_DLL_DIR double cameraGetPIDintegralGain(); // Return I.
CAM86_DLL_DIR double cameraGetPIDderivativeGain(); // Return D.