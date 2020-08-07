//-----------------------------------------------------------------------------------------
// Title:	Game Manager
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "CSettingsManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <stdlib.h>

//data constants
static const char* stm_settingsFile = "data/settings.txt";

//system wide settings
static const char* stm_resolutionSetting = "system.resolution";
static const char* const stm_resolutionOptions[] = { "auto", "1920x1080", "1280x720", 0 };

//! Main constructor
CSettingsManager::CSettingsManager()
{
	std::ifstream in(stm_settingsFile);
	std::ostringstream ss;
	ss << in.rdbuf();
	fileData = ss.str();
	in.close();
	
	//init system settings
	char resolution[128];
	this->getPropertyString(stm_resolutionSetting, stm_resolutionOptions[0], resolution, 128);
	selectedResolution = 0;
	for(int i=0; stm_resolutionOptions[i]; i++) if(strcmp(stm_resolutionOptions[i], resolution)==0) { selectedResolution = i; break; }
	this->setResolution(selectedResolution);
}

//! Destructor
CSettingsManager::~CSettingsManager()
{
}

//! Gets the string value for the given property
void CSettingsManager::getPropertyString(const char* property, const char* defVal, char* out, int max)
{
	if(defVal) std::strcpy(out, defVal);
	else for(int i=0; i<max; i++) out[i] = 0;
	
	if(fileData.size() > 0) {
		int propertySize = 0;
		while(property[propertySize]) propertySize++;

		//search
		for(unsigned int i=0; i<fileData.size()-propertySize; i++) {

			//ignore any lines that are commented with '#'
			for(unsigned int j=i; j<fileData.size()-propertySize && fileData[j] != 13 && fileData[j] != 10; j++) {
				if(fileData[j] == '#') {
					//ignore
					while(i<fileData.size()-propertySize && fileData[i] != 13 && fileData[i] != 10) i++;
					break;
				} else if(fileData[j] != ' ') {
					//valid
					break;
				}
			}

			//check for match
			bool match = true;
			for(int j=0; j<propertySize; j++) {
				if(fileData[i+j] != property[j]) {
					match = false;
					break;
				}
			}
			if(match) {
				unsigned int index = i + propertySize;

				//look for '='
				bool found = false;
				while(index < fileData.size()) {
					if(fileData[index] == '=') {
						found = true;
						break;
					} else if(fileData[index] != ' ') {
						break;
					}
					index++;
				}
				if(found) {
					index++;

					//trim leading white space
					while(fileData[index] == ' ' && index < fileData.size()) index++;

					//copy to output
					signed int j = 0;
					for(; j<max-1 && index+j < fileData.size() && fileData[index+j] != 13 && fileData[index+j] != 10; j++) out[j] = fileData[index+j];
					out[j] = 0;

					//trim trailing whitespace
					for(j--; j>-1 && out[j] == ' '; j--) out[j] = 0;

					break;
				}
			}
		}
	}
}

//! Gets the integer value for the given property
signed int CSettingsManager::getPropertyInteger(const char* property, signed int defVal)
{
	if(fileData.size() > 0) {
		char val[32];
		getPropertyString(property, "null", val, 32);
		
		//check if null
		if(strcmp(val, "null")==0) return defVal;

		//get leading sign
		int start = 0;
		signed int sign = 1;
		if(val[0] == '-') { start = 1; sign = -1; }
		if(val[0] == '+') { start = 1; }

		//check if valid integer
		for(int i=start; i<32; i++) {
			if(val[i]==0) break;
			if(val[i] < 48 || val[i] > 57) return defVal;
		}

		//get integer
		signed int value = 0;
		for(int i=start; val[i] != 0; i++) value = (value*10) + (val[i]-48);
		return value*sign;
	}
	return defVal;
}

//! Sets the string value for the given property
void CSettingsManager::setPropertyString(const char* property, const char* value)
{
	int propertySize = 0;
	while(property[propertySize]) propertySize++;
	signed int start = -1;
	signed int end = -1;

	//search
	if(fileData.size() > 0) {
		for(unsigned int i=0; i<fileData.size()-propertySize; i++) {

			//ignore any lines that are commented with '#'
			for(unsigned int j=i; j<fileData.size()-propertySize && fileData[j] != 13 && fileData[j] != 10; j++) {
				if(fileData[j] == '#') {
					//ignore
					while(i<fileData.size()-propertySize && fileData[i] != 13 && fileData[i] != 10) i++;
					break;
				} else if(fileData[j] != ' ') {
					//valid
					break;
				}
			}

			//check for match
			bool match = true;
			for(int j=0; j<propertySize; j++) {
				if(fileData[i+j] != property[j]) {
					match = false;
					break;
				}
			}
			if(match) {
				unsigned int index = i + propertySize;

				//look for '='
				bool found = false;
				while(index < fileData.size()) {
					if(fileData[index] == '=') {
						found = true;
						break;
					} else if(fileData[index] != ' ') {
						break;
					}
					index++;
				}
				if(found) {
					index++;
					start = index;
					while(index < fileData.size() && fileData[index] != 13 && fileData[index] != 10) index++;
					end = index;

					break;
				}
			}
		}
	}
	
	if(start > -1 && end > -1) {
		
		//update setting line
		fileData = fileData.substr(0,start) + ' ' + value + fileData.substr(end,fileData.size());
	} else {
		
		//add setting line
		if(fileData.size() > 0) fileData += "\r\n";
		fileData = fileData + property + "= " + value;
	}
	saveFile();
}

//! Sets the integer value for the given property
void CSettingsManager::setPropertyInteger(const char* property, signed int value)
{
	if(fileData.size() > 0) {
		char val[32];
		for(int i=0; i<32; i++) val[i] = 0;
		
		std::stringstream ss; int x = 23;
		ss << value;
		std::string str = ss.str();
		for(int i=0; i<str.size(); i++) val[i] = str[i];

		setPropertyString(property, val);
	}
}
	
//! Gets the resolution option at the given index	
const char* CSettingsManager::getResolutionOption(int index)
{
	int numOptions = getNumResolutionOptions();
	if(index > -1 && index < numOptions) return stm_resolutionOptions[index];
	return 0;	
}

//! Gets the number of resolution options
int CSettingsManager::getNumResolutionOptions()
{
	int numOptions = 0;
	for(int i=0; stm_resolutionOptions[i]; i++) numOptions++;
	return numOptions;
}

//! Gets the set resolution
int CSettingsManager::getResolution()
{
	return selectedResolution;
}

//! Sets the system resolution
void CSettingsManager::setResolution(int index)
{
	int numOptions = getNumResolutionOptions();
	if(index < 0) index = numOptions-1; 
	if(index > numOptions-1) index = 0; 
	selectedResolution = index;
	
	if(selectedResolution == 0) system("sudo raspi-config nonint do_resolution 0 0");
	else if(selectedResolution == 1) system("sudo raspi-config nonint do_resolution 1 16");
	else if(selectedResolution == 2) system("sudo raspi-config nonint do_resolution 1 4");
	this->setPropertyString(stm_resolutionSetting, stm_resolutionOptions[selectedResolution]);
}

//! Saves the settings file
void CSettingsManager::saveFile()
{
	if(fileData.size() > 0) {
		std::ofstream out(stm_settingsFile);
		out << fileData;
		out.close();
	}
}
