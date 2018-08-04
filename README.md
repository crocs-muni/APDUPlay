PC/SC APDU inspection and manipulation tool (APDUPlay)
========

[![Latest release](https://img.shields.io/github/release/crocs-muni/apduplay.svg)](https://github.com/crocs-muni/apduplay/releases/latest)

Windows DLL: [![Build status](https://ci.appveyor.com/api/projects/status/ktwde29drhtw7jml?svg=true)](https://ci.appveyor.com/project/petrs/apduplay)
Linux SO: [![Build Status](https://travis-ci.org/crocs-muni/apduplay.svg?branch=master)](https://travis-ci.org/crocs-muni/apduplay)

The APDUPlay project allows you to log, modify, redirect and visualize smartcard communication realized via PC/SC interface (winscard.dll library). The functionality is achieved by custom "stub" library (provided by APDUPlay) which intercepts and redirects the communication to original winscard.dll (provided by Microsoft) or remote socket proxy. The project supports applications running on Windows Vista, 7, 8, 10 and Linux both 32- and 64-bit.

The primary uses for APDUPlay project are debugging and reverse engineering of unknown APDU-based protocols, redirection of communication to the remote smartcard and penetration testing for black-box applications. The APDUPlay project is based on (now inactive) ApduView tool (http://www.fernandes.org/apduview/index.html) by Andrew Fernandes which allowed to log PC/SC communication.  

The APDUPlay project provides the following functionality: 
  * Log content and additional information about the exchanged PC/SC communication (APDU packets).
  * Redirect communication via socket to other device/computer to support remotely connected smartcards (only Windows version at the moment).
  * Manipulate the communication in real time based on pattern matching rules (e.g., always return success for VERIFY PIN command despite an incorrect PIN value).
  * Reorder list of smart card readers detected and returned by SCardListReaders (e.g., if multiple readers exist and application always connects only to the first one)
  * Visualize captured data in a structured way (separate Java project Parser combined with [Graphviz](https://www.graphviz.org/)).

See more details at https://github.com/crocs-muni/APDUPlay/wiki.

##  Installation (Windows OS)
1. Find out if a targeted application is 32- or 64-bit [(Use Microsoft Sysinternals Sigcheck utility)](https://docs.microsoft.com/en-us/sysinternals/downloads/sigcheck). Run sigcheck.exe targetApp.exe and look for MachineType: 32-bit or 64-bit (also works for dll files) 
1. Copy Winscard.dll from your system folder (c:\Windows\System32\winscard.dll for 64-bit target application (if you are running 64-bit OS) or c:\Windows\SysWOW64\winscard.dll for 32-bit application) to the folder with target application and rename it to original32.dll or original64.dll respectively. NOTE: c:\Windows\System32\ contains either 32-bit or 64-bit version based on your OS.
2. Copy Winscard.dll from APDUPlay project to the folder with target application (the folder should contain winscard.dll binary from APDUPlay project AND originalXX.dll which is Microsoft's original winscard.dll)
3. Run the application and inspect resulting files winscard_log.txt and winscard_rules_log.txt
4. (Optional) Change configuration file winscard_rules.txt to modify default behavior (see below)

##  1. Logging of exchanged APDU commands
This simplest use case allows you to log all APDUs exchanged between a target application and a physical smartcard.  

```ini
target_application <--> APDUPlay_winscard.dll <--> original64.dll <--> smartcard
                         |                                  ^
                         v                                  | 
                   winscard_log.txt (logged APDUs)     renamed winscard.dll by Microsoft
```

1. Find out if your application requires 32- or 64-bit winscard.dll (e.g., using [Sigcheck utility](https://docs.microsoft.com/en-us/sysinternals/downloads/sigcheck))
2. Copy Microsoft's original winscard.dll library to a target application folder and rename it to original32.dll or original64.dll (based on Step 1). 
3. Place APDUPlay's custom winscard.dll library to a target application folder, so it is loaded first.
4. Place configuration file named winscard_rules.txt into the same folder.

The target application folder should now look like this (CAProfiler_64b.exe used as an example)
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
```ini
[begin]
SCardTransmit (handle 0x00000000)#
apduCounter:0#
totalBytesINCounter:1#
transmitted:00 a4 04 00 05 a0 00 00 00 03 00
responseTime:12#
SCardTransmit result:0x0#
received:6f 10 84 08 a0 00 00 00 03 00 00 00 a5 04 9f 65 01 ff 90 00

SCardTransmit (handle 0x00000000)#
apduCounter:1#
totalBytesINCounter:12#
transmitted:00 a4 04 00 05 a0 00 00 00 04 00
responseTime:9#
SCardTransmit result:0x0#
received:6a 82
...
```
7. (Optional) Inspect other APDUPlay features like APDU commands modification or redirection to remote socket proxy.

The installation and usage process is relatively simple, but may be little tricky if something does not work (e.g., the library is loaded from a different than an expected path, the application terminates without giving any error message, etc.). The first step is always to make sure that APDUPlay's stub library is used together with the correct winscard_rules.txt. See Examples section for detailed step-by-step installation and troubleshooting. 

**Note:** This example shows the simplest use case only with logging of the exchanged APDU commands. For more advanced examples check [wiki pages for examples](https://github.com/crocs-muni/APDUPlay/wiki/Examples-for-OS-MS-Windows). 


##  2. Redirection of APDU commands to socket proxy (on a remote computer)
This use case allows you to utilize APDUPlay to redirect all APDU commands to specified socket proxy and propagate back the proxy response as response APDU command back to target application. The proxy can run on localhost or remote computer and can be written in any language. 

There are two primary uses for this feature:
1. A simulation of locally-connected smartcard while connected to a remote computer. 
```ini
target_application <--> APDUPlay_winscard.dll <--> socket_proxy_localhost

target_application <--> APDUPlay_winscard.dll <--> socket_proxy_remotehost <--> smartcard_remotehost
```
2. More advanced processing of the exchanged APDUs in any suitable programming language and without need for recompilation of APDUPlay dll/so libraries. The proxy can be written in any language (e.g., Python pySimonaProxy). 
```ini
target_application <--> APDUPlay_winscard.dll <--> python_socket_proxy_localhost

target_application <--> APDUPlay_winscard.dll <--> socket_proxy_localhost <--> RESTproxy_remotehost <--> smartcard_remotehost
```

Note, that original winscard.dll (renamed as original64.dll) is not even used for transmission of APDU in this case as no physical smartcard is contacted/present on localhost.

##  3. Manipulate exchanged APDUs in real time based on pattern matching rules 
This use case allows to match exchanged APDU (both input and response) against the defined patterns and modify it accordingly before sending to physical smartcard (input APDU) or back to the target application (response APDU). 

```ini
target_application <--> APDUPlay_winscard.dll <--> modified_apdu <--> original64.dll <--> smartcard
                                 ^
                                 |
                           winscard_rules.txt (definition of rewrite rules)
```
```ini
[WINSCARD]
...
MODIFY_APDU_BY_RULES = 1                           <---- enable modification of the exchanged APDUs
...
[RULE1]                                            <---- first modification rule 
APDUIN = 1                                         <---- if 1, rule is checked for an input APDU (application to card), if 0 then on response APDU (smartcard to application)
MATCH1=in=1,cla=80,ins=ca,p1=9f,p2=17,data0=90 00  <---- apply rule when input APDU has CLA==0x80, INS==0xca ...
ACTION=in=1,cla=80,ins=cb,p1=9f,p2=17,data0=97 00  <---- if match, then change INS to 0xca and data[0] to 0x97 
USAGE = 1                                          <---- if 1, rule is in use. If 0, then rule is ignored  
[RULE2]                                            <---- another modification rule
...
```

## Examples
The localization of correct winscard.dll path can sometimes be a tedious task, especially for applications using additional frameworks to access PC/SC interface. Check [wiki pages for examples](https://github.com/crocs-muni/APDUPlay/wiki/Examples-for-OS-MS-Windows) with the increasing difficulty:
  1. Simple application directly using winscard.dll ([CAProfiler.exe](https://github.com/petrs/CAProfiler/releases/latest))
  2. Application with persistent agent ([gpg2.exe --card-edit](https://gpg4win.org/download.html))
  3. Java-based application accessing smartcards via JRE: ([GlobalPlatformPro gp -l](https://github.com/martinpaljak/GlobalPlatformPro))

Alternatively, you may replace winscard.dll directly inside the system folder. Warning: PC/SC communication from ALL applications running on your system is now intercepted and logged to file including your PINs, passwords etc. - so use this option with care!


## Troubleshooting
Please read [Troubleshooting](https://github.com/crocs-muni/APDUPlay/wiki/Troubleshooting) wiki page

## Bugs and issues
Please, open an issue in case of any bug is found. 


