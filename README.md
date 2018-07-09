PC/SC APDU inspection and manipulation tool (APDUPlay)
========

[![Latest release](https://img.shields.io/github/release/crocs-muni/apduplay.svg)](https://github.com/crocs-muni/apduplay/releases/latest)

Windows DLL: [![Build status](https://ci.appveyor.com/api/projects/status/ktwde29drhtw7jml?svg=true)](https://ci.appveyor.com/project/crocs-muni/apduplay)
Linux SO: [![Build Status](https://travis-ci.org/crocs-muni/apduplay.svg?branch=master)](https://travis-ci.org/crocs-muni/apduplay)


The project APDUPlay is based on ApduView tool (http://www.fernandes.org/apduview/index.html) which allows you to log communication realized via PC/SC interface (winscard.dll library). The APDUPlay project provides a winscard.dll library which is stub used instead of original winscard.dll provided by Microsoft. For correct usage, you need to find  Microsoft's library and copy it to the folder with an application you are trying to control with APDUPlay. 

Note: The APDUPlay tool is available for download including source codes, yet documentation still lack a bit behind. But as it was requested several times, and I hope it will be useful.

The APDUPlay project is providing following functionality: 
  * Log content and additional information about exchanged PC/SC communication
  * Manipulate communication in real time
  * Redirect communication via socket to other device/computer (only Windows version at the moment)
  * Reorder list of smart card readers detected in a system
  * Visualize captured data in a structured way by GraphViz (Java project Parser)

See more details at https://github.com/petrs/APDUPlay/wiki.

##  Installation  
1. Find out if your targeted application is 32- or 64-bit [(Use Microsoft Sysinternals Sigcheck utility)](https://docs.microsoft.com/en-us/sysinternals/downloads/sigcheck). Run sigcheck.exe targetApp.exe and look for  MachineType: 32-bit or 64-bit (works also for dll files) 
1. Copy Winscard.dll from your system folder (c:\Windows\System32\winscard.dll for 64-bit target application (if you are running 64-bit OS) or c:\Windows\SysWOW64\winscard.dll for 32-bit application) to the folder with target application and rename it to original32.dll or original64.dll respectively. NOTE: c:\Windows\System32\ will contain either 32-bit or 64-bit version based on your OS.
2. Copy Winscard.dll from APDUPlay project to the folder with target application (the folder should contain winscard.dll binary from APDUPlay project AND originalXX.dll which is original Microsoft's winscard.dll)
3. Run the application and inspect resulting files winscard_log.txt and winscard_rules_log.txt
4. (Optional) Change configuration file winscard_rules.txt to modify default behavior (see below)

## Examples

The localization of correct winscard.dll path can be tedious task for some applications. Here are some examples with increasing difficulty:
  1. Simple application directly using winscard.dll (example.exe)
  2. Application with persistent agent (gpg2.exe --card-edit)
  3. Java-based application: (GlobalPlatformPro gp -l)


## Troubleshooting

  * Problem: After running target application, the following error message is displayed: "The procedure entry point original.g_rgSCardT1Pci could not be located in the dynamic link library WinSCard.dll.". You likely mismatched 64-bit and 32-bit versions of APDUPlay's winscard.dll and Microsoft's original library (renamed as original32.dll). Use [Microsoft Sysinternals Sigcheck utility](https://docs.microsoft.com/en-us/sysinternals/downloads/sigcheck) to verify that both libraries as either 64-bit or 32-bit (based on your target application needs).
  
  * Problem: Target application is (probably) not loading modified winscard.dll from APDUPlay project, but uses standard Microsoft's one from system folder (no files with logged communication are created). Use [Process Monitor utility]( https://docs.microsoft.com/en-us/sysinternals/downloads/procmon) from Microsoft to find location of loaded libraries (use Filter option to limit results only to target application: CTRL+L -> Process Name is 'targetApp.exe' -> Add). Search for event 'Load Image path_to_folder\Winscard.dll'. The path_to_folder should point to APDUPlay's version of winscard.dll, not Microsoft one.

  * Problem: Logging seems to work, but only for the first of application. When started again, changes done to winscard_rules.txt does not apply. Target application might have persistent component (e.g., GPG have gpg-agent.exe) which loads the dll (and rules from winscard_rules.txt) and runs even when target application is terminated. Try to locate and kill this component, or restart computer (will force component to restart again).

  * Problem: Target application always opens winscard.dll from system folders (either system32 or sysWOW64 folder). 
Run cmd as admin, then:
```
cd target_folder (either system32 or sysWOW64)
takeown /f winscard.dll
cacls winscard.dll /G your_username:F
rename winscard.dll to winscard_MS.dll  (winscard.dll might be currently used by some other process so direct copy woudl not be possible)
copy APDUPlay's winscard.dll instead winscard.dll
```

Please, open an issue in case of any bug found. 


