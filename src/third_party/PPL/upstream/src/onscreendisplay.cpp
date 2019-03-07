// Copyright (c) 2017, Philipp Ringler philipp@x-plane.com
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are those
// of the authors and should not be interpreted as representing official policies,
// either expressed or implied, of the FreeBSD Project.

#include "XPStandardWidgets.h"
#include "XPWidgets.h"

#include "onscreendisplay.h"

using namespace PPL;

OnScreenDisplay::OnScreenDisplay(int width, int height, const std::string& title):
    screen_w_("sim/graphics/view/window_width"),
    screen_h_("sim/graphics/view/window_height"),
    vr_enabled_("sim/graphics/VR/enabled"),
    title_(title)
{
    left_ = (screen_w_ - width)/2;
    right_ = (screen_w_ + width)/2;
    top_ = (screen_h_ + height)/2 + 250;
    bottom_ = (screen_h_ - height)/2 + 250;

    widget_id_ = XPCreateWidget(left_,
                                top_,
                                right_,
                                bottom_,
                                1,
                                title_.c_str(),
                                1,
                                nullptr,
                                xpWidgetClass_MainWindow);
    XPSetWidgetProperty(widget_id_, xpProperty_MainWindowType, xpMainWindowStyle_Translucent);
    XPSetWidgetProperty(widget_id_, xpProperty_Object, reinterpret_cast<intptr_t>(this));
    XPAddWidgetCallback(widget_id_, widgetCallback);
    window_id_ = XPGetWidgetUnderlyingWindow(widget_id_);
    if (vr_enabled_ == 1)
    {
        XPLMSetWindowPositioningMode(window_id_, xplm_WindowVR, -1);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

OnScreenDisplay::~OnScreenDisplay()
{
    XPSetWidgetProperty(widget_id_, xpProperty_Object, 0);
    XPDestroyWidget(widget_id_,1);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int OnScreenDisplay::processMessages(XPWidgetMessage inMessage, intptr_t, intptr_t)
{
    if (inMessage == xpMessage_CloseButtonPushed)
    {
        XPDestroyWidget(widget_id_, 1);
        return 1;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int OnScreenDisplay::widgetCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t param1, intptr_t param2)
{
    OnScreenDisplay* display = reinterpret_cast<OnScreenDisplay*>(XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr));
    if (display)
    {
        return display->processMessages(inMessage, param1, param2);
    }
    return 0;
}
