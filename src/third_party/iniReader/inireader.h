/*
 * File:   iniReader.h
 * Author: benmaynard
 *
 * Created on August 26, 2010, 2:49 PM
 */

#ifndef INIREADER_H
#define INIREADER_H

#include <algorithm>
#include <fstream>
#include <string>

/**
 * Parse a configuration file
 *
 * @param	fileName The name of the file to parse
 * @return	none
 */
void parseIniFile(char *fileName);

/**
 * If you are finished with the config item, use this function to cleanup the
 * results
 *
 * @return none
 */
void cleanupIniReader();

/**
 * Return the value of the requested key in with the string type
 *
 * @param	key The option key
 * @return	string The value of the requested key
 */
std::string getOptionToString(std::string key);

/**
 * Return the value of the requested key in with the int type
 *
 * @param	key The option key
 * @return	int The value of the requested key
 * @note	If item is not an integer (or does not exist) then 0 will be
 * returned.
 */
int getOptionToInt(std::string key);

/**
 * Write the value of the requested key to the variable provided by pointer.
 *
 * @param	key The option key
 * @return	none
 */
void readOptionAsInt(std::string key, int *value);

/**
 * Return the value of the requested key in with the char type
 *
 * @param    key The option key
 * @return   char The value of the requested key
 */
const char *getOptionToChar(std::string key);

std::string parseOptionName(std::string value);
std::string parseOptionValue(std::string value);
std::string trim(std::string s);
std::string rtrim(std::string s);
std::string ltrim(std::string s);

#endif /* INIREADER_H */
