
#if _MSC_VER                    // compiling via MS Visual Studio
#include <windows.h>            // need to make sure this is read first
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <Carbon/Carbon.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>
#endif


