PC/SC APDU inspection and manipulation tool (APDUPlay)
========

The project APDUPlay is based on ApduView tool (http://www.fernandes.org/apduview/index.html) which allows you to log communication realized via PC/SC interface (winscard.dll library). If you are interested only in the log of transmitted data, you can readily
use APDUView project (although APDUPlay project provides information about communication in more structured way more suitable for later post-processing and add some additional information).

Note: The APDUPlay tool is available for download including source codes, yet documentation still lack a bit behind. But as it was requested several times, and I hope it will be useful.

The APDUPlay project is providing following functionality: 
  * Log content and additional information about exchanged PC/SC communication
  * Manipulate communication in real time
  * Redirect communication via socket to other device/computer
  * Reorder list of smart card readers detected in a system
  * Visualize captured data in a structured way by GraphViz

See more details at https://github.com/petrs/APDUPlay/wiki.

##  Installation  
1. Find out if your targeted application is 32 or 64 bit (howto)[https://superuser.com/questions/103071/quick-way-to-tell-if-an-installed-application-is-64-bit-or-32-bit#103073]. 
1. Copy Winscard.dll from your system folder (c:\Windows\winscard.dll for 32bit target application or c:\Windows\SysWOW64\winscard.dll for 64bit application) to the folder with target application and rename it to original.dll
2. Copy Winscard.dll from APDUPlay project to the folder with target application
3. Run the application and inspect resulting files winscard_log.txt and winscard_rules_log.txt
4. (Optional) Change configuration file winscard_rules.txt to modify default behavior (see below)

The APDUPlay project provides a winscard.dll library which is stub used instead of original winscard.dll provided by Microsoft. For correct usage, you need to find  Microsoft's library and copy it to the folder with an application you are trying to control with APDUPlay. 

## Troubleshooting
NOTE: If you use (wrongly) 64bit version of library winscard.dll, it will fail with "The procedure entry point original.g_rgSCardT1Pci could not be located in the dynamic link library WinSCard.dll."  

Please, open an issue in case of any bug found. 


