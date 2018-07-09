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
1. Find out if your targeted application is 32- or 64-bit [(howto)](https://superuser.com/questions/103071/quick-way-to-tell-if-an-installed-application-is-64-bit-or-32-bit#103073). 
1. Copy Winscard.dll from your system folder (c:\Windows\winscard.dll for 32-bit target application or c:\Windows\SysWOW64\winscard.dll for 64-bit application) to the folder with target application and rename it to original32.dll or original64.dll respectively
2. Copy Winscard.dll from APDUPlay project to the folder with target application (the folder should contain winscard.dll binary from APDUPlay project AND originalXX.dll which is original Microsoft's winscard.dll)
3. Run the application and inspect resulting files winscard_log.txt and winscard_rules_log.txt
4. (Optional) Change configuration file winscard_rules.txt to modify default behavior (see below)

## Troubleshooting

  * If you use (wrongly) 64bit version of library Microsoft's winscard.dll (renamed as original32.dll), it will fail with "The procedure entry point original.g_rgSCardT1Pci could not be located in the dynamic link library WinSCard.dll."  
  



Please, open an issue in case of any bug found. 


