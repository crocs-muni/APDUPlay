PC/SC APDU inspection and manipulation tool (APDUPlay)
========

[![Latest release](https://img.shields.io/github/release/crocs-muni/apduplay.svg)](https://github.com/crocs-muni/apduplay/releases/latest)

Windows DLL: [![Build status](https://ci.appveyor.com/api/projects/status/ktwde29drhtw7jml?svg=true)](https://ci.appveyor.com/project/petrs/apduplay)
Linux SO: [![Build Status](https://travis-ci.org/crocs-muni/apduplay.svg?branch=master)](https://travis-ci.org/crocs-muni/apduplay)

The APDUPlay project allows you to log, modify, redirect and visualize smartcard communication realized via PC/SC interface (winscard.dll library). The functionality is achieved by custom "stub" library (provided by APDUPlay) which intercepts and redirects the communication to  original winscard.dll (provided by Microsoft) or . The project is applicable to applications running on Windows Vista, 7, 8, 10 and Linux both 32- and 64-bit.

The primary uses for APDUPlay project are debugging and reverse engineering of unknown APDU-based protocols, redirection of communication to remote smartcard and penetration testing for black-box applications. The APDUPlay project is based on (now inactive) ApduView tool (http://www.fernandes.org/apduview/index.html) by Andrew Fernandes which allowed to log PC/SC communication.  

The APDUPlay project provides following functionality: 
  * Log content and additional information about the exchanged PC/SC communication (APDU packets).
  * Manipulate the communication in real time based on pattern matching rules (e.g., always return success for VERIFY PIN command despite an incorrect PIN value).
  * Redirect communication via socket to other device/computer to support remotely connected smartcards (only Windows version at the moment).
  * Reorder list of smart card readers detected and returned by SCardListReaders (e.g., if multiple readers exist and application always connects only to the first one)
  * Visualize captured data in a structured way (separate Java project Parser combined with [Graphviz](graphviz.org)).

See more details at https://github.com/petrs/APDUPlay/wiki.

##  Installation and use 
1. Find out if your application requires 32- or 64-bit winscard.dll (e.g., using [Sigcheck utility](https://docs.microsoft.com/en-us/sysinternals/downloads/sigcheck))
2. Copy Microsoft's original winscard.dll library to a target application folder and rename it to original32.dll or original64.dll (based on Step 1). 
3. Place APDUPlay's custom winscard.dll library to a target application folder so it is loaded first.
4. Place configuration file named winscard_rules.txt into the same folder.

The target application folder should now looks like this (CAProfiler_64b.exe used as example)
```cmd
.
..
CAProfiler_64b.exe       <---- target application (64-bit)
original64.dll           <---- original Microsoft's winscard.dll copied from C:\System\System32\
Winscard.dll             <---- APDUPlay's stub winscard.dll (64-bit)
winscard_rules.txt       <---- configuration file loaded by APDUPlay's stub winscard.dll
```
The simple example content of winscard_rules.txt file should look like: 
```ini
[WINSCARD]
LOG_EXCHANGED_APDU = 1
LOG_BASE_PATH = ./
```

5. Now run target application (CAProfiler_64b.exe in this example). The application should execute normally. 
```cmd
>CAProfiler_64b.exe
Available readers:
1. Generic EMV Smartcard Reader 0
2. Simona /111.222.123.033@07
... 
The following applets were detected:
APPLET_AID_JCOP_CM             : a0 00 00 00 03 00 00 00
APPLET_AID_VISA_PREFIX         : a0 00 00 00 03
```
6. Inspect the generated files with an intercepted communication (winscard_log.txt, apduplay_debug.txt ...).

The installation and usage process is relatively simple, but may be little tricky if something doesn't work (e.g., library is loaded from a different than expected path, application terminates without giving any error message etc.). See Examples section for detailed step-by-step installation and troubleshooting. 

##  Installation (Windows OS)
1. Find out if targeted application is 32- or 64-bit [(Use Microsoft Sysinternals Sigcheck utility)](https://docs.microsoft.com/en-us/sysinternals/downloads/sigcheck). Run sigcheck.exe targetApp.exe and look for MachineType: 32-bit or 64-bit (works also for dll files) 
1. Copy Winscard.dll from your system folder (c:\Windows\System32\winscard.dll for 64-bit target application (if you are running 64-bit OS) or c:\Windows\SysWOW64\winscard.dll for 32-bit application) to the folder with target application and rename it to original32.dll or original64.dll respectively. NOTE: c:\Windows\System32\ contains either 32-bit or 64-bit version based on your OS.
2. Copy Winscard.dll from APDUPlay project to the folder with target application (the folder should contain winscard.dll binary from APDUPlay project AND originalXX.dll which is Microsoft's original winscard.dll)
3. Run the application and inspect resulting files winscard_log.txt and winscard_rules_log.txt
4. (Optional) Change configuration file winscard_rules.txt to modify default behavior (see below)


## Examples
The localization of correct winscard.dll path can be sometimes a tedious task, especially for applications using additional frameworks to access PC/SC interface. Here are some examples with increasing difficulty:
  1. Simple application directly using winscard.dll ([CAProfiler.exe](https://github.com/petrs/CAProfiler/releases/latest))
  2. Application with persistent agent ([gpg2.exe --card-edit](https://gpg4win.org/download.html))
  3. Java-based application accessing smartcards via JRE: ([GlobalPlatformPro gp -l](https://github.com/martinpaljak/GlobalPlatformPro))

Alternatively, you may replace winscard.dll directly in the system folder. Warning: PC/SC communication from ALL applications runing on your system is now intercepted and logged to file including your PINs, passwords etc. - so use this option with care!


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


