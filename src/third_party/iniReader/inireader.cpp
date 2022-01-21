/*
 * File:   iniReader.c++
 * Author: benmaynard
 *
 * Created on August 26, 2010, 2:49 PM
 */

#include "inireader.h"
#include <cctype>
#include <functional>
#include <map>
#include <stdexcept>
#include <utility>

using namespace std;

std::map<std::string, std::string> configMap;

void parseIniFile(char *fileName) {
  std::string optionValue;
  std::ifstream infile;
  infile.open(fileName);

  // Does the file exist?
  if (!infile.is_open()) {
    return;
  }

  std::string key;

  while (!infile.eof()) // To get you all the lines.
  {
    getline(infile, optionValue); // Saves the line in STRING.

    // Is the option a comment
    if (optionValue.substr(0, 1) == "#") {
      continue;
    }

    key = parseOptionName(optionValue);

    if (key.length() > 0) {
      configMap[key] = parseOptionValue(optionValue);
    }
  }

  infile.close();
}

void cleanupIniReader() { configMap.clear(); }

std::string getOptionToString(std::string key) {
  try {
    return configMap.at(key);
  } catch (const std::out_of_range &oor) {
    return "";
  }
}

const char *getOptionToChar(std::string key) {
  try {
    return configMap.at(key).c_str();
  } catch (const std::out_of_range &oor) {
    return "";
  }
}

int getOptionToInt(std::string key) {
  try {
    return atoi(configMap.at(key).c_str());
  } catch (const std::out_of_range &oor) {
    return 0;
  }
}

void readOptionAsInt(std::string key, int *value) {
  try {
    *value = atoi(configMap.at(key).c_str());
  } catch (const std::out_of_range &oor) {
  }
}

std::string parseOptionName(std::string value) {
  size_t found;

  found = value.find('=');

  if (found > 100) {
    return "";
  }

  std::string key = value.substr(0, (found - 1));
  key = trim(key);

  return key;
}

std::string parseOptionValue(std::string value) {
  size_t found;

  found = value.find('=');

  if (found > 100) {
    return "";
  }

  std::string keyValue = value.substr((found + 1));
  keyValue = trim(keyValue);

  return keyValue;
}

std::string trim(std::string s) { return ltrim(rtrim(std::move(s))); }

// trim from start
std::string ltrim(std::string s) {
  s.erase(s.begin(),
          std::find_if(s.begin(), s.end(),
                       [](int ch) { return !std::isspace(ch); }));
  return s;
}

// trim from end
std::string rtrim(std::string s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](int ch) { return !std::isspace(ch); })
              .base(),
          s.end());
  return s;
}
