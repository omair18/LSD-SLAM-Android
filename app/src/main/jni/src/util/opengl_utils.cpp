#include "opengl_helper.h"

namespace OPENGLHELPER {

    void OpenGLHelper::bindTexture_ex (GLuint tex,  INT idx, GLuint filter) {
        glActiveTexture (GL_TEXTURE0 + idx);
        glBindTexture   (GL_TEXTURE_2D, tex);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP,    GL_FALSE);

        if (GLEW_VERSION_1_2) {
            // It's better to use clamping to edge here if available.
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        } else {
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        }
    }

    const char * OpenGLHelper::checkFramebufferStatus (void) {
        GLenum status;
        status=(GLenum)glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        switch(status) {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            return NULL;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            return "Framebuffer incomplete,incomplete attachment\n";
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            return "Unsupported framebuffer format\n";
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            return "Framebuffer incomplete,missing attachment\n";
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            return "Framebuffer incomplete,attached images must have same dimensions\n";
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            return "Framebuffer incomplete,attached images must have same format\n";
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            return "Framebuffer incomplete,missing draw buffer\n";
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            return "Framebuffer incomplete,missing read buffer\n";
        }

        return NULL;
    }

    const char * OpenGLHelper::checkGLErrors (void) {
        GLenum errCode;
        if ((errCode = glGetError()) != GL_NO_ERROR) {
            return (char*) gluErrorString (errCode);
        }

        return NULL;
    }
}
