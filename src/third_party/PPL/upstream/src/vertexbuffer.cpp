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

#include "vertexbuffer.hpp"

using namespace PPL;

VertexBuffer::VertexBuffer(Type type, std::size_t num_verts, std::size_t num_normals, std::size_t num_tex, std::size_t num_tex2, std::size_t num_colors):
    num_vrt_(num_verts),
    num_nrm_(num_normals),
    num_tx_(num_tex),
    num_tx2_(num_tex2),
    num_col_(num_colors),
    mode_(type),
    vbo_(0)
{
    stride_floats_ = num_vrt_ + num_nrm_ + num_tx_ + num_tx2_ + num_col_;
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &vbo_);
}

std::size_t VertexBuffer::beginSpecifyVerts(std::size_t num_vertices, volatile float **out_verts, volatile float **out_normals, volatile float **out_texes, volatile float **out_texes2, volatile float **out_colors)
{
    if(vbo_ == 0)
        glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    GLenum usage = GL_STATIC_DRAW;
    switch(mode_) {
    case STATIC:    usage = GL_STATIC_DRAW; break;
    case STREAMING: usage = GL_STREAM_DRAW; break;
    }
    glBufferData(GL_ARRAY_BUFFER, num_vertices * stride_floats_ * sizeof(GLfloat), nullptr, usage);
    GLfloat* bp = static_cast<GLfloat*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

    *out_verts = bp;
    bp += num_vrt_;
    if(out_normals)
    {
        if(num_nrm_)
            *out_normals = bp;
        else
            *out_normals = nullptr;
    }
    bp += num_nrm_;
    if(out_texes)
    {
        if(num_tx_)
            *out_texes = bp;
        else
            *out_texes = nullptr;
    }
    bp += num_tx_;
    if(out_texes2)
    {
        if(num_tx2_)
            *out_texes2 = bp;
        else
            *out_texes2 = nullptr;
    }
    bp += num_tx2_;
    if(out_colors)
    {
        if(num_col_)
            *out_colors = bp;
        else
            *out_colors = nullptr;
    }
    return stride_floats_;
}

void VertexBuffer::endSpecifyVerts()
{
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::setupForDraw()
{
    if (num_nrm_)
        glEnableClientState(GL_NORMAL_ARRAY);
    if (num_col_)
        glEnableClientState(GL_COLOR_ARRAY);
    if (num_tx_ || num_tx2_)
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    float* p = nullptr;

    std::size_t stride = stride_floats_*sizeof(GLfloat);

    glVertexPointer(num_vrt_, GL_FLOAT, stride, p);
    p += num_vrt_;
    if (num_nrm_)
        glNormalPointer(GL_FLOAT, stride, p);
    p += num_nrm_;
    if(num_tx_)
        glTexCoordPointer(num_tx_, GL_FLOAT, stride, p);
    p += num_tx_;
    if (num_tx2_)
    {
        glClientActiveTexture(GL_TEXTURE1);
        glTexCoordPointer(num_tx2_, GL_FLOAT, stride, p);
        glClientActiveTexture(GL_TEXTURE0);
    }
    p += num_tx2_;
    if(num_col_)
        glColorPointer(num_col_, GL_FLOAT, stride, p);
}

void VertexBuffer::draw(GLenum mode, GLint first, GLsizei count)
{
    glDrawArrays(mode,first,count);
}

void VertexBuffer::endDraw()
{
    if (num_col_)
        glDisableClientState(GL_COLOR_ARRAY);
    if (num_tx_)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    if (num_tx2_)
    {
        glClientActiveTexture(GL_TEXTURE0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    if (num_nrm_)
        glDisableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


