SIDBlasterUSB HardSID emulation driver BETA (Windows)
-----------------------------------------------------

The driver is a drop-in replacement for hardsid.dll and should be compatible
with most programs that support HardSID playback. The old driver program that
came with the device (SIDBlaster.exe) must be disabled for the driver to work
properly.

The driver has been tested with ACID64Player Pro v3.6 Beta 5, Sidplay2, Sidplay26/W and VICE2.4

As of 13.02.2015, the release version of ACID64Player does not support the SIDBlasterUSB 
device, and you will need at least the 3.60 Beta 5 build for proper support. This can be
downloaded from: 

http://www.acid64.com/download/beta/acid64_player_pro_v360_beta5.zip

This is partly due to the hardsid detection method used by older versions of the program. 

Depending on your system, tunes with high data rates (multispeed tunes or digitunes) may 
play slower if the latency of the USB driver is too high. This can be remedied by setting
the driver write buffer size to a larger value, by setting the environment variable 
SIDBLASTERUSB_WRITEBUFFER_SIZE like for instance:

SET SIDBLASTERUSB_WRITEBUFFER_SIZE=8

A larger write buffer may adversely effect the sound quality in tunes that need cycle exact 
emulation since writes will be batched together.

I have created a test archive with some players and tunes, some normal speed, some multispeed,
and a couple of digi-tunes. This can be downloaded from csdb.

I have implemented multi-device support, but since I only have a single SIDBlaster, this has
not been properly tested.

I welcome feedback on all aspects of the driver. Other programs, multi-device etc.

Thanks to:

* Wilfred Bos for his assistance in developing this driver and also for extending his
already awesome ACID64Player with SIDBlasterUSB support

* Isildur for testing

* Davey for creating this awesome device.

Oslo, 13.02.2015

stein.pedersen@gmail.com

