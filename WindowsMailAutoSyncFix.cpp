#using <System.dll>
#include <ShObjIdl.h>
#include <appmodel.h>
#include <Shlwapi.h>
#include <Psapi.h>

const wchar_t* PACKAGE_FAMILY_NAME = L"microsoft.windowscommunicationsapps_8wekyb3d8bbwe";
const wchar_t* APP_USER_MODEL_ID = L"microsoft.windowscommunicationsapps_8wekyb3d8bbwe!microsoft.windowslive.mail";
const wchar_t* BACKGROUND_EXE_NAME = L"HxTsr.exe";

wchar_t background_exe_path[MAX_PATH];

DWORD GetFirstPidByPath(PCWSTR path) {
	DWORD processes[1024];
	DWORD needed;
	if (!EnumProcesses(processes, sizeof(processes), &needed)) {
		return 0;
	}

	for (auto i = 0; i < needed / sizeof(DWORD); i++) {
		auto pid = processes[i];
		auto handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
		WCHAR file_name[MAX_PATH];
		GetModuleFileNameEx(handle, NULL, file_name, MAX_PATH);
		CloseHandle(handle);
		if (wcscmp(file_name, path) == 0) {
			return pid;
		}
	}

	return 0;
}

void SystemEvents_PowerChanged(System::Object^ sender, Microsoft::Win32::PowerModeChangedEventArgs^ e) {
	if (e->Mode == Microsoft::Win32::PowerModes::Suspend) {
		auto pid = GetFirstPidByPath(background_exe_path);
		if (pid == 0) {
			return;
		}
		HANDLE handle = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
		TerminateProcess(handle, 0); // Not sure what error code should I use
		CloseHandle(handle);
	}
}

int main() {

	// Allow single instance only
	HRESULT hr;
	WCHAR us_path[MAX_PATH];
	hr = GetModuleFileName(NULL, us_path, MAX_PATH);
	if (hr == 0) {
		return hr;
	}
	// Replace '\' with '/' since CreateMutex treats it as a namespace separator
	// https://devblogs.microsoft.com/oldnewthing/20120112-00/?p=8583
	for (auto i = 0; i < MAX_PATH; i++) {
		auto character = us_path[i];
		if (character == L'\0') {
			break;
		}
		if (character == L'\\') {
			us_path[i] = L'/';
		}
	}
	wchar_t mutex_name[MAX_PATH + 7] = L"Global\\";
	wcsncat_s(mutex_name, us_path, MAX_PATH);
	auto handle = CreateMutex(NULL, TRUE, mutex_name);
	if (handle == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
		return 1;
	}

	// System.dll seems to have called CoInitialize already
	//auto hr = CoInitialize(NULL);
	//if (hr != S_OK) {
	//	return hr;
	//}

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

	// Get background exe path (for suspend/resume on sleep/wakeup)
	UINT32 path_length = 0;
	hr = GetPackagePathByFullName(package_full_name, &path_length, NULL);
	if (hr != ERROR_INSUFFICIENT_BUFFER) {
		return hr;
	}
	auto package_path = new WCHAR[path_length];
	hr = GetPackagePathByFullName(package_full_name, &path_length, package_path);
	if (hr != S_OK) {
		return hr;
	}
	auto out = PathCombine(background_exe_path, package_path, BACKGROUND_EXE_NAME);
	if (out == NULL) {
		return 1;
	}

	// Enable debug
	IPackageDebugSettings* package_debug_settings;
	hr = CoCreateInstance(
		CLSID_PackageDebugSettings,
		NULL,
		CLSCTX_ALL,
		IID_IPackageDebugSettings,
		(void**)&package_debug_settings
	);
	if (hr != S_OK) {
		return hr;
	}

	hr = package_debug_settings->EnableDebugging(package_full_name, NULL, NULL);
	if (hr != S_OK) {
		return hr;
	}

	// Suspend and resume background exe on power state change
	Microsoft::Win32::SystemEvents::PowerModeChanged += gcnew Microsoft::Win32::PowerModeChangedEventHandler(SystemEvents_PowerChanged);

	// Prelaunch once terminated
	IApplicationActivationManager* app_activation_manager;
	hr = CoCreateInstance(
		CLSID_ApplicationActivationManager,
		NULL,
		CLSCTX_ALL,
		IID_IApplicationActivationManager,
		(void**)&app_activation_manager
	);
	if (hr != S_OK) {
		return hr;
	}

	while (true) {
		DWORD pid;
		hr = app_activation_manager->ActivateApplication(APP_USER_MODEL_ID, NULL, AO_PRELAUNCH, &pid);
		if (hr != S_OK) {
			return hr;
		}
		auto handle = OpenProcess(SYNCHRONIZE, FALSE, pid);
		auto status = WaitForSingleObject(handle, INFINITE);
		if (status != WAIT_OBJECT_0) {
			Sleep(1000); // Prevent infinite loop consume too much cpu
		}
		CloseHandle(handle);
	}

}
