
#ifndef autd3capih_
#define autd3capih_


extern "C" {
    typedef void *AUTDControllerHandle;
	typedef void *AUTDGainHandle;
    __declspec(dllexport) int AUTDOpenController(const char* location, AUTDControllerHandle *out);
	__declspec(dllexport) void AUTDFreeController(AUTDControllerHandle handle);
	__declspec(dllexport) void AUTDAddDevice(AUTDControllerHandle handle, float x, float y, float z, float rz1, float ry, float rz2);
	__declspec(dllexport) int AUTDSetGain(AUTDControllerHandle handle, int deviceIndex, int transIndex, int amp, int phase);
	__declspec(dllexport) int AUTDSetFocalpoint(AUTDControllerHandle handle, float x, float y, float z, int amp);
}

#endif