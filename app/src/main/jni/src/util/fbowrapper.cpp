#include "fbowrapper.h"

static const char * g_err_NOT_FOUND  = "Not found.";
static const char * g_err_NOT_INITED = "Not initialized.";


FBOWrapper::FBOWrapper (IOpenGLHelper * helper) {
    m_Id = ISpawnedObject::allocateId ();
    ISpawnedObject::_register (this);

    m_Helper = helper;
    if (m_Helper) {
        m_Helper->addRef ();

        glGenFramebuffersEXT (1, &m_FBO);

        m_Counter = 0;
        glGetIntegerv (GL_MAX_COLOR_ATTACHMENTS, (GLint*) &m_nAttachments);

        m_nAttachments = MIN (64, m_nAttachments);
        reset ();
    } else {
        m_FBO = UINT_UNDEFINED;
    }

    m_Logger    = NULL;
    m_LastError = NULL;
    m_LoggingEnabled = FALSE;
}

FBOWrapper::~FBOWrapper (void) {
    ISpawnedObject::_unregister (this);

    if (m_FBO != UINT_UNDEFINED) {
        glDeleteFramebuffersEXT (1, &m_FBO);
        logMessage (LOGIMPORTANCE_NEVERMIND, "Destroyed FBO.");
    }

    // Sanity check.
    int attached = 0;
    for (int i = 0; i < m_nAttachments; i++) {
        if (m_Attached[i] != UINT_UNDEFINED) {
            attached++;
        }
    }

    if (attached) {
        logMessage (LOGIMPORTANCE_NEVERMIND, "Found %d undetached patches.", attached);
    }

    SAFE_RELEASE (m_Logger);
    SAFE_RELEASE (m_Helper);
}

void FBOWrapper::reset (void) {
    for (int i = 0; i < m_nAttachments; i++) {
        m_Attached[i]           = UINT_UNDEFINED;
        m_AttachmentPriority[i] = 0;
        m_AttachmentScores[i]   = 0L;
        m_AttachmentHits[i]     = 0L;
    }
}

void FBOWrapper::bind (void) {
    PROFILE_FUNC();

    m_LastError = NULL;
    if (m_Helper) {
        GLuint fbo = UINT_UNDEFINED;
        glGetIntegerv (GL_FRAMEBUFFER_BINDING_EXT, (GLint*) &fbo);
        if (fbo != m_FBO) {
#ifdef DEBUG__LOG_FBO_SWITCH
            logMessage (LOGIMPORTANCE_NEVERMIND, "Switched FBO.");
#endif
            glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, m_FBO);
        }
    } else {
        m_LastError = g_err_NOT_INITED;
    }
}

bool FBOWrapper::attachTexture_explicit (GLuint tex, int point) {
    if (point < m_nAttachments) {
        if (m_Attached[point] != tex) {
            glFramebufferTexture2DEXT (
                GL_FRAMEBUFFER_EXT,
                GL_COLOR_ATTACHMENT0_EXT + point,
                GL_TEXTURE_2D,
                tex, 0
            );

            m_Attached[point]           = tex;
            m_AttachmentScores[point]   = m_Counter++;
            m_AttachmentPriority[point] = 0;
            m_AttachmentHits[point]     = 0L;

#ifdef DEBUG__LOG_FBO_SWITCH
            logMessage (LOGIMPORTANCE_NEVERMIND, "Attached a new texture to point %d.", point);
#endif
        }

        return TRUE;
    }

    return FALSE;
}

GLenum FBOWrapper::attachTexture (GLuint tex, int priority) {
    PROFILE_FUNC();

    m_LastError = NULL;

#if _DEBUG
    // Sanity check.
    for (int i = 0; i < m_nAttachments; i++) {
        if (m_Attached[i] != UINT_UNDEFINED) {
            GLuint a;
            glGetFramebufferAttachmentParameterivEXT (
                GL_FRAMEBUFFER_EXT,
                GL_COLOR_ATTACHMENT0_EXT + i,
                GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                (GLint*) &a
            );

            assert (m_Attached[i] == a);
        }
    }
#endif

    int attached = -1;
    int point    = -1;

    for (int i = 0; i < m_nAttachments; i++) {
        if (m_Attached[i] == tex) {
            attached = i;
            break;
        }

        if (m_Attached[i] == UINT_UNDEFINED &&
            point < 0) {
            point = i;
        }
    }

    // If attached, return the point.
    if (attached >= 0) {
        m_AttachmentScores[attached]   = m_Counter++;
        m_AttachmentPriority[attached] = priority;
        m_AttachmentHits[attached]++;
        return GL_COLOR_ATTACHMENT0_EXT + attached;
    }

    // If no points available, we
    // have to detach something.
    for (int p = 0; point < 0 && p <= priority; p++) {
        unsigned long score = (unsigned long) -1;
        for (int i = 0; i < m_nAttachments; i++) {
            if (m_AttachmentPriority[i] <= p &&
                score > m_AttachmentScores[i]) {
                score = m_AttachmentScores[i];
                point = i;
            }
        }
    }

    if (point < 0) {
        logMessage (LOGIMPORTANCE_ERROR, "Failed to attach texture.");
        return 0;
    }

    // Bind at last.
    glFramebufferTexture2DEXT (
        GL_FRAMEBUFFER_EXT,
        GL_COLOR_ATTACHMENT0_EXT + point,
        GL_TEXTURE_2D,
        tex, 0
    );

    m_Attached[point]           = tex;
    m_AttachmentScores[point]   = m_Counter++;
    m_AttachmentPriority[point] = priority;
    m_AttachmentHits[point]     = 0L;

#ifdef DEBUG__LOG_FBO_SWITCH
    logMessage (LOGIMPORTANCE_NEVERMIND, "Attached a new texture to point %d.", point);
#endif

    return GL_COLOR_ATTACHMENT0_EXT + point;
}

void FBOWrapper::detachTexture (GLuint tex) {
    m_LastError = NULL;

    for (int i = 0; i < m_nAttachments; i++) {
        if (m_Attached[i] == tex) {
            m_Attached[i]           = UINT_UNDEFINED;
            m_AttachmentPriority[i] = 0;
            m_AttachmentScores[i]   = 0L;
            m_AttachmentHits[i]     = 0L;

#ifdef DEBUG__LOG_FBO_SWITCH
            logMessage (LOGIMPORTANCE_NEVERMIND, "Detached a texture from point %d.", i);
#endif
            return;
        }
    }

    m_LastError = g_err_NOT_FOUND;
}
