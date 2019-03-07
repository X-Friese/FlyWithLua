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

#ifndef PLUGINPATH_H
#define PLUGINPATH_H

#include <string>
#include <stdexcept>

namespace PPL {

/**
  * PluginPath provides a platform-independant absolute path to directory
  * the plugin currently runs in.
  * @author (c) 2009-2011 by Philipp Muenzel
  * @version 0.3
  */
class PluginPath
{
public:

    class PathSetupError : public std::runtime_error{
    public:
        PathSetupError(const std::string& msg):
            std::runtime_error(msg)
        {}
    };


    static std::string prependXPlanePath(const std::string&);

    /**
      * prepend the absolute path to the directory of the currently running plugin
      * to the path
      * @param path to which absolute path should be prepended
      * @return the absolute path
      * @exception PathSetupError is thrown if path conversion fails
      */
    static std::string prependPluginPath(const std::string& path);

    /**
      * prepend the absolute path to the Resources subdirectory of the
      * directory of the currently running plugin to the path
      * @param path to which absolute path should be prepended
      * @return the absolute path
      * @exception PathSetupError is thrown if path conversion fails
      */
    static std::string prependPluginResourcesPath(const std::string& path);

    /**
      * prepend the absolute path to the directory of the currently loaded
      * user airplane to the path
      * @param path to which absolute path should be prepended
      * @return the absolute path
      * @exception PathSetupError is thrown if path conversion fails
      */
    static std::string prependPlanePath(const std::string& path);

    /**
      * set the name of the directory where fat plugin resides
      * @param name
      */
    static void setPluginDirectoryName(const std::string& name);

private:

    static std::string plugin_directory;

};

}

#endif // PLUGINPATH_H
