#include <ShObjIdl.h>
#include <wrl.h>
#include <appmodel.h>

const wchar_t* PACKAGE_FAMILY_NAME = L"microsoft.windowscommunicationsapps_8wekyb3d8bbwe";

int main() {

	auto hr = CoInitialize(NULL);
	if (!SUCCEEDED(hr)) {
		return hr;
	}

	// Get package full name
	UINT32 count = 0;
	UINT32 buffer_length = 0;
	hr = GetPackagesByPackageFamily(PACKAGE_FAMILY_NAME, &count, NULL, &buffer_length, NULL);
	if (count < 1 || !SUCCEEDED(hr) && hr != ERROR_INSUFFICIENT_BUFFER) {
		return hr;
	}
	auto full_names = new PWSTR[count];
	auto buffer = new WCHAR[buffer_length];
	hr = GetPackagesByPackageFamily(PACKAGE_FAMILY_NAME, &count, full_names, &buffer_length, buffer);
	if (!SUCCEEDED(hr)) {
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
	if (!SUCCEEDED(hr)) {
		return hr;
	}

	hr = package_debug_settings->EnableDebugging(package_full_name, NULL, NULL);
	if (!SUCCEEDED(hr)) {
		return hr;
	}

}
