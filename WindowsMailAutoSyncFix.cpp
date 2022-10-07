#include <ShObjIdl.h>
#include <wrl.h>
#include <appmodel.h>

const wchar_t* PACKAGE_FAMILY_NAME = L"microsoft.windowscommunicationsapps_8wekyb3d8bbwe";
const wchar_t* APP_USER_MODEL_ID = L"microsoft.windowscommunicationsapps_8wekyb3d8bbwe!microsoft.windowslive.mail";

int main() {

	auto hr = CoInitialize(NULL);
	if (hr != S_OK) {
		return hr;
	}

	// Get package full name
	UINT32 count = 0;
	UINT32 buffer_length = 0;
	hr = GetPackagesByPackageFamily(PACKAGE_FAMILY_NAME, &count, NULL, &buffer_length, NULL);
	if (count < 1 || hr != ERROR_INSUFFICIENT_BUFFER) {
		return hr;
	}
	auto full_names = new PWSTR[count];
	auto buffer = new WCHAR[buffer_length];
	hr = GetPackagesByPackageFamily(PACKAGE_FAMILY_NAME, &count, full_names, &buffer_length, buffer);
	if (hr != S_OK) {
		return hr;
	}
	auto package_full_name = full_names[0];

	// Enable debug
	Microsoft::WRL::ComPtr<IPackageDebugSettings> package_debug_settings;
	hr = CoCreateInstance(
		CLSID_PackageDebugSettings,
		NULL,
		CLSCTX_ALL,
		IID_IPackageDebugSettings,
		&package_debug_settings
	);
	if (hr != S_OK) {
		return hr;
	}

	hr = package_debug_settings->EnableDebugging(package_full_name, NULL, NULL);
	if (hr != S_OK) {
		return hr;
	}

	// Prelaunch once terminated
	Microsoft::WRL::ComPtr<IApplicationActivationManager> app_activation_manager;
	hr = CoCreateInstance(
		CLSID_ApplicationActivationManager,
		NULL,
		CLSCTX_ALL,
		IID_IApplicationActivationManager,
		&app_activation_manager
	);
	if (hr != S_OK) {
		return hr;
	}

	while (true) {
		DWORD procces_id;
		hr = app_activation_manager->ActivateApplication(APP_USER_MODEL_ID, NULL, AO_PRELAUNCH, &procces_id);
		if (hr != S_OK) {
			return hr;
		}

		HANDLE handle = OpenProcess(SYNCHRONIZE, FALSE, procces_id);
		auto status = WaitForSingleObject(handle, INFINITE);
		if (status != WAIT_OBJECT_0) {
			Sleep(1000); // Prevent infinite loop consume too much cpu
		}
		CloseHandle(handle);
	}

}
