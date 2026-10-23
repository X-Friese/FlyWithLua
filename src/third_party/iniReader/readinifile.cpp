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

  // Do we want to enable dynamic ImGui mouse cursors inside windows?
  readOptionAsInt("ImguiMouseCursors", &imgui_mouse_cursors);

  // Set font size1
  readOptionAsInt("ImguiFontSize1", &imgui_font_size1);

  // Set font size2
  readOptionAsInt("ImguiFontSize2", &imgui_font_size2);

  // Set font size3
  readOptionAsInt("ImguiFontSize3", &imgui_font_size3);

  // Do we want to proggy clean to be the default?
  readOptionAsInt("ProggyCleanDefault", &proggy_clean_default);

  // Use Custom Fonts?
  readOptionAsInt("UseCustomFonts", &use_custom_fonts);

  // Privide Custom Font One Name
  imgui_custom_font1 = getOptionToString("ImguiCustomFont1");

  // Privide Custom Font Two Name
  imgui_custom_font2 = getOptionToString("ImguiCustomFont2");

  // Privide Custom Font Three Name
  imgui_custom_font3 = getOptionToString("ImguiCustomFont3");

  // Do we want to enable script subfolder discovery?
  readOptionAsInt("EnableScriptSubfolders", &enable_script_subfolders);

  // Option A (1, default) vs Option B (0): Does subfolder script loading dynamically set SCRIPT_DIRECTORY?
  readOptionAsInt("SubfolderSetsScriptDirectory", &subfolder_sets_script_directory);
}
