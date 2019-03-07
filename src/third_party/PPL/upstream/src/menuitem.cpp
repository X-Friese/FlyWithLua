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

#include "menuitem.h"
#include "action.h"
#include <iostream>

using namespace PPL;

MenuItem::MenuItem(const std::string& title)
{
    m_item_id = XPLMAppendMenuItem(XPLMFindPluginsMenu(), title.c_str(), nullptr, 1);
    m_menu_id = XPLMCreateMenu(title.c_str(), XPLMFindPluginsMenu(), m_item_id, menuHandler, this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

MenuItem::~MenuItem()
{
    XPLMClearAllMenuItems(m_menu_id);
    XPLMEnableMenuItem(XPLMFindPluginsMenu(), m_item_id, 0);
    XPLMRemoveMenuItem(XPLMFindPluginsMenu(), m_item_id);
    XPLMDestroyMenu(m_menu_id);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MenuItem::menuHandler(void *iRef)
{
    intptr_t menu_item = reinterpret_cast<intptr_t>(iRef);
    m_actions[menu_item]->doAction();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MenuItem::addSubItem(Action* action)
{
    m_actions.push_back(action);
    XPLMAppendMenuItem(m_menu_id, action->name().c_str(), reinterpret_cast<void*>(m_actions.size()-1), 1);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MenuItem::menuHandler(void *mRef, void *iRef)
{
    MenuItem* menu = static_cast<MenuItem*>(mRef);
    menu->menuHandler(iRef);
}
