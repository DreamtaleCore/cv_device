#include <Python.h>

// https://msdn.microsoft.com/en-us/library/windows/desktop/dd377566(v=vs.85).aspx
#include <windows.h>
#include <dshow.h>
#include <comutil.h>
#include <vector>
#include <mfplay.h>
using namespace std;

#pragma comment(lib, "strmiids")

#if PY_MAJOR_VERSION >= 3
#ifndef IS_PY3K
#define IS_PY3K 1
#endif
#endif

struct module_state {
    PyObject *error;
};

#if defined(IS_PY3K)
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

// #pragma comment(lib, "kernel32")
// #pragma comment(lib, "user32")
// #pragma comment(lib, "gdi32")
// #pragma comment(lib, "winspool")
// #pragma comment(lib, "comdlg32")
// #pragma comment(lib, "advapi32")
// #pragma comment(lib, "shell32")
// #pragma comment(lib, "ole32")
// #pragma comment(lib, "oleaut32")
// #pragma comment(lib, "uuid")
// #pragma comment(lib, "odbc32")
// #pragma comment(lib, "odbccp32")
#pragma comment(lib, "comsuppwd.lib")

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

PyObject* DisplayDeviceInformation(IEnumMoniker *pEnum)
{
	// Create an empty Python list
	PyObject* pyList = PyList_New(0); 

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
			// Append a result to Python list
			char *pValue = _com_util::ConvertBSTRToString(var.bstrVal);
			hr = PyList_Append(pyList, Py_BuildValue("s", pValue));
			delete[] pValue;  
			if (FAILED(hr)) {
				printf("Failed to append the object item at the end of list list\n");
				return pyList;
			}

			// printf("%S\n", var.bstrVal);
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
			// Append a result to Python list
			char *pValue = _com_util::ConvertBSTRToString(var.bstrVal);
			hr = PyList_Append(pyList, Py_BuildValue("s", pValue));
			delete[] pValue;  
			if (FAILED(hr)) {
				printf("Failed to append the object item at the end of list list\n");
				return pyList;
			}

			// printf("Device path: %S\n", var.bstrVal);
			VariantClear(&var);
		}

		pPropBag->Release();
		pMoniker->Release();
	}

	return pyList;
}

static PyObject *
getDeviceList(PyObject *self, PyObject *args)
{
	PyObject* pyList = NULL; 
	
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	//backup; run it without multithreading.
	if (!SUCCEEDED(hr))
	{				
		hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	}

	if (SUCCEEDED(hr))
	{
		IEnumMoniker *pEnum;

		hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
		if (SUCCEEDED(hr))
		{
			pyList = DisplayDeviceInformation(pEnum);
			pEnum->Release();
		}
		CoUninitialize();
	} else {
		PyErr_SetString(PyExc_TypeError, "Could not call CoInitializeEx");		
	}

    return pyList;
}

// vector< string > _get_devices() {
//     vector< string > result;
//     DeviceList  g_devices;
//     HRESULT hr = S_OK, hserial = S_OK;

//     WCHAR* szFriendlyName = NULL;
//     g_devices.Clear();

//     hr = g_devices.EnumerateDevices();

//     if (FAILED(hr)) {
//         return result;
//     }

//     for (UINT32 iDevice = 0; iDevice < g_devices.Count(); iDevice++)
//     {

//         hr = g_devices.GetDeviceName(iDevice, &szFriendlyName);
// 		hserial = g_devices.GetDevice(iDevice, )

//         if (FAILED(hr)) { return result; }
//         wstring ws(szFriendlyName);
//         string szFriendlyNameStr(ws.begin(), ws.end());
//         result.push_back(szFriendlyNameStr);
//         CoTaskMemFree(szFriendlyName);
//         szFriendlyName = NULL;
//     }
//     return result;
// }


// PyObject* getDeviceList(PyObject* self, PyObject* args) {
//     vector< string > devices = _get_devices();
//     PyObject* result = PyList_New(devices.size());
//     for (int i = 0; i < devices.size(); ++i) {
//         PyObject* pytmp = Py_BuildValue("s", devices[i].c_str());
//         PyList_SetItem(result, i, pytmp);
//     }
//     return result;
// }

static PyMethodDef Methods[] =
{
    {"getDeviceList", getDeviceList, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initdevice(void);

#if defined(IS_PY3K)

static int device_traverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

static int device_clear(PyObject *m) {
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "cv_device",
    NULL,
    sizeof(struct module_state),
    Methods,
    NULL,
    device_traverse,
    device_clear,
    NULL
};

#define INITERROR return NULL

PyMODINIT_FUNC
PyInit_cv_device(void)

#else
#define INITERROR return
void
initdevice(void)
#endif
{
#if defined(IS_PY3K)
    PyObject *module = PyModule_Create(&moduledef);
#else
    PyObject *module = Py_InitModule("cv_device", Methods);
#endif

    if (module == NULL)
        INITERROR;
    struct module_state *st = GETSTATE(module);

    st->error = PyErr_NewException("dbr.Error", NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }

#if defined(IS_PY3K)
    return module;
#endif
}