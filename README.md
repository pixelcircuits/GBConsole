# GameBoy Console
The GameBoy Console is an emulator that plays physical GameBoy, GameBoy Color and GameBoy Advance cartridges on your TV! The goal of this project is to provide an inexpensive solution for revisiting beloved GameBoy games on the big screen and help preserve cartridge save data.

<img src="https://raw.githubusercontent.com/pixelcircuits/GBConsole_Documentation/master/guide/img/gbconsole_img2.jpg" width="400" hspace="4"/><img src="https://raw.githubusercontent.com/pixelcircuits/GBConsole_Documentation/master/guide/img/gbconsole_dev2.jpg" width="400" hspace="4"/><img src="https://raw.githubusercontent.com/pixelcircuits/GBConsole_Documentation/master/guide/img/screen_cartridge.png" width="400" hspace="4"/><img src="https://raw.githubusercontent.com/pixelcircuits/GBConsole_Documentation/master/guide/img/screen_catalog.png" width="400" hspace="4"/>

## Software Setup
Please follow these steps to install and setup the GameBoy Console on a Raspberry Pi Zero or Raspberry Pi Zero W (other Raspberry Pi versions not yet supported). You will need the appropriate hardware connected to the GPIO of the Raspberry Pi in order to read cartridges. Refer to the additional documentation [here](https://github.com/pixelcircuits/GBConsole_Documentation) for information on setting up the hardware.

### 1. Install and Setup Raspbian Lite
a) The first thing to do is start with a fresh SD card with [Raspbian Lite](https://www.raspberrypi.org/downloads/raspbian/) loaded. Refer to [this tutorial](https://www.raspberrypi.org/documentation/installation/installing-images/README.md) for getting Raspbian up and running (last known working version is 2020-02-13)

b) Boot up the Raspberry Pi and from the command line run the raspi configuration tool and set the following options
```
sudo raspi-config
	Network Options -> Wi-Fi (setup)
	Interfacing Options -> SPI (enable)
	Advanced Options -> Overscan (disable)
	Advanced Options -> Memory Split (256)
```
c) Reboot the Raspberry Pi and run the following commands to make sure the package manager is up to date
```
sudo apt-get update
sudo apt-get dist-upgrade -y
sudo apt-get install -y git
```

### 2. Install Emulators from RetroPie
a) Run the following commands to download the RetroPie seutp script
```
cd /home/pi
git clone --depth=1 https://github.com/RetroPie/RetroPie-Setup.git
```
b) Run the RetroPie seutp script and install the following packages (be patient, this will take a while...)
```
cd /home/pi/RetroPie-Setup
sudo ./retropie_setup.sh
	Manage packages -> Manage core packages -> retroarch -> Install from binary
	Manage packages -> Manage main packages -> lr-gambatte -> Install from binary
	Manage packages -> Manage main packages -> lr-gpsp -> Install from binary
	Manage packages -> Manage main packages -> lr-mgba -> Install from binary
```

### 3. Build and Install GameBoy Console
a) Run the following commands to install dependencies, download the source code and build the GameBoy Console
```
cd /home/pi
sudo apt-get install -y libsdl1.2-dev libsdl-image1.2-dev libsdl-gfx1.2-dev libsdl-ttf2.0-dev libssl-dev
git clone https://github.com/pixelcircuits/GBConsole.git
cd GBConsole
make
```
b) To have the application run on boot add the following lines to rc.local by running the nano text editor (press ctrl+x to exit and make sure to save)
```
sudo nano /etc/rc.local
	cd GBConsole
	sudo ./GBConsole
```

### 4. Download Box Art and Screen Captures
a) Run the following commands to download box art and screen captures from the libretro repository used to enhance the the UI (be patient, this will take a while...)
```
cd /home/pi
mkdir libretro
wget https://github.com/libretro-thumbnails/Nintendo_-_Game_Boy/archive/master.zip
unzip master.zip
mv /home/pi/Nintendo_-_Game_Boy-master /home/pi/libretro/gb
rm master.zip
wget https://github.com/libretro-thumbnails/Nintendo_-_Game_Boy_Color/archive/master.zip
unzip master.zip
mv /home/pi/Nintendo_-_Game_Boy_Color-master /home/pi/libretro/gbc
rm master.zip
wget https://github.com/libretro-thumbnails/Nintendo_-_Game_Boy_Advance/archive/master.zip
unzip master.zip
mv /home/pi/Nintendo_-_Game_Boy_Advance-master /home/pi/libretro/gba
rm master.zip
```

### 5. Troubleshooting
If you find yourself having issues with any of the emulator cores you can install them from a previously known working binary. Run the following commands for the problem cores.
##### lr-mgba
```
wget https://raw.githubusercontent.com/pixelcircuits/GBConsole_Documentation/master/bin/lr-mgba.zip
unzip lr-mgba.zip -d /home/pi/lr-mgba
sudo rm -r /opt/retropie/libretrocores/lr-mgba
sudo mv /home/pi/lr-mgba /opt/retropie/libretrocores/lr-mgba
rm /home/pi/lr-mgba.zip
```
##### lr-gpsp
```
wget https://raw.githubusercontent.com/pixelcircuits/GBConsole_Documentation/master/bin/lr-gpsp.zip
unzip lr-gpsp.zip -d /home/pi/lr-gpsp
sudo rm -r /opt/retropie/libretrocores/lr-gpsp
sudo mv /home/pi/lr-gpsp /opt/retropie/libretrocores/lr-gpsp
rm /home/pi/lr-gpsp.zip
```
##### lr-gambatte
```
wget https://raw.githubusercontent.com/pixelcircuits/GBConsole_Documentation/master/bin/lr-gambatte.zip
unzip lr-gambatte.zip -d /home/pi/lr-gambatte
sudo rm -r /opt/retropie/libretrocores/lr-gambatte
sudo mv /home/pi/lr-gambatte /opt/retropie/libretrocores/lr-gambatte
rm /home/pi/lr-gambatte.zip
```

