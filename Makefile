CXX=g++
XPLANE_DEFINES=-DIBM=1 -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1
INCLUDES=-I/opt/XSDK/CHeaders/XPLM/ -I/opt/XSDK/CHeaders/Widgets/ -Iinclude64
LDPATHS=-L /opt/XSDK/Libraries/Win/ -L /opt/XSDK/CHeaders/Widgets/ -Linclude64 -LGLUT_for_Windows/gl
LDLIBS=-l XPLM_64 -l XPWidgets_64 -lsetupapi -llua5.1.dll -lOpenGL32 -lglut64 -lopenal.dll
LDFLAGS=--shared -static -static-libgcc -static-libstdc++ $(LDPATHS) $(LDLIBS)
CXXDEFINES=-DWIN64 -DNDEBUG -D_WINDOWS -D_MBCS -D_USRDLL -DCustomCommands_EXPORTS -DSIMDATA_EXPORTS -DUSE_EXTERNAL_ALLOCATOR
CXXFLAGS=-O3 -Wshadow -Wfloat-equal -pedantic -w -std=c++11 -m64 -fexceptions -fpermissive  $(CXXDEFINES) $(XPLANE_DEFINES) $(INCLUDES)
CFLAGS=$(CXXFLAGS)

win.xpl: FlyWithLua.o include64/hid.o FloatingWindow.o
	$(CXX) -o win.xpl FlyWithLua.o FloatingWindow.o include64/hid.o $(LDFLAGS)

clean:
	rm -f FlyWithLua.o FloatingWindow.o include64/hid.o
