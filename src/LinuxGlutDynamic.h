#ifndef FWL_LINUX_GLUT_DYNAMIC_H
#define FWL_LINUX_GLUT_DYNAMIC_H

#ifdef __linux__

// Distinct tag addresses representing the 5 legacy GLUT bitmap fonts used by FlyWithLua.
// Mapped at runtime to the real FreeGLUT font symbols resolved via dlopen/dlsym.
extern int fwl_glut_tag_times_10;
extern int fwl_glut_tag_times_24;
extern int fwl_glut_tag_helv_10;
extern int fwl_glut_tag_helv_12;
extern int fwl_glut_tag_helv_18;

#define GLUT_BITMAP_TIMES_ROMAN_10 (static_cast<void*>(&fwl_glut_tag_times_10))
#define GLUT_BITMAP_TIMES_ROMAN_24 (static_cast<void*>(&fwl_glut_tag_times_24))
#define GLUT_BITMAP_HELVETICA_10   (static_cast<void*>(&fwl_glut_tag_helv_10))
#define GLUT_BITMAP_HELVETICA_12   (static_cast<void*>(&fwl_glut_tag_helv_12))
#define GLUT_BITMAP_HELVETICA_18   (static_cast<void*>(&fwl_glut_tag_helv_18))

// Lazily loads FreeGLUT (trying libglut.so.3, libglut.so.3.13, libglut.so.3.12, etc.)
// and calls glutInit() once on first legacy GLUT font request. Returns true if FreeGLUT
// is available, or false if FreeGLUT is not installed (allowing XPLM text fallback).
bool fwlLinuxGlutEnsureReady();

void glutInit(int* argcp, char** argv);
void glutBitmapCharacter(void* font, int character);
int  glutBitmapWidth(void* font, int character);

#endif // __linux__

#endif // FWL_LINUX_GLUT_DYNAMIC_H
