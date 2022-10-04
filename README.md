# Windows Mail Auto Sync Fix

[Windows Mail app](https://apps.microsoft.com/store/detail/mail-and-calendar/9WZDNCRFHVQM) doesn't sync Gmail (download new content set to as items arrive) automatically when minimized/closed (UWP suspended)

And after searching through the internet for like a whole day and tried a lot methods

> reset the app, update your app, update windows, reboot, hope it helps

I gave up and made this

This program simply run it with debug enabled, and this will make it sync after minimized/closed
(according to [Microsoft Docs](https://learn.microsoft.com/en-us/windows/uwp/launch-resume/run-minimized-with-extended-execution#:~:text=These%20application%20lifecycle%20time%20constraints%20are%20disabled%20while%20the%20app%20is%20running%20under%20a%20debugger.))

> These application lifecycle time constraints are disabled while the app is running under a debugger.

## Usage

Get the lastest excutable from [Releases](./releases/latest) and run the .exe file, then the Mail app should be auto sync fixed

If you want this app to run at start up, make a shortcut and put it in `C:\ProgramData\Microsoft\Windows\Start Menu\Programs\StartUp`(all users) or `%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup`(current user)

## Acknowledgement

Big thanks to this article: https://www.unknowncheats.me/forum/general-programming-and-reversing/177183-basic-intermediate-techniques-uwp-app-modding.html
