# Windows Mail Auto Sync Fix

[Windows Mail app](https://apps.microsoft.com/store/detail/mail-and-calendar/9WZDNCRFHVQM) doesn't sync Gmail (download new content set to as items arrive) automatically when minimized/closed (UWP suspended)

And after searching through the internet for like a whole day and tried a lot methods

> reset the app, update your app, update windows, reboot, hope it helps

I gave up and made this

This program simply run it with debug enabled
(according to [Microsoft Docs](https://learn.microsoft.com/en-us/windows/uwp/launch-resume/run-minimized-with-extended-execution#:~:text=These%20application%20lifecycle%20time%20constraints%20are%20disabled%20while%20the%20app%20is%20running%20under%20a%20debugger.)), and prelaunch it once terminated, and this will make it sync after minimized and even closed (will re-prelaunch in background)

> These application lifecycle time constraints are disabled while the app is running under a debugger.

## Usage

Get the [latest excutable](https://github.com/Legend-Master/WindowsMailAutoSyncFix/releases/latest/download/WindowsMailAutoSyncFix.exe
) and run it, then the Mail app should be auto sync fixed

If you want this app to run at start up, make a shortcut and put it in `C:\ProgramData\Microsoft\Windows\Start Menu\Programs\StartUp`(all users) or `%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup`(current user)

Note: this program will make the Mail app run in background once you close it, so you'll need to terminate this program first if you want to terminate the Mail app

## Known Issues

~~After resume from sleep/hibernate mode while the Mail app is running in the background (showing in task manager background processes), you'll need to open the Mail app once to get any notifications~~ (Fixed on v0.0.4)

## Acknowledgement

Big thanks to this article: https://www.unknowncheats.me/forum/general-programming-and-reversing/177183-basic-intermediate-techniques-uwp-app-modding.html

`NtSuspendProcess()` and `NtResumeProcess()`'s usage is from: https://github.com/FedericoCarboni/node-ntsuspend
> This library uses NtSuspendProcess() and NtResumeProcess() from NTDLL, these functions are not officially documented on MSDN; they have been consistently available since Windows XP, but are not guaranteed to work in the future.
