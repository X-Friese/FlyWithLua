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

#ifndef MESSAGEWINDOW_H
#define MESSAGEWINDOW_H

#include <string>
#include <list>
#include <vector>

#include "XPWidgets.h"
#include "XPStandardWidgets.h"

#include "dataref.h"

namespace PPL {

/**
  * A convenient way to display a message box with a title and a longer text message
  * that quits X-Plane when closed. This is a very good way to handle improper setups,
  * blame the user, and force quit afterwards
  * @version 1.0
  * @author (c) 2009-2011 by Philipp Muenzel
  */
class MessageWindow
{
public:
    /**
      * This exception is thrown during setup of a Message box, if the requested
      * size of the box is too large to fit in the simulator window.
      */
    class BoxOutOfBoundsException: public std::runtime_error {
    public:
        BoxOutOfBoundsException(const std::string& msg):
            std::runtime_error(msg)
        {}
    };

    /**
      * construct message box centered in the X-Plane window, with title and auto-breaking message
      * @param width in pixels
      * @param height in pixels
      * @param title text in the titlebar
      * @param message text gets automatically br'd according to window width
      * @param quit whether the simulator should be closed after the message was confirmed
      */
    MessageWindow(int width, int height, const std::string& title, const std::string& message, bool quit);

    /**
      * destroy window and all child windows
      */
    ~MessageWindow();

    MessageWindow(const MessageWindow&) = delete;
    MessageWindow& operator=(const MessageWindow&) = delete;

    /**
      * @param width in pixels
      * @exception BoxOutOfBoundsException if width is inappropriate
      */
    void setWidth(int width);

    /**
      * @param height in pixels
      * @exception BoxOutOfBoundsException if height is inappropriate
      */
    void setHeight(int height);

    /**
      * @param title to show in the title bar of the message box
      */
    void setTitle(const std::string& title);

    /**
      * @param message to show in the message box
      */
    void setMessage(const std::string& message);

    /**
      * @param quit quit the sime after the user confirms the dialog? yes or no
      */
    void quitSimOnConfirm(bool quit);

    /**
      * display the window in the sim, forces user to acknownledge the dialog
      */
    void display();

    /**
      * @return is the window currently drawn by X-Plane
      */
    bool isDisplayed();

    /**
      * internal processing of message from X-Plane's widget system
      * it was already figured out that we are adressed
      * @param message
      * @param param1
      * @param param2
      */
    int processMessages(XPWidgetMessage message, intptr_t param1, intptr_t param2);

    /**
      * static widget callback to register in X-Plane's widget logic
      * retrieves the instance via a pointer-to-object stored in the widget struct
      */
    static int widgetCallback(XPWidgetMessage, XPWidgetID, intptr_t, intptr_t);

private:

    /**
      * split a string in words and store them in the list
      * @param L list of strings where words are stored
      * @param seq original string to split up
      * @param _1cdelim string that declares a splitting point
      * @param _removews remove white spaces
      */
    int splitStr(std::list<std::string>& L, const std::string& seq, const std::string& _1cdelim, bool _removews );

    /**
      * create the box widget with title bar and close buttons
      */
    void createSurroundingBox();

    /**
      * create the inner panel on which strings are drawn
      */
    void createInnerScreen();

private:

    int m_left;             //!< upper left corner left offset in pixels from X-Plane main window

    int m_right;            //!< upper right corner right offset in pixels from X-Plane main window

    int m_top;              //!< upper left corner upper offset in pixels from X-Plane main window

    int m_bottom;           //!< lower left corner bottom offset in pixels from X-Plane main window

    std::string m_title;    //!< title bar caption

    std::string m_message;  //!< message to display

    bool m_quit_on_confirm; //!< quit X-Plane when user closes message box

    bool m_is_displayed;

    XPWidgetID m_box_widget;

    XPWidgetID m_screen_widget;

    std::vector<XPWidgetID> m_caption_widgets_list;

    DataRef<int> m_xp_width;   //!< X-Plane main window width

    DataRef<int> m_xp_height;  //!< X-Plane main window height
};

}

#endif // MESSAGEWINDOW_H
