# ESP32-Dot-Bit-Lookup-Tool
Record Query and QR Code generator for The .bit DID service running on the Nervos Network.

Has been designed to run on an ESP32 HMI (Human Machine Interface) which is an all in one ESP32 and IL9241 Touch display board. It uses LovyanGFX (which has dependancy on TFT_eSPI), its highly likely it'll work fine on a separate ESP32 and IL9341 as well as any other 320x240 TFT with a bit of setup tweaking but i haven't catered for this and you need some knowledge to make this happen. The combined MCU and display i used can be found here https://a.aliexpress.com/_mtlIxW4 or here https://www.makerfabs.com/sunton-esp32-2-8-inch-tft-with-touch.html or a few other online retailers. The Aliexpress link is direct and makerfabs seem to be a well priced resller. I have no affiliation with either.

The Code has been written using the Arduino 2.1 IDE but should work fine on the legacy version or on Platform.IO. There are several required libraries to make the sketch work. Also required is an SD card as this is where both the GUI images and Saved Records/Bitmaps will be stored. 8GB should be enough to store several thousand .bit records and their associated bitmap QR files, i beleive the ESP32 can handle a maximum size of 32GB but please refer to the manufacturers specs. The code is also desined to utilise the speaker out JST connector on the HMI board giving audible feedback to touch input but the sketch will run fine without having this speaker attached. I've used a 1w speaker which is a little on the loud side but i've yet to integrate anything in the way of volume control. Be aware the draw of a 1 watt speaker can make the screen dim if not using a sufficient power source, not debilitating but noticable.

Demo Video of the ESP-Dot-Bit-Lookup-Tool
https://youtu.be/i9GtNlogzb0

For a guide to setting up your ESP32-HMI device go here and see the setup guide by MakerFabs https://wiki.makerfabs.com/Sunton_ESP32_2.8_inch_240x320_TFT_with_Touch.html

The above guide recommends Arduino 1.8, if you're just starting out and haven't got a personal preference of IDE i would recommend going with the newer 2.1 IDE as it adds several valuable features included improved highlighting, auto-promting for functions and definition lookups.

All the required libraries can be installed via the library manager in Arduino they are as follows in no particular order;

Time by Michael Margolis
ArduinoJson by Benoit Blanchon
ESP32Time by fbiego
ESP Mail Client by Mobizt
LovyanGFX by lovyan03
TFT_eSPI by Bodmer

To install the sketch copy the contents of "src" to a new folder in your main Arduino folder called "dotBitSearch_v1". Then you need to copy the contents of "images" (just the folders inside not the images folder itself) to you FAT32 formatted SD card. Two additional folders will be created when saving "qrcodes" and "records". "qrcodes" will hold the generated bitmaps, while "records" holds the saved .json files of .bit records.

Upon initial startup there are a couple things to remember, you must go to settings and setup your WiFi details, the tool has built in WiFi scanning so you just need to select your network and then enter the password. While not connected to WiFi the tool will attempt to auto reconnect every 30 seconds once you have entered a password, during this time the screen will be inoperable/unresponsive for up to 5 seconds while establishing connection. Sending QR codes via email requires you to input your email account details aswell as the SMTP server details for your account. These details are stored within the ESP32's EEPROM upon saving. 


