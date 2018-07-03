CXX=g++
XPLANE_DEFINES=-DIBM=1 -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1
INCLUDES=-I/opt/XSDK/CHeaders/XPLM/ -I/opt/XSDK/CHeaders/Widgets/ -Iinclude64
LDPATHS=-L /opt/XSDK/Libraries/Win/ -L /opt/XSDK/CHeaders/Widgets/ -Linclude64 -LGLUT_for_Windows/gl
LDLIBS=-l XPLM_64 -l XPWidgets_64 -lsetupapi -llua5.1.dll -lOpenGL32 -lglut64 -lopenal.dll
LDFLAGS=--shared -static -static-libgcc -static-libstdc++ $(LDPATHS) $(LDLIBS)
CXXDEFINES=-DWIN64 -DNDEBUG -D_WINDOWS -D_MBCS -D_USRDLL -DCustomCommands_EXPORTS -DSIMDATA_EXPORTS -DUSE_EXTERNAL_ALLOCATOR -DCREATECOMPLETEEDITION
CXXFLAGS=-O3 -Wshadow -Wfloat-equal -pedantic -w -std=c++11 -m64 -fexceptions -fpermissive  $(CXXDEFINES) $(XPLANE_DEFINES) $(INCLUDES)
CFLAGS=$(CXXFLAGS)

all: win.xpl

win.xpl: FlyWithLua.o include64/hid.o FloatingWindows/FLWIntegration.o FloatingWindows/ImGUIIntegration.o FloatingWindows/FloatingWindow.o FloatingWindows/imgui/imgui.o FloatingWindows/imgui/imgui_demo.o FloatingWindows/imgui/imgui_draw.o FloatingWindows/imgui/imgui_lua_bindings.o
	$(CXX) -o win.xpl FlyWithLua.o FloatingWindows/FLWIntegration.o FloatingWindows/ImGUIIntegration.o FloatingWindows/FloatingWindow.o include64/hid.o FloatingWindows/imgui/imgui.o FloatingWindows/imgui/imgui_demo.o FloatingWindows/imgui/imgui_draw.o FloatingWindows/imgui/imgui_lua_bindings.o $(LDFLAGS)

install: win.xpl
	cp win.xpl /c/Users/rme/Desktop/X-Plane\ 11/Resources/plugins/FlyWithLua/64/

clean:
	rm -f FlyWithLua.o FloatingWindows/FloatingWindow.o include64/hid.o FloatingWindows/imgui/imgui.o FloatingWindows/imgui/imgui_demo.o FloatingWindows/imgui/imgui_draw.o FloatingWindows/imgui/ImgWindow.o FloatingWindows/imgui/imgui_lua_bindings.o
