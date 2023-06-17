# The Dev Story

### The start

When I first getting started, I'm just looking at the behavior of the Mail app, when does it work?

It only works when you have it's window opened(not minimized, not closed, can be not focused/get overlapped by other windows), or it works pretty inconsistent

But why? I found this https://learn.microsoft.com/en-us/windows/uwp/launch-resume/app-lifecycle

> <table>
> <tr>
> <th>ApplicationExecutionState</th>
> <th>Explanation</th>
> <th>Action to take</th>
> </tr>
> <tbody>
> <tr>
> <td><strong>NotRunning</strong></td>
> <td>An app could be in this state because it hasn't been launched since the last time the user rebooted or logged > in. It can also be in this state if it was running but then crashed, or because the user closed it earlier.</td>
> <td>Initialize the app as if it is running for the first time in the current user session.</td>
> </tr>
> <tr>
> <td><strong>Suspended</strong></td>
> <td>The user either minimized or switched away from your app and didn't return to it within a few seconds.</td>
> <td>When the app was suspended, its state remained in memory. You only need to reacquire any file handles or other > resources you released when the app was suspended.</td>
> </tr>
> <tr>
> <td><strong>Terminated</strong></td>
> <td>The app was previously suspended but was then shutdown at some point because the system needed to reclaim memory.> </td>
> <td>Restore the state that the app was in when the user switched away from it.</td>
> </tr>
> <tr>
> <td><strong>ClosedByUser</strong></td>
> <td>The user closed the app with the system close button, or with Alt+F4. When the user closes the app, it is first > suspended and then terminated.</td>
> <td>Because the app has essentially gone through the same steps that lead to the Terminated state, handle this the > same way you would the Terminated state.</td>
> </tr>
> <tr>
> <td><strong>Running</strong></td>
> <td>The app was already open when the user tried to launch it again.</td>
> <td>Nothing. Note that another instance of your app is not launched. The already running instance is simply > activated.</td>
> </tr>
> </tbody>
> </table>

So basically UWP apps will suspend on minimize, and I assume they're not doing things right in the background task, so it only works when you have it at running state, and if we can resume it every time it suspends, it's gonna be fixed right?

And I looked into the UWP debug docs a bit and got this: https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ipackagedebugsettings

Then I tried all these methods and I noticed that after I call `EnableDebugging`, it will not suspends any more, and isn't that what we wanted?

And some more searches, found this: https://learn.microsoft.com/en-us/windows/uwp/launch-resume/run-minimized-with-extended-execution#:~:text=These%20application%20lifecycle%20time%20constraints%20are%20disabled%20while%20the%20app%20is%20running%20under%20a%20debugger

And yeah, problem solved right?

Well, only for minimized, not for closed

Then I'm thinking about I saw UWP apps hanging in task mannager before, should take a look at it

And I found something really odd, other UWP apps like Calculator go into Background process and having a small icon saying it's suspended when you close it, but not for Mail, at least not consistently

After some more seaches, at https://learn.microsoft.com/en-us/windows/uwp/launch-resume/handle-app-prelaunch

I finally realized this thing is prelauch, and after doing a prelauch after close, everything just works, yeah, we fixed it

Unfortunately still no, it breaks when you go hibernate/sleep

I had no clue why, and the only solution I can think of would be suspend/close the app when hibernate/sleep, and resume/re-open on wake up

And I ran into a problem, there're 2 ways of getting power(sleep/resume) change events, one for window, one for service, to be honest, this program should be a service, but I'm not feeling confident about it, so I went for the window one, but this program don't have a window, and I don't really like the invisible window solution as well, so I searched up and found CLR(.Net stuffs in c++), and there's a function for this

Next problem, suspend or kill the Mail, kill the process is not a good idea, since you can't really restore things after wake up, so I went for suspend

And I can't quite understand why suspend a process is that much work (no such documented API, only `SuspendThread`), so you'll have to create a snapshot and go through each one to suspend all threads), so I seached again, an undocumented API `NtSuspendProcess`, `NtResumeProcess`, that's it!

So I did this, `NtSuspendProcess` Mail's main process (HxOutlook.exe) on sleep, and `NtResumeProcess` it on wake up, and it worked

I was happy about this, but after a few days, I can't get any notifications again, and the worse thing is that I can't reproduce it consistantly, it's going to break after a while, but no pattern I can find (and testing this is a nightmare)

Then, I tried to suspend the background process (HxTsr.exe, which is also the background exe for Calendar app), and it worked better than suspending main process (HxOutlook.exe), but still, not always

I gave up on suspending, and just kill the program on sleep, and yeah, it's way better, but still not always

At this point, guess I'll have to live with it, put my phone aside and make it notify me with a sound, overall, fixing a program which you can't interact directly, is hard, and I'm proud of myself for going this far

### Updates from May 2023:

#### Fix having to restart when Mail updates

It's really bad that it stops working whenever the Mail app updates, when it updates, it's going to shutdown the Mail app, and then we detected it, so we'll try to restart prelaunch it, and it leads to a prompt saying "it's updating, please wait", and a failed call causing the program to exit

My first thought was if there's a way to detect if it's updating, but I never found it, and I think keep trying won't be a good idea because of that prompt (yeah, unacceptable)

So for a long time I'm just leaving it and stop thinking about it

Util a day, just getting bored and picked this up again, and I just feel like trying to call `ActivateApplication` with `AO_NOERRORUI` (which is something I didn't understand what is it for), and it's be like they know what I'm doing, the Mail app updated in just 2 days, and it worked!

#### Message box style

I was using message box to debug if it worked, since it's a easy way to prompt me when the Mail app updates and if something's going on (too lazy to log things)

And found that the message box icons are in old styles, after some searches, I reallized that it's more complicated then I thought

The system icons are related to comctl32.dll, and Windows shipped two version of it, the default one is in the System32 directory, and the newer version is in the side load directory WinSxS, and specifing Common-Controls version to 6 in the app manifest will make it load the newer version

### Updates from June 2023

#### Remove CLR

> but this program don't have a window, and I don't really like the invisible window solution as well, so I searched up and found CLR(.Net stuffs in c++), and there's a function for this

Well, so .NET is using this method anyway... https://github.com/microsoft/referencesource/blob/51cf7850defa8a17d815b4700b67116e3fa283c2/System/compmod/microsoft/win32/SystemEvents.cs#LL1506C34-L1506C34

After looking for more documents and some more experiments, I found [`PowerRegisterSuspendResumeNotification`](https://learn.microsoft.com/en-us/windows/win32/api/powerbase/nf-powerbase-powerregistersuspendresumenotification), and Go is using it as well (https://github.com/golang/go/blob/c5463218a228b082661df3f5f1ba0492a4d3df18/src/runtime/os_windows.go#L305)
