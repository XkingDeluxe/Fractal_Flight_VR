#ifndef TEXTUREBUFFER_H
#define TEXTUREBUFFER_H
#include "GL/CAPI_GLE.h"
#include "Extras/OVR_Math.h"
using namespace OVR;
class TextureBuffer {
private:
    struct DepthBuffer
    {
        GLuint        texId;

        DepthBuffer(Sizei size)
        {
            glGenTextures(1, &texId);
            glBindTexture(GL_TEXTURE_2D, texId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            GLenum internalFormat = GL_DEPTH_COMPONENT24;
            GLenum type = GL_UNSIGNED_INT;
            if (GLE_ARB_depth_buffer_float)
            {
                internalFormat = GL_DEPTH_COMPONENT32F;
                type = GL_FLOAT;
            }

            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.w, size.h, 0, GL_DEPTH_COMPONENT, type, NULL);
        }
        ~DepthBuffer()
        {
            if (texId)
            {
                glDeleteTextures(1, &texId);
                texId = 0;
            }
        }
    };
public:
    GLuint              texId;
    GLuint              fboId;
    Sizei               texSize;

    TextureBuffer(Sizei size, unsigned char* data) :
        texId(0),
        fboId(0),
        texSize(0, 0)
    {
        texSize = size;

        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texSize.w, texSize.h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glGenFramebuffers(1, &fboId);
    }

    ~TextureBuffer()
    {
        if (texId)
        {
            glDeleteTextures(1, &texId);
            texId = 0;
        }
        if (fboId)
        {
            glDeleteFramebuffers(1, &fboId);
            fboId = 0;
        }
    }

    Sizei GetSize()
    {
        return texSize;
    }

    void SetAndClearRenderSurface(DepthBuffer* dbuffer)
    {
        GLuint curTexId = texId;

        glBindFramebuffer(GL_FRAMEBUFFER, fboId);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dbuffer->texId, 0);

        glViewport(0, 0, texSize.w, texSize.h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void UnsetRenderSurface()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fboId);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    }
};
#endif