# Target Name
TARGET=GBConsole

# Directories
BUILDDIR=build
SOURCEDIR=source
BASEDIR=core

# Objects to Build
OBJECTSC=$(BUILDDIR)/vid.o $(BUILDDIR)/bt.o $(BUILDDIR)/usb.o $(BUILDDIR)/inp.o $(BUILDDIR)/vkey.o $(BUILDDIR)/wgc.o $(BUILDDIR)/nrf.o $(BUILDDIR)/spi.o $(BUILDDIR)/egpio.o $(BUILDDIR)/gbx.o \
	$(BUILDDIR)/gbc.o $(BUILDDIR)/gbc_cart.o $(BUILDDIR)/gbc_rom.o $(BUILDDIR)/gbc_mbc1.o $(BUILDDIR)/gbc_mbc2.o $(BUILDDIR)/gbc_mbc3.o $(BUILDDIR)/gbc_mbc5.o \
	$(BUILDDIR)/gba.o $(BUILDDIR)/gba_cart.o $(BUILDDIR)/gba_rom.o $(BUILDDIR)/gba_save.o $(BUILDDIR)/gba_sram.o $(BUILDDIR)/gba_flash.o $(BUILDDIR)/gba_eeprom.o 
OBJECTSCXX=$(BUILDDIR)/main.o $(BUILDDIR)/CSettingsManager.o $(BUILDDIR)/CSceneManager.o $(BUILDDIR)/CMenuManager.o $(BUILDDIR)/CGameManager.o \
	$(BUILDDIR)/CSceneNode.o $(BUILDDIR)/CRectSceneNode.o $(BUILDDIR)/CImageSceneNode.o $(BUILDDIR)/CTextSceneNode.o  $(BUILDDIR)/COutlineSceneNode.o

# Libraries to Include
SDLCONFIG=`sdl-config --cflags` `sdl-config --libs`
LIBRARIES=-lSDL -lSDL_image -lSDL_gfx -lSDL_ttf -lcrypto -lpthread

# Compiler
CC=gcc
CXX=g++

# Flags
CFLAGS=
CXXFLAGS=$(CFLAGS)

#===============================================================================

$(TARGET): $(OBJECTSC) $(OBJECTSCXX)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(SDLCONFIG) $(LIBRARIES)

$(BUILDDIR)/%.o : $(SOURCEDIR)/$(BASEDIR)/%.c
	$(CXX) -I$(SOURCEDIR)/$(BASEDIR) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o : $(SOURCEDIR)/$(BASEDIR)/gba/%.c
	$(CXX) -I$(SOURCEDIR)/$(BASEDIR) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o : $(SOURCEDIR)/$(BASEDIR)/gbc/%.c
	$(CXX) -I$(SOURCEDIR)/$(BASEDIR) $(CFLAGS) -c -o $@ $<
	
$(BUILDDIR)/%.o : $(SOURCEDIR)/%.cpp
	$(CXX) -I$(SOURCEDIR)/$(BASEDIR) $(CXXFLAGS) -c -o $@ $<

$(shell mkdir -p $(BUILDDIR))

clean:
	rm -f $(TARGET)
	rm -f $(BUILDDIR)/*.o

.PHONY: FORCE