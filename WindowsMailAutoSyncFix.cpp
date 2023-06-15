#include <memory>

#using <System.dll>
#include <ShObjIdl.h>
#include <appmodel.h>
#include <Shlwapi.h>
#include <Psapi.h>
#include <winrt/base.h>

const wchar_t* PACKAGE_FAMILY_NAME = L"microsoft.windowscommunicationsapps_8wekyb3d8bbwe";
const wchar_t* APP_USER_MODEL_ID = L"microsoft.windowscommunicationsapps_8wekyb3d8bbwe!microsoft.windowslive.mail";
const wchar_t* BACKGROUND_EXE_NAME = L"HxTsr.exe";

const wchar_t* ERROR_MESSAGE_BOX_TITLE = L"Mail Auto Sync Fix";

wchar_t background_exe_path[MAX_PATH];

void ShowErrorMessageBox(long error_code) {
	wchar_t error_text[1000];
	if (!FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
		NULL,
		error_code,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		// 0,
		error_text,
		sizeof(error_text),
		NULL))
	{
		MessageBox(NULL, L"Something wrong when trying to get the error message", ERROR_MESSAGE_BOX_TITLE, MB_ICONERROR);
		return;
	}
	MessageBox(NULL, error_text, ERROR_MESSAGE_BOX_TITLE, MB_ICONERROR);
}

DWORD GetFirstPidByPath(wchar_t* path) {
	DWORD processes[1024];
	DWORD needed;
	if (!EnumProcesses(processes, sizeof(processes), &needed)) {
		return 0;
	}

	for (auto i = 0; i < needed / sizeof(DWORD); i++) {
		auto pid = processes[i];
		auto handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid);
		wchar_t file_name[MAX_PATH];
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
		HANDLE handle = OpenProcess(PROCESS_TERMINATE, false, pid);
		TerminateProcess(handle, 0); // Not sure what error code should I use
		CloseHandle(handle);
	}
}

long MainFunctions() {
	// Get package full name
	uint32_t count = 0;
	uint32_t buffer_length = 0;
	auto error_code = GetPackagesByPackageFamily(PACKAGE_FAMILY_NAME, &count, NULL, &buffer_length, NULL);
	if (count < 1 || error_code != ERROR_INSUFFICIENT_BUFFER) {
		return error_code;
	}
	auto full_names = std::make_unique<wchar_t* []>(count);
	auto buffer = std::make_unique<wchar_t[]>(buffer_length);
	error_code = GetPackagesByPackageFamily(PACKAGE_FAMILY_NAME, &count, full_names.get(), &buffer_length, buffer.get());
	if (error_code != NO_ERROR) {
		return error_code;
	}
	auto package_full_name = full_names[0];

	// Get background exe path (for suspend/resume on sleep/wakeup)
	uint32_t path_length = 0;
	error_code = GetPackagePathByFullName(package_full_name, &path_length, NULL);
	if (error_code != ERROR_INSUFFICIENT_BUFFER) {
		return error_code;
	}
	auto package_path = std::make_unique<wchar_t[]>(path_length);
	error_code = GetPackagePathByFullName(package_full_name, &path_length, package_path.get());
	if (error_code != NO_ERROR) {
		return error_code;
	}
	auto out = PathCombine(background_exe_path, package_path.get(), BACKGROUND_EXE_NAME);
	if (out == NULL) {
		return 1;
	}

	// Enable debug
	auto package_debug_settings = winrt::create_instance<IPackageDebugSettings>(CLSID_PackageDebugSettings);
	auto hr = package_debug_settings->EnableDebugging(package_full_name, NULL, NULL);
	if (hr != S_OK) {
		return hr;
	}

	return NO_ERROR;
}

int main() {

	// MessageBox(NULL, L"Text", ERROR_MESSAGE_BOX_TITLE, MB_ICONWARNING);

	// Allow single instance only
	wchar_t us_path[MAX_PATH];
	auto path_length = GetModuleFileName(NULL, us_path, MAX_PATH);
	if (path_length == 0) {
		return GetLastError();
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
	auto handle = CreateMutex(NULL, true, mutex_name);
	if (handle == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
		return 1;
	}

	// System.dll seems to have called CoInitialize already
	//winrt::init_apartment();
	//auto hr = CoInitialize(NULL);
	//if (hr != S_OK) {
	//	return hr;
	//}

	auto error_code = MainFunctions();
	if (error_code != NO_ERROR) {
		ShowErrorMessageBox(error_code);
		return error_code;
	}

	// Suspend and resume background exe on power state change
	Microsoft::Win32::SystemEvents::PowerModeChanged += gcnew Microsoft::Win32::PowerModeChangedEventHandler(SystemEvents_PowerChanged);

	// Prelaunch once terminated
	auto app_activation_manager = winrt::create_instance<IApplicationActivationManager>(CLSID_ApplicationActivationManager);
	auto failed_once = false;
	while (true) {
		DWORD pid;
		auto hr = app_activation_manager->ActivateApplication(APP_USER_MODEL_ID, NULL, AO_PRELAUNCH | AO_NOERRORUI, &pid);
		if (hr != S_OK) {
			failed_once = true;
			Sleep(5000);
			continue;
		}
		if (failed_once) {
			failed_once = false;
			error_code = MainFunctions();
			if (error_code != NO_ERROR) {
				ShowErrorMessageBox(error_code);
				return error_code;
			}
			// MessageBox(NULL, L"Restart succeed", ERROR_MESSAGE_BOX_TITLE, MB_ICONINFORMATION);
		}
		auto handle = OpenProcess(SYNCHRONIZE, false, pid);
		auto status = WaitForSingleObject(handle, INFINITE);
		if (status != WAIT_OBJECT_0) {
			Sleep(1000); // Prevent infinite loop consume too much cpu
		}
		CloseHandle(handle);
	}

}
