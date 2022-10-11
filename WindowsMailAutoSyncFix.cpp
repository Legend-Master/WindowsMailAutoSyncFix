#using <System.dll>
#include <ShObjIdl.h>
#include <appmodel.h>

typedef NTSTATUS(NTAPI* pNtSuspendProcess)(IN HANDLE ProcessHandle);
pNtSuspendProcess NtSuspendProcess;
typedef NTSTATUS(NTAPI* pNtResumeProcess)(IN HANDLE ProcessHandle);
pNtResumeProcess NtResumeProcess;

const wchar_t* PACKAGE_FAMILY_NAME = L"microsoft.windowscommunicationsapps_8wekyb3d8bbwe";
const wchar_t* APP_USER_MODEL_ID = L"microsoft.windowscommunicationsapps_8wekyb3d8bbwe!microsoft.windowslive.mail";

DWORD pid = 0;

void SystemEvents_PowerChanged(System::Object^ sender, Microsoft::Win32::PowerModeChangedEventArgs^ e)
{
	if (pid == 0) {
		return;
	}
	if (e->Mode == Microsoft::Win32::PowerModes::Suspend) {
		HANDLE handle = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
		NtSuspendProcess(handle);
		CloseHandle(handle);
	}
	else if (e->Mode == Microsoft::Win32::PowerModes::Resume) {
		HANDLE handle = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
		NtResumeProcess(handle);
		CloseHandle(handle);
	}
}

int main() {

	// System.dll seems to have called CoInitialize already
	//auto hr = CoInitialize(NULL);
	//if (hr != S_OK) {
	//	return hr;
	//}
	HRESULT hr;

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

	auto ntdll = GetModuleHandle(L"ntdll.dll");
	if (ntdll == NULL) {
		return GetLastError();
	}
	NtSuspendProcess = (pNtSuspendProcess)GetProcAddress(ntdll, "NtSuspendProcess");
	NtResumeProcess = (pNtResumeProcess)GetProcAddress(ntdll, "NtResumeProcess");

	Microsoft::Win32::SystemEvents::PowerModeChanged += gcnew Microsoft::Win32::PowerModeChangedEventHandler(SystemEvents_PowerChanged);

	while (true) {
		hr = app_activation_manager->ActivateApplication(APP_USER_MODEL_ID, NULL, AO_PRELAUNCH, &pid);
		if (hr != S_OK) {
			return hr;
		}

		HANDLE handle = OpenProcess(SYNCHRONIZE, FALSE, pid);
		auto status = WaitForSingleObject(handle, INFINITE);
		if (status != WAIT_OBJECT_0) {
			Sleep(1000); // Prevent infinite loop consume too much cpu
		}
		CloseHandle(handle);
	}

}
