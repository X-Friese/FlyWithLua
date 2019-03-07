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

#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <vector>
#include "stdint.h"

namespace PPL {

class Texture
{
public:

#pragma pack(push, ident, 2)

    struct BMPFILEHEADER
    {
        int16_t   bfType;
        int32_t   bfSize;
        int16_t   bfReserved1;
        int16_t   bfReserved2;
        int32_t    bfOffBits;
    };

    struct BMPINFOHEADER
    {
        int32_t    biSize;
        int32_t    biWidth;
        int32_t    biHeight;
        int16_t   biPlanes;
        int16_t   biBitCount;
        int32_t    biCompression;
        int32_t    biSizeImage;
        int32_t    biXPelsPerMeter;
        int32_t    biYPelsPerMeter;
        int32_t    biClrUsed;
        int32_t    biClrImportant;
    };

    struct IMAGEDATA
    {
        std::vector<unsigned char> pData;
        int32_t    Width;
        int32_t    Height;
        int32_t    Padding;
        int16_t   Channels;
        unsigned int bpp;
    };

#pragma pack(pop, ident)

    Texture(const std::string& file_name, bool build_mipmaps = false);
    ~Texture();
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    int id() const;
    int width() const;
    int height() const;

private:
    void swapRedBlue();
    IMAGEDATA m_imagedata;
    int m_id;
};

}

#endif // TEXTURE_H
