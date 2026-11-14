#ifndef FWL_SKUNKCRAFTS_H_
#define FWL_SKUNKCRAFTS_H_

#include <string>

namespace flywithlua {
namespace scu {

// Shores up FlyWithLua's own SkunkCrafts Updater configuration files (creating
// missing files on partial installs without overriding user channel choices)
// and starts the one-time background update check when CheckForUpdates != 0.
void OnPluginEnable();

// Main-thread 1 Hz tick called from MyFastLoopCallback: flushes buffered async
// log messages, syncs VR state, and displays the update alert card when ready.
void OnFlightLoopTick();

// Triggered from Plugins -> FlyWithLua -> "Check for FlyWithLua Updates...":
// launches a fresh remote check (if none is running) and opens the dialog.
void OpenUpdateStatusWindow();

// Destroys any open C++ alert ImgWindow before ImgWindow::sFontAtlas is torn down.
void CloseAlertWindow();

// Joins any background worker thread during XPluginStop before module unload.
void StopAsyncWorker();

// Updates or appends a single key = value preference in fwl_prefs.ini while
// preserving all existing comments, blank lines, and user Font.* entries.
bool UpdatePrefInIniFile(const std::string& key, const std::string& value);

} // namespace scu
} // namespace flywithlua

#endif // FWL_SKUNKCRAFTS_H_
