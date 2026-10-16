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

  // Do we want to enable imgui debug popup?
  readOptionAsInt("ImguiDebugPopup", &imgui_debug_popup);

  // Set font size1
  readOptionAsInt("ImguiFontSize1", &imgui_font_size1);

  // Set font size2
  readOptionAsInt("ImguiFontSize2", &imgui_font_size2);

  // Set font size3
  readOptionAsInt("ImguiFontSize3", &imgui_font_size3);

  // Do we want to proggy clean to be the default?
  readOptionAsInt("ProggyCleanDefault", &proggy_clean_default);

}
