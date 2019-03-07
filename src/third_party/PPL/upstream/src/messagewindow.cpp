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

#include <cstdlib>

#include "messagewindow.h"

using namespace PPL;

MessageWindow::MessageWindow(int width, int height, const std::string& title, const std::string& message, bool quit):
        m_is_displayed(false),
        m_xp_width("sim/graphics/view/window_width"),
        m_xp_height("sim/graphics/view/window_height")
{
    try
    {
        setWidth(width);
        setHeight(height);
    } catch (BoxOutOfBoundsException& ex)
    {
        Log() << Log::Error << "Cannot create widget with dimensions: " << ex.what() << Log::endl;
        try {
            setWidth(m_xp_width - 100);
            setHeight(m_xp_height - 100);
        } catch(BoxOutOfBoundsException&)
        {
            Log() << Log::Info << "Something is seriously fucked up with window mngmt ..." << Log::endl;
            std::abort();
        }
    }

    setTitle(title);
    setMessage(message);
    quitSimOnConfirm(quit);
    display();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

MessageWindow::~MessageWindow()
{
    XPSetWidgetProperty(m_box_widget, xpProperty_Object, 0);
    XPDestroyWidget(m_box_widget,1);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MessageWindow::setWidth(int width)
{
    if (width > m_xp_width)
        throw (BoxOutOfBoundsException("Requested window width greater than X-Plane window"));
    m_left = (m_xp_width - width)/2;
    m_right = (m_xp_width + width)/2;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MessageWindow::setHeight(int height)
{
    if (height > m_xp_height)
        throw (BoxOutOfBoundsException("Requested window height greater than X-Plane window"));
    m_top = (m_xp_height + height)/2;
    m_bottom = (m_xp_height - height)/2;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MessageWindow::setTitle(const std::string &title)
{
    m_title = title;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MessageWindow::setMessage(const std::string &message)
{
    m_message = message;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MessageWindow::quitSimOnConfirm(bool quit)
{
    m_quit_on_confirm = quit;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MessageWindow::display()
{
    createSurroundingBox();
    createInnerScreen();
    // break up string in portions
    //!< @todo Measure actual string length with 2.0 function and decide according to window width
    const unsigned int max_chars = 70;
    std::list<std::string> list_of_words;
    splitStr(list_of_words, m_message, " ", false);

    std::list<std::string> list_of_sequences;
    std::string seq;
    while (!list_of_words.empty())
    {
        if (seq.size() + list_of_words.front().size() < max_chars)
        {
            // Adding the current word to the sequence doesn't exceed max length. Append it and remove it from words
            seq.append(list_of_words.front()).append(" ");
            list_of_words.pop_front();
        } else {
            // The string would grow too large. save its actual contents and clear the temporary string
            list_of_sequences.push_back(seq);
            seq.clear();
        }
    }
    if (!seq.empty())
        list_of_sequences.push_back(seq);
    // for each sequence of maximum length, create a caption inside the screen widget
    m_caption_widgets_list.reserve(list_of_sequences.size());
    for (int i=0 ; !list_of_sequences.empty() ; i++)
    {
        m_caption_widgets_list.push_back( XPCreateWidget(m_left + 10,
                                                         m_top - 30 - i * 12,
                                                         m_right - 10,
                                                         m_top - 40 - i * 12,
                                                         1,
                                                         list_of_sequences.front().c_str(),
                                                         0,
                                                         m_box_widget,
                                                         xpWidgetClass_Caption)
                                          );
        list_of_sequences.pop_front();
    }

    XPSetWidgetProperty(m_screen_widget, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);
    XPSetWidgetProperty(m_box_widget, xpProperty_Object, reinterpret_cast<intptr_t>(this));
    XPAddWidgetCallback(m_box_widget, widgetCallback);
    m_is_displayed = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool MessageWindow::isDisplayed()
{
    return m_is_displayed;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int MessageWindow::processMessages(XPWidgetMessage inMessage, intptr_t, intptr_t)
{
    if (inMessage == xpMessage_CloseButtonPushed)
    {
        if (m_quit_on_confirm)
        {
            //Log::finish();
            std::exit(1);
        } else
        {
            XPDestroyWidget(m_box_widget, 1);
            m_is_displayed = false;
        }
        return 1;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int MessageWindow::widgetCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t param1, intptr_t param2)
{
    MessageWindow* widget = reinterpret_cast<MessageWindow *>(XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr));
    if (widget)
    {
        return widget->processMessages(inMessage, param1, param2);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int MessageWindow::splitStr(std::list<std::string>& L, const std::string& seq, const std::string& _1cdelim, bool _removews )
{
    typedef std::string::size_type ST;
    std::string delims = _1cdelim;
    std::string STR;
    if(delims.empty()) delims = "\n\r";
    if(_removews) delims += " ";

    ST pos=0, LEN = seq.size();
    while(pos < LEN )
    {
        STR=""; // Init/clear the STR token buffer

        // remove any delimiters including optional (white)spaces
        while( (delims.find(seq[pos]) != std::string::npos) && (pos < LEN) ) ++pos;

        // leave if @eos
        if(pos==LEN) return L.size();
        // Save token data
        while( (delims.find(seq[pos]) == std::string::npos) && (pos < LEN) ) STR += seq[pos++];
        // put valid STR buffer into the supplied list
        if( ! STR.empty() ) L.push_back(STR);
    }
    return L.size();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MessageWindow::createSurroundingBox()
{
    m_box_widget = XPCreateWidget(m_left,
                                  m_top,
                                  m_right,
                                  m_bottom,
                                  1,
                                  m_title.c_str(),
                                  1,
                                  nullptr,
                                  xpWidgetClass_MainWindow);
    XPSetWidgetProperty(m_box_widget, xpProperty_MainWindowHasCloseBoxes, 1);
    XPSetWidgetProperty(m_box_widget, xpProperty_MainWindowType, xpMainWindowStyle_MainWindow);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MessageWindow::createInnerScreen()
{
    m_screen_widget = XPCreateWidget(m_left + 5,
                                     m_top - 25,
                                     m_right - 5,
                                     m_bottom + 5,
                                     1,
                                     "",
                                     0,
                                     m_box_widget,
                                     xpWidgetClass_SubWindow);
}
