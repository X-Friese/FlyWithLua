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

#ifndef VERTEXBUFFER_HPP
#define VERTEXBUFFER_HPP

#include <cstddef>

#if APL == 1
#include <OpenGL/gl.h>
#elif IBM == 1
#include <windows.h>
#include "GL/glew.h"
#include <gl/gl.h>
#elif LIN == 1
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#include "XPLMGraphics.h"

namespace PPL{


/**
  * @brief A class to use Vertex Buffer Objects (VBOs) for drawing in X-Plane.
  *
  * To quote Ben Supnik "VBO beats the pants off of using our own memory."
  * Use this class to specify vertices, normals, colors and texture coordinates for drawing
  * and store them in GPU driver memory (or even in VRAM itself) and later draw them
  * in a drawing callback.
  *
  * For efficiency, VBOs should be at least 4096 bytes long - that's somewhere
  * between 70 and 341 vertices, but it depends on how much data each vertex has.
  * Don't make a VBO for 4 vertices!!  A few big VBOs is a lot better than many
  *	small ones.
  *
  *
  */
class VertexBuffer
{
public:

    /**
      * @brief Give the driver a hint how much the geometry in the VBO will change.
      *
      * Either the VBO will be set up like once to draw more or less the same
      * thing every frame.
      * Or you can specify the VBO is going to change every single frame.
      */
    enum Type {
        STATIC,     //!< VBO will not change much, save in VRAM for max draw speed
        STREAMING   //!< Save in AGP RAM so changing vertices is not so bad.
    };

    /** @param type, whether to use static or streaming buffer
      * The following params define the number of components in each vertex!
      * @param num_verts 2, 3 or 4
      * @param num_normals number of normal coordinates 0 or 3
      * @param num_tex number of texture coordinates, 0 to 4
      * @param num_tex2 number of secondary texture coordinates, 0 to 4
      * @param num_colors 0 or 4
      */
    VertexBuffer(Type type,
                 std::size_t num_verts,
                 std::size_t num_normals,
                 std::size_t num_tex,
                 std::size_t num_tex2,
                 std::size_t num_colors);

    ~VertexBuffer();

    /**
     * @brief Provide non-null pointers for the params you want to set
     * @param num_vertices
     * @param out_verts
     * @param out_normals
     * @param out_texes
     * @param out_texes2
     * @param out_colors
     * @return the stride length in bytes
     */
    std::size_t beginSpecifyVerts(std::size_t num_vertices,
                                  volatile float **out_verts,
                                  volatile float **out_normals,
                                  volatile float **out_texes,
                                  volatile float **out_texes2,
                                  volatile float **out_colors);

    /**
     * @brief call this when done setting the vertices
     */
    void endSpecifyVerts();

    /**
     * @brief call this immediately before drawing the stuff, don't call any
     * other openGL op between this and the actual drawing
     */
    void setupForDraw();

    /**
     * @brief draw This works like you are used to from glDrawArrays
     */
    void draw(GLenum mode,
              GLint first,
              GLsizei count);

    /**
     * @brief endDraw call this to reset the states after drawing
     */
    void endDraw();

private:
    std::size_t num_vrt_;
    std::size_t num_nrm_;
    std::size_t num_tx_;
    std::size_t num_tx2_;
    std::size_t num_col_;
    std::size_t stride_floats_;
    Type mode_;
    GLuint vbo_;
};

}

#endif // VERTEXBUFFER_HPP
