AppleIntelInfo.kext
===================

What do I need to do?

You first need to download and then compile AppleIntelInfo with Xcode.

How do I load AppleIntelInfo.kext?

``` sh
sudo kextload AppleIntelInfo.kext

or with

sudo kextutil AppleIntelInfo.kext 
```
Warning: Do not copy the kext to: /System/Library/Extensions or /Library/Extensions and do not inject it with help of the boot loader!

How do I unload AppleIntelInfo.kext?

``` sh
sudo kextunload AppleIntelInfo.kext
```

Where can I find the output?


The output of AppleIntelInfo.kext can be found with
``` sh
sudo cat /tmp/AppleIntelInfo.dat
```

Note: This has changed in version 1.2 (data no longer added to /var/log/system.log)

Settings
--------

There are five (5) settings that you can change in AppleIntelInfo.kext/Contents/Info.plist:
```
logCStates
logIGPU
logIPGStyle
logIntelRegs
logMSRs
```

All set to YES by default.

Bugs
----

All possible bugs (so called 'issues') should be filed at:

https://github.com/Piker-Alpha/AppleIntelInfo/issues

Please do **not** use my blog for this. Thank you!
