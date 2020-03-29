//-----------------------------------------------------------------------------------------
// Title:	Settings Manager
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H
#include <string>

//! Manages Settings Data
class CSettingsManager
{
public:
	//! Main Constructor
	CSettingsManager();

	//! Destructor
	~CSettingsManager();
	
	//! Gets the string value for the given property
	void getPropertyString(const char* property, const char* defVal, char* out, int max);

	//! Gets the integer value for the given property
	signed int getPropertyInteger(const char* property, signed int defVal);
	
	//! Sets the string value for the given property
	void setPropertyString(const char* property, const char* value);

	//! Sets the integer value for the given property
	void setPropertyInteger(const char* property, signed int value);
	
	//! Gets the resolution option at the given index	
	const char* getResolutionOption(int index);
	
	//! Gets the number of resolution options
	int getNumResolutionOptions();
	
	//! Gets the set resolution
	int getResolution();
	
	//! Sets the system resolution
	void setResolution(int index);
	
private:
	std::string fileData;
	int selectedResolution;

	//Util functions
	void saveFile();
};

#endif
