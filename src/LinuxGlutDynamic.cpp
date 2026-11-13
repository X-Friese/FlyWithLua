#ifdef __linux__

#include "LinuxGlutDynamic.h"
#include "XPLMGraphics.h"
#include "XPLMUtilities.h"

#include <cstring>
#include <dlfcn.h>
#include <glob.h>
#include <string>

int fwl_glut_tag_times_10 = 10;
int fwl_glut_tag_times_24 = 24;
int fwl_glut_tag_helv_10  = 110;
int fwl_glut_tag_helv_12  = 112;
int fwl_glut_tag_helv_18  = 118;

namespace {

using PFN_glutInit            = void (*)(int*, char**);
using PFN_glutBitmapCharacter = void (*)(void*, int);
using PFN_glutBitmapWidth     = int (*)(void*, int);

bool                    s_loadAttempted       = false;
bool                    s_glutReady           = false;
void*                   s_glutHandle          = nullptr;
PFN_glutInit            s_glutInit            = nullptr;
PFN_glutBitmapCharacter s_glutBitmapCharacter = nullptr;
PFN_glutBitmapWidth     s_glutBitmapWidth     = nullptr;

void* s_fontTimes10 = nullptr;
void* s_fontTimes24 = nullptr;
void* s_fontHelv10  = nullptr;
void* s_fontHelv12  = nullptr;
void* s_fontHelv18  = nullptr;

template <typename Fn>
bool resolveFn(void* handle, const char* symName, Fn& outFn)
{
    static_assert(sizeof(Fn) == sizeof(void*), "Function pointer size mismatch");
    void* raw = dlsym(handle, symName);
    if (!raw)
        return false;
    std::memcpy(&outFn, &raw, sizeof(Fn));
    return true;
}

bool tryLoadCandidate(const char* soName)
{
    void* handle = dlopen(soName, RTLD_LAZY | RTLD_LOCAL);
    if (!handle)
        return false;

    PFN_glutInit            fnInit  = nullptr;
    PFN_glutBitmapCharacter fnChar  = nullptr;
    PFN_glutBitmapWidth     fnWidth = nullptr;

    void* fTimes10 = dlsym(handle, "glutBitmapTimesRoman10");
    void* fTimes24 = dlsym(handle, "glutBitmapTimesRoman24");
    void* fHelv10  = dlsym(handle, "glutBitmapHelvetica10");
    void* fHelv12  = dlsym(handle, "glutBitmapHelvetica12");
    void* fHelv18  = dlsym(handle, "glutBitmapHelvetica18");

    if (!resolveFn(handle, "glutInit", fnInit) ||
        !resolveFn(handle, "glutBitmapCharacter", fnChar) ||
        !resolveFn(handle, "glutBitmapWidth", fnWidth) ||
        !fTimes10 || !fTimes24 || !fHelv10 || !fHelv12 || !fHelv18)
    {
        dlclose(handle);
        return false;
    }

    s_glutHandle          = handle;
    s_glutInit            = fnInit;
    s_glutBitmapCharacter = fnChar;
    s_glutBitmapWidth     = fnWidth;
    s_fontTimes10         = fTimes10;
    s_fontTimes24         = fTimes24;
    s_fontHelv10          = fHelv10;
    s_fontHelv12          = fHelv12;
    s_fontHelv18          = fHelv18;

    int   argc    = 1;
    char  app[]   = "FlyWithLua";
    char* argv[2] = {app, nullptr};
    s_glutInit(&argc, argv);

    s_glutReady = true;
    std::string msg = std::string("FlyWithLua Info: Dynamically loaded FreeGLUT runtime (") + soName + ").\n";
    XPLMDebugString(msg.c_str());
    return true;
}

void* mapFontTag(void* tag)
{
    if (tag == GLUT_BITMAP_HELVETICA_12)
        return s_fontHelv12;
    if (tag == GLUT_BITMAP_HELVETICA_18)
        return s_fontHelv18;
    if (tag == GLUT_BITMAP_HELVETICA_10)
        return s_fontHelv10;
    if (tag == GLUT_BITMAP_TIMES_ROMAN_10)
        return s_fontTimes10;
    if (tag == GLUT_BITMAP_TIMES_ROMAN_24)
        return s_fontTimes24;
    return s_fontHelv12;
}

} // namespace

bool fwlLinuxGlutEnsureReady()
{
    if (s_loadAttempted)
        return s_glutReady;
    s_loadAttempted = true;

    // 1. Standard SONAMEs across Linux distributions:
    //    - libglut.so.3    (Debian 11/12, Ubuntu 20.04/22.04, Arch, Fedora, openSUSE)
    //    - libglut.so.3.13 (FreeGLUT 3.6+/3.8+ split SONAME)
    //    - libglut.so.3.12 (Ubuntu 24.04 / Mint 22 libglut3.12 where .so.3 symlink was omitted)
    //    - libglut.so      (development package symlink)
    static const char* const kCandidates[] = {
        "libglut.so.3",
        "libglut.so.3.13",
        "libglut.so.3.12",
        "libglut.so"
    };

    for (const char* name : kCandidates)
    {
        if (tryLoadCandidate(name))
            return true;
    }

    // 2. Future-proof wildcard scan for any libglut.so.3.* installed in standard 64-bit library paths:
    static const char* const kGlobPatterns[] = {
        "/usr/lib/x86_64-linux-gnu/libglut.so.3*",
        "/lib/x86_64-linux-gnu/libglut.so.3*",
        "/usr/lib64/libglut.so.3*",
        "/lib64/libglut.so.3*",
        "/usr/lib/libglut.so.3*",
        "/lib/libglut.so.3*"
    };

    for (const char* pattern : kGlobPatterns)
    {
        glob_t gl{};
        if (glob(pattern, GLOB_NOSORT, nullptr, &gl) == 0)
        {
            for (size_t i = 0; i < gl.gl_pathc; ++i)
            {
                if (tryLoadCandidate(gl.gl_pathv[i]))
                {
                    globfree(&gl);
                    return true;
                }
            }
            globfree(&gl);
        }
    }

    XPLMDebugString("FlyWithLua Info: FreeGLUT (libglut.so.3*) not found on system; using XPLM font fallback for legacy GLUT text.\n");
    return false;
}

void glutInit(int* argcp, char** argv)
{
    (void)argcp;
    (void)argv;
    // Intentionally deferred: FreeGLUT is loaded and initialized lazily on first legacy GLUT font call.
}

void glutBitmapCharacter(void* font, int character)
{
    if (!fwlLinuxGlutEnsureReady())
        return;

    void* realFont = mapFontTag(font);
    if (realFont && s_glutBitmapCharacter)
        s_glutBitmapCharacter(realFont, character);
}

int glutBitmapWidth(void* font, int character)
{
    if (fwlLinuxGlutEnsureReady())
    {
        void* realFont = mapFontTag(font);
        if (realFont && s_glutBitmapWidth)
            return s_glutBitmapWidth(realFont, character);
    }

    char ch[2] = {static_cast<char>(character), '\0'};
    return static_cast<int>(XPLMMeasureString(xplmFont_Proportional, ch, 1) + 0.5f);
}

#endif // __linux__
