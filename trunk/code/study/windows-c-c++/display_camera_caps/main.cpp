#include <windows.h>
#include <dshow.h>

#pragma comment(lib, "strmiids")
#  define G_STRFUNC     ((const char*) (__FUNCTION__))
HRESULT EnumerateDevices(REFGUID category, IEnumMoniker **ppEnum)
{
	// Create the System Device Enumerator.
	ICreateDevEnum *pDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the category.
		hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
		if (hr == S_FALSE)
		{
			hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
		}
		pDevEnum->Release();
	}
	return hr;
}


void DisplayDeviceCapsInfomation(IBaseFilter *filter)
{

	IEnumPins * enumpins = NULL;
	IPin * pin = NULL;
	HRESULT hr;

	//hr = IBaseFilter_EnumPins(filter, &enumpins);
	hr = filter->EnumPins(&enumpins);
	if (FAILED(hr))
	{
		printf("%s: IEnumPins::EnumPins failed\n", G_STRFUNC);
		return ;
	}

	//while (IEnumPins_Next(enumpins, 1, &pin, NULL) == S_OK)
	while (enumpins->Next(1,&pin, NULL) == S_OK)
	{
		GUID pin_category;
		DWORD cbReturned;
		IKsPropertySet * ks_prop_set = NULL;
		IAMStreamConfig * stream_config = NULL;

		//IPin_QueryInterface(pin, &IID_IKsPropertySet, &ks_prop_set);
		pin->QueryInterface(IID_IKsPropertySet, (void **)&ks_prop_set);
		if (!ks_prop_set)
		{
			//IPin_Release(pin);
			pin->Release();
			continue;
		}
		//IPin_QueryInterface(pin, &IID_IAMStreamConfig, &stream_config);
		pin->QueryInterface(IID_IAMStreamConfig, (void **)&stream_config);
		if (!stream_config)
		{
			//IKsPropertySet_Release(ks_prop_set);
			ks_prop_set->Release();
			//IPin_Release(pin);
			pin->Release();
			continue;
		}

		//hr = IKsPropertySet_Get(ks_prop_set, &AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0, &pin_category, sizeof(GUID), &cbReturned);
		hr = ks_prop_set->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0, &pin_category, sizeof(GUID), &cbReturned);

		//IKsPropertySet_Release(ks_prop_set);
		ks_prop_set->Release();
	
		if (SUCCEEDED(hr) && IsEqualGUID(pin_category, PIN_CATEGORY_CAPTURE))
		{
			int count, size;

			//hr = IAMStreamConfig_GetNumberOfCapabilities(stream_config, &count, &size);
			hr = stream_config->GetNumberOfCapabilities(&count, &size);
			if (SUCCEEDED(hr) && size == sizeof(VIDEO_STREAM_CONFIG_CAPS))
			{
				int i;
				
				for (i = 0; i < count; i++)
				{
					AM_MEDIA_TYPE * mediatype = NULL;
					VIDEO_STREAM_CONFIG_CAPS vscc;
					int min_w, min_h, max_w, max_h, MinFrameInterval, MaxFrameInterval;
					//hr = IAMStreamConfig_GetStreamCaps(stream_config, i, &entry->mediatype, (LPBYTE)&entry->vscc);
					hr = stream_config->GetStreamCaps(i, &mediatype, (BYTE *)&vscc);
					if (SUCCEEDED(hr))
					{
						//printf("%s: IAMStreamConfig::GetNumberOfCapabilities failed\n", G_STRFUNC);
						min_w = vscc.MinOutputSize.cx;
						max_w = vscc.MaxOutputSize.cx;
						min_h = vscc.MinOutputSize.cy;
						max_h = vscc.MaxOutputSize.cy;
						MaxFrameInterval = (int)vscc.MaxFrameInterval;
						MinFrameInterval = (int)vscc.MinFrameInterval;
						printf("Caps: AM_MEDIA_TYPE is %x width is [%d - %d], height is [%d - %d], framerate is [%d - %d]\n", mediatype->subtype.Data1, min_w, max_w, min_h, max_h, MinFrameInterval, MaxFrameInterval);
					}
					else 
					{
						printf("%s: IAMStreamConfig::GetStreamCaps failed\n", G_STRFUNC);
					}
				}
			}
			else {
				printf("%s: IAMStreamConfig::GetNumberOfCapabilities failed\n", G_STRFUNC);
			}
		}
	}


}


void DisplayDeviceInformation(IEnumMoniker *pEnum)
{
	IMoniker *pMoniker = NULL;

	while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
	{
		IPropertyBag *pPropBag;
		HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
		if (FAILED(hr))
		{
			pMoniker->Release();
			continue;
		}

		VARIANT var;
		VariantInit(&var);

		// Get description or friendly name.
		hr = pPropBag->Read(L"Description", &var, 0);
		if (FAILED(hr))
		{
			hr = pPropBag->Read(L"FriendlyName", &var, 0);
		}
		if (SUCCEEDED(hr))
		{
			printf("%S\n", var.bstrVal);
			VariantClear(&var);
		}

		hr = pPropBag->Write(L"FriendlyName", &var);

		// WaveInID applies only to audio capture devices.
		hr = pPropBag->Read(L"WaveInID", &var, 0);
		if (SUCCEEDED(hr))
		{
			printf("WaveIn ID: %d\n", var.lVal);
			VariantClear(&var);
		}

		hr = pPropBag->Read(L"DevicePath", &var, 0);
		if (SUCCEEDED(hr))
		{
			// The device path is not intended for display.
			printf("Device path: %S\n", var.bstrVal);
			VariantClear(&var);
		}

		IBaseFilter * video_cap_filter = NULL;
		IBindCtx *lpbc = NULL;
		
		hr = CreateBindCtx(0, &lpbc);

		if (SUCCEEDED(hr)) {
			hr = pMoniker->BindToObject(lpbc, NULL, IID_IBaseFilter, (void **)&video_cap_filter);
			//hr = IMoniker_BindToObject(videom, lpbc, NULL, &IID_IBaseFilter, (void **)filter);
		}
		
		if (FAILED(hr))
		{
			return;
		}


		DisplayDeviceCapsInfomation(video_cap_filter);



		if (NULL != lpbc)
		{
			lpbc->Release();
		}
		
		if (NULL != video_cap_filter) video_cap_filter->Release();
		if (NULL != pPropBag)pPropBag->Release();
		if (NULL != pMoniker)pMoniker->Release();


		//GUID pin_category;
		//DWORD cbReturned;
		//IKsPropertySet * ks_prop_set = NULL;
		//IAMStreamConfig * stream_config = NULL;

		//IPin_QueryInterface(pin, &IID_IKsPropertySet, &ks_prop_set);
		//if (!ks_prop_set)
		//{
		//	IPin_Release(pin);
		//	continue;
		//}
		//IPin_QueryInterface(pin, &IID_IAMStreamConfig, &stream_config);
		//if (!stream_config)
		//{
		//	IKsPropertySet_Release(ks_prop_set);
		//	IPin_Release(pin);
		//	continue;
		//}

		//hr = IKsPropertySet_Get(ks_prop_set, &AMPROPSETID_Pin,
		//	AMPROPERTY_PIN_CATEGORY, NULL, 0,
		//	&pin_category, sizeof(GUID), &cbReturned);
		//IKsPropertySet_Release(ks_prop_set);
	}
}

void main()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (SUCCEEDED(hr))
	{
		IEnumMoniker *pEnum;

		hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
		if (SUCCEEDED(hr))
		{
			DisplayDeviceInformation(pEnum);
			pEnum->Release();
		}
		hr = EnumerateDevices(CLSID_AudioInputDeviceCategory, &pEnum);
		if (SUCCEEDED(hr))
		{
			DisplayDeviceInformation(pEnum);
			pEnum->Release();
		}
		CoUninitialize();
	}
}
