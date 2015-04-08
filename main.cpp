/*****************************************************************************
 * DirectShow Pan/Tilt/Zoom sample for Logitech QuickCam devices
 * 
 * Copyright 2007 (c) Logitech. All Rights Reserved.
 *
 * This code and information is provided "as is" without warranty of
 * any kind, either expressed or implied, including but not limited to
 * the implied warranties of merchantability and/or fitness for a
 * particular purpose.
 *
 * Version: 1.1
 ****************************************************************************/

/*****************************************************************************
 * The original code was written by logitech.
 * I added the argument for pan/tilt to main function, 
 * and eliminated some codes (like digital zoom etc...
*****************************************************************************/

#include <dshow.h>
#include <Ks.h>				// Required by KsMedia.h
#include <KsMedia.h>		// For KSPROPERTY_CAMERACONTROL_FLAGS_*


struct ControlInfo {
	long min;
	long max;
	long step;
	long def;
	long flags;
};

/*
*	Grobal Parameter of Pan, Tilt and Moving Time.
*	05,Aug,2011 by 7M4MON
*/
int gPan;	//Relative Angle of Pan (-128 to 128 : QCAM-200R)
int gTilt;	//Relative Angle of Tilt (-54 to 54 : QCAM-200R)
int gTime;	//Initial Wait Time of Pan, Tilt
int gWait;	//Inclination Wait Time of Pan, Tilt

/*
 * Pans the camera by a given angle.
 *
 * The angle is given in degrees, positive values are clockwise rotation (seen from the top),
 * negative values are counter-clockwise rotation. If the "Mirror horizontal" option is
 * enabled, the panning sense is reversed.
 */
HRESULT set_mechanical_pan_relative(IAMCameraControl *pCameraControl, long value)
{
	HRESULT hr = 0;
	long flags = KSPROPERTY_CAMERACONTROL_FLAGS_RELATIVE | KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;

	hr = pCameraControl->Set(CameraControl_Pan, value, flags);
	if(hr != S_OK)
		fprintf(stderr, "ERROR: Unable to set CameraControl_Pan property value to %d. (Error 0x%08X)\n", value, hr);

	// Note that we need to wait until the movement is complete, otherwise the next request will
	// fail with hr == 0x800700AA == HRESULT_FROM_WIN32(ERROR_BUSY).
	Sleep(gTime);

	return hr;
}


/*
 * Tilts the camera by a given angle.
 *
 * The angle is given in degrees, positive values are downwards, negative values are upwards.
 * If the "Mirror vertical" option is enabled, the tilting sense is reversed.
 */
HRESULT set_mechanical_tilt_relative(IAMCameraControl *pCameraControl, long value)
{
	HRESULT hr = 0;
	long flags = KSPROPERTY_CAMERACONTROL_FLAGS_RELATIVE | KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;

	hr = pCameraControl->Set(CameraControl_Tilt, value, flags);
	if(hr != S_OK)
		fprintf(stderr, "ERROR: Unable to set CameraControl_Tilt property value to %d. (Error 0x%08X)\n", value, hr);

	// Note that we need to wait until the movement is complete, otherwise the next request will
	// fail with hr == 0x800700AA == HRESULT_FROM_WIN32(ERROR_BUSY).
	Sleep(gTime);

	return hr;
}


/*
 * Test a camera's pan/tilt properties
 *
 * See also:
 *
 * IAMCameraControl Interface
 *     http://msdn2.microsoft.com/en-us/library/ms783833.aspx
 * PROPSETID_VIDCAP_CAMERACONTROL
 *     http://msdn2.microsoft.com/en-us/library/aa510754.aspx
 */
HRESULT test_pan_tilt(IBaseFilter *pBaseFilter)
{
	HRESULT hr = 0;
	IAMCameraControl *pCameraControl = NULL;
	ControlInfo panInfo = { 0 };
	ControlInfo tiltInfo = { 0 };
	ControlInfo zoomInfo = { 0 };
	long value = 0, flags = 0;

	//printf("    Reading pan/tilt property information ...\n");

	//// Get a pointer to the IAMCameraControl interface used to control the camera
	hr = pBaseFilter->QueryInterface(IID_IAMCameraControl, (void **)&pCameraControl);
	if(hr != S_OK)
	{
		fprintf(stderr, "ERROR: Unable to access IAMCameraControl interface.\n");
		return hr;
	}


	//*
	//printf("    Testing mechanical pan ...\n");
	set_mechanical_pan_relative(pCameraControl, gPan);
	Sleep(abs(gPan)*gWait);
	//*/

	//*
	//printf("    Testing mechanical tilt ...\n");
	set_mechanical_tilt_relative(pCameraControl, gTilt);
	Sleep(abs(gTilt)*gWait);
	//*/
	
	printf("    DONE\n");
	return S_OK;
}


/*
 * Do something with the filter. In this sample we just test the pan/tilt properties.
 */
void process_filter(IBaseFilter *pBaseFilter)
{
	test_pan_tilt(pBaseFilter);
}


/*
 * Enumerate all video devices
 *
 * See also:
 *
 * Using the System Device Enumerator:
 *     http://msdn2.microsoft.com/en-us/library/ms787871.aspx
 */
int enum_devices()
{
	HRESULT hr;

	printf("Enumerating video input devices ...\n");

	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if(FAILED(hr))
	{
		fprintf(stderr, "ERROR: Unable to create system device enumerator.\n");
		return hr;
	}

	// Obtain a class enumerator for the video input device category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if(hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			if(SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					// Display the name in your UI somehow.
					wprintf(L"  Found device: %s\n", varName.bstrVal);
				}
				VariantClear(&varName);

				// To create an instance of the filter, do the following:
				IBaseFilter *pFilter;
				hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
					(void**)&pFilter);
				
				process_filter(pFilter);

				//Remember to release pFilter later.
				pPropBag->Release();

			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();

	return 0;
}


//int wmain(int argc, wchar_t* argv[])
int main(int argc, char **argv)
{
	int result;
	const char* argPan = argc >= 2 ? argv[1] : "180";
	const char* argTilt  = argc >= 3 ? argv[2] : "180";
	const char* argTime = argc >= 4 ? argv[3] : "100";
	const char* argWait = argc >= 5 ? argv[4] : "9";

	
	printf("Trying Pan %s, Tilt %s, Wait %s, Time %s \n",argPan,argTilt,argTime,argWait);

	gPan = atoi(argPan);
	gTilt = atoi(argTilt);
	gTime = atoi(argTime);
	gWait = atoi(argWait);


	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	result = enum_devices();

	CoUninitialize();

	return result;
}
