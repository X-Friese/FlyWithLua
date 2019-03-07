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

#include <string>
#include <cstring>

#include "XPLMUtilities.h"
#include "XPLMPlanes.h"

#include "pluginpath.h"

using namespace PPL;

std::string PluginPath::plugin_directory = "";

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

std::string PluginPath::prependXPlanePath(const std::string& file)
{
    char path[512];
    XPLMGetSystemPath(path);
    std::string absolute_path(path);
    absolute_path.append(file);
    return absolute_path;
}

std::string PluginPath::prependPluginPath(const std::string& file)
{
    std::string path = "/Resources/plugins/";
    path.append(plugin_directory).append("/").append(file);
    return prependXPlanePath(path);
}

std::string PluginPath::prependPluginResourcesPath(const std::string& file)
{
    std::string res_path("Resources/");
    res_path.append(file);
    return prependPluginPath(res_path);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

std::string PluginPath::prependPlanePath(const std::string& file)
{
    char name[512];
    char path[512];
    XPLMGetNthAircraftModel(0, name, path);
    std::string absolute_path(path);
    std::size_t pos = absolute_path.find(name);
    absolute_path = absolute_path.substr(0, pos);
    absolute_path.append(file);
    return absolute_path;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void PluginPath::setPluginDirectoryName(const std::string& name)
{
    PluginPath::plugin_directory = name;
}

