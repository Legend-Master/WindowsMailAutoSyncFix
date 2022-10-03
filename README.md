# Windows Mail Auto Sync Fix

[Windows Mail app](https://apps.microsoft.com/store/detail/mail-and-calendar/9WZDNCRFHVQM) does not sync Gmail (download new content set to as items arrive) automatically when minimized (UWP suspended)

And after searching through the internet for like a whole day and tried a lot methods

> reset the app, update your app, update windows, reboot, hope it helps

I gave up and made this

This program simply run it with debug enabled, and this will make it sync after minimized
(according to [Microsoft Docs](https://learn.microsoft.com/en-us/windows/uwp/launch-resume/run-minimized-with-extended-execution#:~:text=These%20application%20lifecycle%20time%20constraints%20are%20disabled%20while%20the%20app%20is%20running%20under%20a%20debugger.))

> These application lifecycle time constraints are disabled while the app is running under a debugger.

## Usage

Get the excutable from Releases and run the .exe file to open Mail app with auto sync fixed

If you want this app show up in the app list (when you press <kbd>Win</kbd>), make a shortcut and put it in `C:\ProgramData\Microsoft\Windows\Start Menu\Programs`(all user) or `%APPDATA%\Microsoft\Windows\Start Menu\Programs`(current user)

If you want the icon to be Mail app's, then it'll go a bit harder (since I don't have the right to distribute it):

1. Goto `C:\Program Files\WindowsApps\microsoft.windowscommunicationsapps_16005.14326.20970.0_x64__8wekyb3d8bbwe\images`(you might need to change `16005.14326.20970.0` to your mail app's version) directory
2. Use an ico maker like https://image.online-convert.com/convert-to-ico to convert an icon you like (I prefer `HxMailAppList.targetsize-256_altform-unplated.png`)
3. Change your shortcut's icon to what you just converted

## Acknowledgement

Big thanks to this article: https://www.unknowncheats.me/forum/general-programming-and-reversing/177183-basic-intermediate-techniques-uwp-app-modding.html
