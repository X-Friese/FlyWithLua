// ****** readinifile.cpp **********
// ****  William R. Good  ********
#include "XPLMCamera.h"
#include "XPLMDataAccess.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMMenus.h"
#include "XPLMPlanes.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"

#include "inireader.h"
#include "FlyWithLua.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <cstring>

// ***** Configuration File Process ******
void flywithlua::process_read_ini_file()
{
  char *iniDefaultPluginPath;
  iniDefaultPluginPath =
      const_cast<char *>("./Resources/plugins/FlyWithLua/fwl_prefs.ini");
  cleanupIniReader();
  std::ifstream ifile(iniDefaultPluginPath);
  if (ifile) {
    XPLMDebugString("FlyWithLua: Found fwl_prefs.ini in the FlyWithLua plugin "
                        "path and it is\n");
    XPLMDebugString(iniDefaultPluginPath);
    XPLMDebugString("\n");
    parseIniFile(iniDefaultPluginPath);
  } else {
    return;
  }
  // Do we want to enable developer mode?
  readOptionAsInt("DeveloperMode", &developer_mode);

  // Do we want to enable verbose logging mode?
  readOptionAsInt("VerboseMode", &verbose_logging_mode);
}
