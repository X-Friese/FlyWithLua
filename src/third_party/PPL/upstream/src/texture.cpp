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

#include <fstream>
#include <stdexcept>
#include <cstring>
#include "texture.h"
#include "XPLMUtilities.h"
#include "XPLMGraphics.h"

#if IBM
#include <Windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#elif LIN
#include <GL/gl.h>
#include <GL/glu.h>
#elif APL
#include <OpenGL/gl.h>
#endif

using namespace PPL;

#if APL && defined(__ppc__)
int16_t Endian(int16_t Data)
{
    unsigned char *pBuffer = (unsigned char *)&Data;
    int16_t Result = (int16_t)(pBuffer[0] & 0xff) + ( (int16_t)(pBuffer[1] & 0xff) << 8) ;
    return(Result);
}

int32_t Endian(int32_t Data)
{
    unsigned char *pBuffer = (unsigned char *)&Data;

    int32_t Result =     (int32_t)(pBuffer[0] & 0xff)
            + ( (int32_t)(pBuffer[1] & 0xff) << 8)
            + ( (int32_t)(pBuffer[2] & 0xff) << 16)
            + ( (int32_t)(pBuffer[3] & 0xff) << 24);

    return(Result);
}

void SwapEndian(int16_t *Data)
{
    *Data = Endian(*Data);
}

void SwapEndian(int32_t *Data)
{
    *Data = Endian(*Data);
}
#else
/// Only the ppc mac needs these so dummy functions for x86.
void SwapEndian(int16_t *){}
void SwapEndian(int32_t *){}
#endif


Texture::Texture(const std::string& file_name, bool build_mipmaps)
{
    if (file_name.rfind(".bmp") != std::string::npos)
    {
        BMPFILEHEADER header;
        BMPINFOHEADER image_info;
        int32_t padding;

        std::ifstream fs(file_name.c_str(), std::ios_base::in | std::ios_base::binary);
        if (!fs)
            throw std::runtime_error("File could not be opened: "+file_name);
        fs.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (!fs)
            throw std::runtime_error("Bitmap header could not be read: "+file_name);
        fs.read(reinterpret_cast<char*>(&image_info), sizeof(image_info));
        if (!fs)
            throw std::runtime_error("Image info could not be read: "+file_name);
      #if APL && defined(__ppc__)
        SwapEndian(&header.bfSize);
        SwapEndian(&header.bfOffBits);

        SwapEndian(&image_info.biWidth);
        SwapEndian(&image_info.biHeight);
        SwapEndian(&image_info.biBitCount);

        if (!(((header.bfType & 0xff) == 'M') &&
              (((header.bfType >> 8) & 0xff) == 'B') &&
      #else
        if (!(((header.bfType & 0xff) == 'B') &&
              (((header.bfType >> 8) & 0xff) == 'M') &&
      #endif
              (image_info.biBitCount == 24) &&
              (image_info.biWidth > 0) &&
              (image_info.biHeight > 0)))
            throw std::runtime_error("Image is not a bitmap: "+file_name);
        if (!((header.bfSize + image_info.biSize - header.bfOffBits) >= (image_info.biWidth * image_info.biHeight * 3)))
            throw std::runtime_error("Image size mismatch: "+file_name);
        padding = (image_info.biWidth * 3 + 3) & ~3;
        padding -= image_info.biWidth * 3;

        m_imagedata.Width = image_info.biWidth;
        m_imagedata.Height = image_info.biHeight;
        m_imagedata.Padding = padding;

        /// Allocate memory for the actual image.
        m_imagedata.Channels = 3;
        std::size_t bytes = image_info.biWidth * image_info.biHeight * m_imagedata.Channels + image_info.biHeight * padding;
        try {
            m_imagedata.pData.resize(bytes);

            fs.read(reinterpret_cast<char*>(&m_imagedata.pData[0]), bytes);


        } catch (std::bad_alloc&)
        {
            throw std::runtime_error("Could not allocate memory for bitmap data");
        }

        swapRedBlue();

        /// Do the opengl stuff using XPLM functions for a friendly Xplane existence.
        XPLMGenerateTextureNumbers(&m_id, 1);
        XPLMBindTexture2d(m_id, 0);

        GLuint type = GL_RGB;                                        // 24bit bmp only supported for now
        glTexImage2D(GL_TEXTURE_2D, 0, type, m_imagedata.Width, m_imagedata.Height, 0, type, GL_UNSIGNED_BYTE, &m_imagedata.pData[0]);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    }
    else if (file_name.rfind(".tga") != std::string::npos)
    {
        GLubyte TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};    // Uncompressed TGA Header
        GLubyte TGAcompare[12];                             // Used To Compare TGA Header
        GLubyte header[6];                                  // First 6 Useful Bytes From The Header
        GLuint  bytesPerPixel;                              // Holds Number Of Bytes Per Pixel Used In The TGA File
        GLuint  imageSize;                                  // Used To Store The Image Size When Setting Aside Ram
        GLuint  temp;                                       // Temporary Variable
        GLuint        type=GL_RGBA;                         // Set The Default GL Mode To RBGA (32 BPP)


        FILE *file = fopen(file_name.c_str(), "rb");        // Open The TGA File

        if(    file==nullptr ||                                                        // Does File Even Exist?
                fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||  // Are There 12 Bytes To Read?
                memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0                || // Does The Header Match What We Want?
                fread(header,1,sizeof(header),file)!=sizeof(header))                // If So Read Next 6 Header Bytes
        {
            if (file == nullptr)                               // Did The File Even Exist? *Added Jim Strong*
                throw std::runtime_error("File could not be opened: "+file_name);
            else                                            // Otherwise
            {
                fclose(file);                               // If Anything Failed, Close The File
                throw std::runtime_error("Images is not a valid TGA file: "+file_name);
            }
        }

        m_imagedata.Width  = header[1] * 256 + header[0];   // Determine The TGA Width    (highbyte*256+lowbyte)
        m_imagedata.Height = header[3] * 256 + header[2];   // Determine The TGA Height    (highbyte*256+lowbyte)

        if(    m_imagedata.Width    <=0    ||               // Is The Width Less Than Or Equal To Zero
                m_imagedata.Height    <=0    ||             // Is The Height Less Than Or Equal To Zero
                (header[4]!=24 && header[4]!=32))           // Is The TGA 24 or 32 Bit?
        {
            fclose(file);                                   // If Anything Failed, Close The File
            throw std::runtime_error("File is not a vaid TGA file: "+file_name);
        }

        m_imagedata.bpp = header[4];                        // Grab The TGA's Bits Per Pixel (24 or 32)
        bytesPerPixel   = m_imagedata.bpp/8;                // Divide By 8 To Get The Bytes Per Pixel
        imageSize       = m_imagedata.Width*m_imagedata.Height*bytesPerPixel;   // Calculate The Memory Required For The TGA Data

        try {
            m_imagedata.pData.resize(imageSize);
            if (fread(m_imagedata.pData.data(), 1, imageSize, file) != imageSize)
            {
                throw std::runtime_error("Image size of tga doesn't match");
            }
        } catch (std::bad_alloc&)
        {
            throw std::runtime_error("Could not allocate memory for tga data");
        }

        for(GLuint i=0; i<(imageSize); i+=bytesPerPixel)        // Loop Through The Image Data
        {                                                       // Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
            temp = m_imagedata.pData[i];                        // Temporarily Store The Value At Image Data 'i'
            m_imagedata.pData[i] = m_imagedata.pData[i + 2];    // Set The 1st Byte To The Value Of The 3rd Byte
            m_imagedata.pData[i + 2] = temp;                    // Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
        }

        fclose (file);
        if (m_imagedata.bpp==24)                                // Was The TGA 24 Bits
        {
            type=GL_RGB;                                        // If So Set The 'type' To GL_RGB
        }
        /// Do the opengl stuff using XPLM functions for a friendly Xplane existence.
        XPLMGenerateTextureNumbers(&m_id, 1);
        XPLMBindTexture2d(m_id, 0);

        glTexImage2D(GL_TEXTURE_2D, 0, type, m_imagedata.Width, m_imagedata.Height, 0, type, GL_UNSIGNED_BYTE, &m_imagedata.pData[0]);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    }
    else
    {
        throw std::runtime_error("The texture file is neither a BMP nor a TGA. Other fileformats are not supported.");
    }
    if (build_mipmaps)
    {
#if APL
        glGenerateMipmap(GL_TEXTURE_2D);
#else
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA, m_imagedata.Width, m_imagedata.Height, GL_ALPHA, GL_UNSIGNED_BYTE, &m_imagedata.pData[0]);
#endif
    }
}

Texture::~Texture()
{
    glDeleteTextures(1, (GLuint*)&m_id);
}

int Texture::id() const
{
    return m_id;
}

int Texture::width() const
{
    return m_imagedata.Width;
}

int Texture::height() const
{
    return m_imagedata.Height;
}

void Texture::swapRedBlue()
{
    int32_t x,y;

    /// Does not support 4 channels.
    if (m_imagedata.Channels == 4)
        return;

    /// Do the swap
    std::size_t i = 0;
    for (y = 0; y < m_imagedata.Height; ++y)
    {
        for (x = 0; x < m_imagedata.Width; ++x)
        {
            std::swap(m_imagedata.pData[i], m_imagedata.pData[i+2]);
            i += 3;
            if (x == (m_imagedata.Width - 1))
                i += m_imagedata.Padding;
        }
    }
}

