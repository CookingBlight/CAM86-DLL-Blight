# CAM86-DLL-Blight
Custom low level driver for CAM86 DIY camera (Sony ICX453AQ sensor) re-written in C++ instead of delphi in Visual Studio.

This project is based on work of Ukrainian telescope amateurs.
Most of their work is shared on this huge [forum thread](http://www.astroclub.kiev.ua/forum/index.php?topic=28929.0).

The current work is described [here](http://www.iceinspace.com.au/forum/showthread.php?t=146493).

The source code for the camera firmware can be found [here](https://github.com/CookingBlight/CAM86-FW-Blight) while the compatible ASCOM driver can be found [here](https://github.com/CookingBlight/CAM86-ASCOM-Blight).

# See also
[Information in Ukrainian](http://astroccd.org/)

[French forum](http://www.webastro.net/forum/showthread.php?t=141764)

[Original Indi driver for Cam86](https://github.com/gehelem/indi_cam86_ccd)

[Cam84/Cam86 forum](http://www.cloudynights.com/topic/497530-diy-astro-ccd-16-bit-color-6mpx-camera/)


# Authors:
Gilmanov Rim (Гильманов Рим) - original camera hardware and low-level camera interaction

Sergiy Vakulenko - Original ASCOM driver 

Their code is shared on [Sergiy Vakulenko 's Github](https://github.com/vakulenko/CAM8_software)


Luka Pravica - current modifications/extensions/customisations to the firmware, low-level driver and ASCOM driver

Tommy Ramberg - Converted dll from delphi to c++ in Visual Studio.

All derived work from this code must be kept free and opensource as governed by GPL v2 or later (see the included file COPYING).
