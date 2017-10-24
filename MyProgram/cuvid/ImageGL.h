/*
 * Copyright 1993-2017 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#ifndef NV_IMAGE_GL
#define NV_IMAGE_GL

#define _USE_MATH_DEFINES
#define M_PI       3.14159265358979323846
#include <cmath>
#include <limits>

#include "dynlink_cuda.h" // <cuda.h>
#include "dynlink_cudaGL.h" //<cudaGL.h>

#include <GL/glew.h>

#if defined(__APPLE__) || defined(__MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#define PAD_ALIGN(x,mask) ( (x + mask) & ~mask )

#define USE_TEXTURE_RECT 0


#if USE_TEXTURE_RECT
#define GL_TEXTURE_TYPE GL_TEXTURE_RECTANGLE_ARB
// gl_shader_ for displaying floating-point texture
static const char *gl_shader_code =
    "!!ARBfp1.0\n"
    "TEX result.color, fragment.texcoord, texture[0], RECT; \n"
    "END";
#else
#define GL_TEXTURE_TYPE GL_TEXTURE_2D
// gl_shader_ for displaying floating-point texture
static const char *gl_shader_code =
    "!!ARBfp1.0\n"
    "TEX result.color, fragment.texcoord, texture[0], 2D; \n"
    "END";
#endif


const int Format2Bpp[] = { 1, 4, 0 };

class ImageGL
{
    public:
        enum PixelFormat
        {
            LUMINANCE_PIXEL_FORMAT,
            BGRA_PIXEL_FORMAT,
            UNKNOWN_PIXEL_FORMAT
        };

        ImageGL(unsigned int nDispWidth, unsigned int nDispHeight,
                unsigned int nTexWidth,  unsigned int nTexHeight,
                bool bVsync,  
                PixelFormat ePixelFormat = BGRA_PIXEL_FORMAT);

        // Destructor
        ~ImageGL();

        void
        registerAsCudaResource(int field_num);

        void
        unregisterAsCudaResource(int field_num);

        void
        setTextureFilterMode(GLuint nMINfilter, GLuint nMAGfilter);

        void
        setCUDAcontext(CUcontext oContext);

        void
        setCUDAdevice(CUdevice oDevice);

        int Bpp()
        {
            return Format2Bpp[(int)e_PixFmt_];
        }

        bool
        isCudaResource()
        const;

        // Map this image's DX surface into CUDA memory space.
        // Parameters:
        //      ppImageData - point to point to image data. On return this
        //          pointer references the mapped data.
        //      pImagePitch - pointer to image pitch. On return of this
        //          pointer contains the pitch of the mapped image surface.
        //      field_num   - optional, if we are going to deinterlace and display fields separately
        // Note:
        //      This method will fail, if this image is not a registered CUDA resource.
        void
        map(CUdeviceptr *ppImageData, size_t *pImagePitch, int field_num = 0);

        void
        unmap(int field_num = 0);

        // Clear the image.
        // Parameters:
        //      nClearColor - the luminance value to clear the image to. Default is white.
        // Note:
        //      This method will not work if this image is not registered as a CUDA resource at the
        //      time of this call.
        void
        clear(unsigned char nClearColor = 0xff);

        unsigned int
        nWidth()
        const
        {
            return nWidth_;
        }

        unsigned int
        nHeight()
        const
        {
            return nHeight_;
        }

        unsigned int
        nTexWidth()
        const
        {
            return nTexWidth_;
        }

        unsigned int
        nTexHeight()
        const
        {
            return nTexHeight_;
        }


        void
        render(int field_num)
        const;

        GLuint getPBO(int field_num = 0)
        {
            return gl_pbo_[field_num];
        }
        GLuint getTexID(int field_num = 0)
        {
            return gl_texid_[field_num];
        }

		void BuildBall(float fSize, unsigned int uNumW, unsigned int uNumH)
		{
			// TODO: this is a trivial way, the top and bottom should be simplified
			unsigned int uSize = uNumW * uNumH;
			m_aVertexArray = new float[3 * uSize];
			m_aTextureArray = new float[2 * uSize];

			float fXRatio = 360.0 / (uNumW - 1);
			float fYRatio = 180.0 / (uNumH - 1);
			float fX;
			float fY;

			for (int i = 0; i<uNumH; i++)
			{
				for (int j = 0; j<uNumW; j++)
				{
					m_aTextureArray[2 * (i * uNumW + j)] = j / (uNumW - 1.0);
					m_aTextureArray[2 * (i * uNumW + j) + 1] = (i / (uNumH - 1.0));
					fX = -180.0 + j * fXRatio;
					fY = -90.0 + i * fYRatio;
					m_aVertexArray[3 * (i * uNumW + j)] = (fSize*cos(fY*M_PI / 180.0)*cos(fX*M_PI / 180.0));
					m_aVertexArray[3 * (i * uNumW + j) + 1] = (fSize*cos(fY*M_PI / 180.0)*sin(fX*M_PI / 180.0));
					m_aVertexArray[3 * (i * uNumW + j) + 2] = (fSize*sin(fY*M_PI / 180.0));
				}
			}

			m_uIndexNum = (2 * uNumW * (uNumH - 1)) + (uNumH - 1);
			m_aIndexArray = new unsigned int[m_uIndexNum];
			unsigned int uIndex = 0;
			for (int i = 0; i<uNumH - 1; i++)
			{
				for (int j = 0; j<uNumW; j++)
				{
					m_aIndexArray[uIndex] = (i*uNumW + j);
					uIndex++;
					m_aIndexArray[uIndex] = ((i + 1)*uNumW + j);
					uIndex++;
				}
				m_aIndexArray[uIndex] = 65535;
				uIndex++;
			}
		}

    private:
        GLuint gl_pbo_[3];     // OpenGL pixel buffer object
        GLuint gl_texid_[3];   // Texture resource for rendering
        GLuint gl_shader_;

        unsigned int nWidth_;
        unsigned int nHeight_;
        unsigned int nTexWidth_;
        unsigned int nTexHeight_;
        PixelFormat e_PixFmt_;
        bool bVsync_;
        bool bIsCudaResource_;

        CUcontext oContext_;
        CUdevice  oDevice_;

		float*			m_aVertexArray = nullptr;
		float*			m_aTextureArray = nullptr;
		unsigned int*	m_aIndexArray = nullptr;
		unsigned int	m_uIndexNum = 0;
		GLuint			m_uTextureID = 0;

};

#endif // IMAGE_GL

