#include "opengl_helper.h"
#include "iArray.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


/// A factory call.
IOpenGLHelper * g_fac_SPAWN_OPENGL_HELPER (void) {
    return new OPENGLHELPER::OpenGLHelper ();
}


static const char * g_err_NOOPENGL = "OpenGL seems to be unavailable on the host system.";
static const char * g_err_GLEWINIT = "Failed to init GLEW.";
static const char * g_err_EXT      = "%s extension is not supported. Failed to proceed.";
static const char * g_err_EXTOPT   = "Optional extension %s is not supported.";


namespace OPENGLHELPER {

    OpenGLHelper::OpenGLHelper (void) {
        m_nShaders  = 0;
        m_LastError = NULL;
        m_Logger    = NULL;
        m_Id        = ISpawnedObject::allocateId ();
        m_Vendor    = GPUVENDOR_UNKNOWN;

        ISpawnedObject::_register (this);

        m_ShadersPath0[0] = 0;
        m_ShadersPath1[0] = 0;
        m_LoggingEnabled = FALSE;
        m_InitedGLEW     = FALSE;
        m_FBOs[0]        = NULL;
        m_FBOs[1]        = NULL;
        m_TEXs[0]        = NULL;
        m_TEXs[1]        = NULL;

        m_nPatchesAllocated = 0;
    }

    OpenGLHelper::~OpenGLHelper (void) {
        ISpawnedObject::_unregister (this);

        deinit ();

        logMessage (LOGIMPORTANCE_NEVERMIND, "It is a good day to die!.. (id = %d)", m_Id);
        SAFE_RELEASE (m_Logger);
    }

    void OpenGLHelper::init (void) {
        m_LastError = NULL;

        if (!m_InitedGLEW) {
            const char * gl_vendor   = (const char *) glGetString (GL_VENDOR);
            const char * gl_renderer = (const char *) glGetString (GL_RENDERER);
            const char * gl_version  = (const char *) glGetString (GL_VERSION);
            if (!gl_vendor || !gl_renderer || !gl_version) {
                logMessage (LOGIMPORTANCE_CRITICAL, g_err_NOOPENGL);
                m_LastError = g_err_NOOPENGL;
            } else {
                if (strstr (gl_vendor, "ATI")) {
                    m_Vendor = GPUVENDOR_ATI;
                    logMessage (LOGIMPORTANCE_NEVERMIND, "Recognized vendor is ATI.");
                } else
                if (strstr (gl_vendor, "NVIDIA") ||
                    strstr (gl_vendor, "NVidia")) {
                    logMessage (LOGIMPORTANCE_NEVERMIND, "Recognized vendor is NVidia.");
                    m_Vendor = GPUVENDOR_NVIDIA;
                }

                logMessage (LOGIMPORTANCE_NEVERMIND, "Initializing...");
                logMessage (LOGIMPORTANCE_NEVERMIND, "GL-version is %s.",  gl_version);
                logMessage (LOGIMPORTANCE_NEVERMIND, "GL-vendor is %s.",   gl_vendor);
                logMessage (LOGIMPORTANCE_NEVERMIND, "GL-renderer is %s.", gl_renderer);
                logMessage (LOGIMPORTANCE_NEVERMIND, "Attempting to init GLEW.");

                if (GLEW_OK == glewInit ()) {
                    logMessage (LOGIMPORTANCE_NEVERMIND, "Successfully inited GLEW.");
                    logMessage (LOGIMPORTANCE_NEVERMIND, "Checking for important extensions...");

                    static const char * const  g_REQUIRED_EXTENSTIONS[] = {
                        "GL_EXT_framebuffer_object",
                        "GL_ARB_vertex_buffer_object",
                        "GL_ARB_fragment_program",
                        "GL_ARB_vertex_program",
                        "GL_ARB_draw_buffers",
                        "GL_ARB_multitexture",
                        NULL
                    };
                    static const char * const  g_OPTIONAL_EXTENSTIONS[] = {
                        "GL_ARB_pixel_buffer_object",
                        NULL
                    };

                    for (int i = 0; i < 64 && g_REQUIRED_EXTENSTIONS[i]; i++) {
                        logMessage (LOGIMPORTANCE_NEVERMIND, "-- %s...", g_REQUIRED_EXTENSTIONS[i]);
                        if (!checkExtension (g_REQUIRED_EXTENSTIONS[i])) {
                            snprintf (m_ExtErrorBuffer, 1024, g_err_EXT, g_REQUIRED_EXTENSTIONS[i]);
                            logMessage (LOGIMPORTANCE_CRITICAL, m_ExtErrorBuffer);
                            m_LastError = m_ExtErrorBuffer;
                            return;
                        }
                    }
                    for (int i = 0; i < 64 && g_OPTIONAL_EXTENSTIONS[i]; i++) {
                        logMessage (LOGIMPORTANCE_NEVERMIND, "-- %s...", g_OPTIONAL_EXTENSTIONS[i]);
                        if (!checkExtension (g_OPTIONAL_EXTENSTIONS[i])) {
                            logMessage (LOGIMPORTANCE_WARNING, g_err_EXTOPT, g_OPTIONAL_EXTENSTIONS[i]);
                        }
                    }

                    if (GLEW_VERSION_1_2) {
                        logMessage (LOGIMPORTANCE_NEVERMIND, "GLEW_VERSION_1_2 is defined.");
                    } else {
                        logMessage (LOGIMPORTANCE_WARNING,   "GLEW_VERSION_1_2 is NOT defined.");
                    }

                    GLint max_attachments = 0;
                    glGetIntegerv (GL_MAX_COLOR_ATTACHMENTS, &max_attachments);
                    logMessage (LOGIMPORTANCE_NEVERMIND, "Maximum number of renderable color attachments is %d.",
                                max_attachments);
                    logMessage (LOGIMPORTANCE_NEVERMIND, "Creating the shared objects...");

                    applyRenderingParams ();
                    createLists ();
                    createTextures ();

                    m_FBOs[0] = new FBOWrapper (this);
                    m_FBOs[1] = new FBOWrapper (this);
                    if (m_LoggingEnabled) {
                        m_FBOs[0]->enableLogging (TRUE);
                        m_FBOs[1]->enableLogging (TRUE);
                        m_FBOs[0]->attachLogger (m_Logger);
                        m_FBOs[1]->attachLogger (m_Logger);
                    }

                    logMessage (LOGIMPORTANCE_NEVERMIND, "Everything seems clear. ^^'");
                    m_InitedGLEW = TRUE;
                } else {
                    logMessage (LOGIMPORTANCE_CRITICAL, g_err_GLEWINIT);
                    m_LastError = g_err_GLEWINIT;
                }
            }
        }
    }

    void OpenGLHelper::deinit (void) {
        if (m_InitedGLEW) {
            logMessage (LOGIMPORTANCE_NEVERMIND, "OpenGLHelper is going down!");

            m_LastError = NULL;

            unloadShaders();
            for (int i = 0; i < 5; i++) {
                glDeleteLists (m_listQuad[i], 1);
            }

            SAFE_RELEASE (m_TEXs[0]);
            SAFE_RELEASE (m_TEXs[1]);
            SAFE_DELETE  (m_FBOs[0]);
            SAFE_DELETE  (m_FBOs[1]);
            glDeleteTextures (m_nPatchesAllocated, m_TransportPatches);
            m_nPatchesAllocated = 0;

            logMessage (LOGIMPORTANCE_NEVERMIND, "Successfully deinited.");
            m_InitedGLEW = FALSE;
        }
    }

    void OpenGLHelper::createLists (void) {
        const float margin = 0.0f;//1.0f / sharedPatchSize ();

        static const float g_ListQuad_x0[5] = {
            -0.5f, -0.5f, +0.0f - margin, -0.5f, +0.0f - margin
        };
        static const float g_ListQuad_x1[5] = {
            +0.5f, +0.0f - margin, +0.5f, +0.0f - margin, +0.5f
        };
        static const float g_ListQuad_y0[5] = {
            -0.5f, -0.5f, -0.5f, +0.0f - margin, +0.0f - margin
        };
        static const float g_ListQuad_y1[5] = {
            +0.5f, +0.0f - margin, +0.0f - margin, +0.5f, +0.5f
        };

        for (int i = 0; i < 5; i++) {
            m_listQuad[i] = glGenLists (1);
            glNewList (m_listQuad[i], GL_COMPILE);
            glBegin   (GL_QUADS);
                glMultiTexCoord2f (GL_TEXTURE0, 0.0f, 0.0f);
                glMultiTexCoord2f (GL_TEXTURE1, 0.0f, 0.0f);
                glVertex3f (g_ListQuad_x0[i], g_ListQuad_y0[i], 1.0f);

                glMultiTexCoord2f (GL_TEXTURE0, 1.0f, 0.0f);
                glMultiTexCoord2f (GL_TEXTURE1, 1.0f, 0.0f);
                glVertex3f (g_ListQuad_x1[i], g_ListQuad_y0[i], 1.0f);

                glMultiTexCoord2f (GL_TEXTURE0, 1.0f, 1.0f);
                glMultiTexCoord2f (GL_TEXTURE1, 1.0f, 1.0f);
                glVertex3f (g_ListQuad_x1[i], g_ListQuad_y1[i], 1.0f);

                glMultiTexCoord2f (GL_TEXTURE0, 0.0f, 1.0f);
                glMultiTexCoord2f (GL_TEXTURE1, 0.0f, 1.0f);
                glVertex3f (g_ListQuad_x0[i], g_ListQuad_y1[i], 1.0f);
            glEnd ();
            glEndList ();
        }
    }

    void OpenGLHelper::createTextures (void) {
        // Noise textures.
        for (int i = 0; i < 2; i++) {
            m_TEXs[i] = g_fac_SPAWN_ARRAY (this, 64, 64,
                                           proposeTextureFormat (OPENGLHELPER_TEXTURE_RGBA),
                                           GL_FLOAT
            );

            if (m_TEXs[i]) {
                // Fill it with a random trash.
                float * p = (float*) m_TEXs[i]->lock_pointer ();
                if (p) {
                    const float mu = 1.0f / RAND_MAX;
                    for (int j = 64 * 64 * 4; j; j--, p++) {
                        *p = mu * rand ();
                    }

                    m_TEXs[i]->unlock ();
                }
            }
        }
    }

    void OpenGLHelper::applyRenderingParams (void) {
        glShadeModel  (GL_FLAT);
        glPolygonMode (GL_FRONT, GL_FILL);
        glDisable     (GL_DEPTH_TEST);
        glDisable     (GL_ALPHA_TEST);
        glDisable     (GL_DITHER);
        glDisable     (GL_STENCIL_TEST);
        glDisable     (GL_CULL_FACE);
        glDisable     (GL_BLEND);
        glDepthMask   (GL_FALSE);

        glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    }

    void OpenGLHelper::setShadersPath (const char * path) {
        m_LastError = NULL;
        strncpy (m_ShadersPath0, path, 255);
        strncpy (m_ShadersPath1, path, 255);

        // Termnate with slash.
        int terminator = strlen (m_ShadersPath0);
        if (m_ShadersPath0[terminator - 1] != '\\' &&
            m_ShadersPath0[terminator - 1] != '/') {
            m_ShadersPath0[terminator + 0] = '/';
            m_ShadersPath0[terminator + 1] = '\0';
            m_ShadersPath1[terminator + 0] = '/';
            m_ShadersPath1[terminator + 1] = '\0';
            terminator += 1;
        }

        // 'cd' into the versioned subfolder.
        strcat (m_ShadersPath0, "1.1/");
        strcat (m_ShadersPath1, "1.1/");
        logMessage (LOGIMPORTANCE_NEVERMIND, "Path to shaders has been changed to %s.", m_ShadersPath0);
    }

    IOpenGLShader * OpenGLHelper::loadShader (const char * name) {
        return loadShader (NULL, name);
    }

    IOpenGLShader * OpenGLHelper::loadShader (const char * vert, const char * frag) {
        m_LastError = NULL;
        if (vert && frag) {
            logMessage (LOGIMPORTANCE_NEVERMIND, "Attempting to load shader: vertex: %s, fragment: %s.",
                        vert, frag);
        } else if (frag) {
            logMessage (LOGIMPORTANCE_NEVERMIND, "Attempting to load shader: %s.", frag);
        } else {
            logMessage (LOGIMPORTANCE_CRITICAL, "No fragment program defined.");
            return NULL;
        }

        int terminator = strlen (m_ShadersPath0);

        // Generate the full path.
        if (vert)
            strncat (m_ShadersPath0, vert, 255);
            strncat (m_ShadersPath1, frag, 255);

        logMessage (LOGIMPORTANCE_NEVERMIND, "Shader's full path is %s.", m_ShadersPath1);

        // Check for existance.
        for (int i = 0; i < m_nShaders; i++) {
            if (m_Shaders[i]->operator == (m_ShadersPath1)) {
                m_ShadersPath0[terminator] = 0;
                m_ShadersPath1[terminator] = 0;
                logMessage (LOGIMPORTANCE_NEVERMIND, "Good. Returning cached value.");

                m_Shaders[i]->addRef ();
                return m_Shaders[i];
            }
        }

        // Try to create a new shader.
        if (m_nShaders < MAX_SHADERS) {
            CShader * sh = new CShader (vert ? m_ShadersPath0 : NULL, m_ShadersPath1,
                                         this, m_LoggingEnabled ? m_Logger : NULL);

            m_ShadersPath0[terminator] = 0;
            m_ShadersPath1[terminator] = 0;
            m_Shaders[m_nShaders] = sh;
            if (sh->isLoaded ()) {
                m_nShaders++;
                logMessage (LOGIMPORTANCE_NEVERMIND, "Good. Created a new shader program.");

                sh->addRef ();
                return sh;
            } else {
                delete sh;
            }
        }

        m_ShadersPath0[terminator] = 0;
        m_ShadersPath1[terminator] = 0;

        return NULL;
    }

    void OpenGLHelper::unloadShaders (void) {
        for (int i = 0; i < m_nShaders; i++) {
            if (m_Shaders[i])
                m_Shaders[i]->release ();
        }

        m_nShaders = 0;
    }

    bool OpenGLHelper::checkExtension (const char * extension) {
        if (glewIsSupported (extension)) {
            return TRUE;
        } else {
            logMessage (LOGIMPORTANCE_NEVERMIND, "Extension %s not found", extension);
            return FALSE;
        }
    }

    GLuint OpenGLHelper::sharedPatchLock (void) {
        // Search for a free patch.
        for (int i = 0; i < m_nPatchesAllocated; i++) {
            if (!m_PatchesLocked[i]) {
                 m_PatchesLocked[i] = TRUE;
                 return m_TransportPatches[i];
            }
        }

        if (m_nPatchesAllocated + 16 > MAX_TRASNPORT_PATCHES)
            return UINT_UNDEFINED;

        logMessage (LOGIMPORTANCE_NEVERMIND, "Allocating 16 additional patches.");

        // Allocate the additional patches.
        glGenTextures (16, m_TransportPatches + m_nPatchesAllocated);
        for (int i = m_nPatchesAllocated; i < m_nPatchesAllocated + 16; i++) {
            m_PatchesLocked[i] = FALSE;

            // Actually generate the texture.
            bindTexture_ex (m_TransportPatches[i], 0);
            glTexImage2D (GL_TEXTURE_2D, 0,
                  proposeTextureFormat (OPENGLHELPER_TEXTURE_RGBA),
                  sharedPatchSize (),
                  sharedPatchSize (),
                  0,
                  GL_RGBA,
                  GL_UNSIGNED_BYTE,
                  0
            );
        }

        // Lock a forst new patch.
        GLuint result = m_TransportPatches[m_nPatchesAllocated];
        m_PatchesLocked[m_nPatchesAllocated] = TRUE;

        m_nPatchesAllocated += 16;
        return result;
    }

    void OpenGLHelper::sharedPatchUnlock (GLuint name) {
        // Search for this patch name.
        for (int i = 0; i < m_nPatchesAllocated; i++) {
            if (m_TransportPatches[i] == name) {
                m_PatchesLocked[i] = FALSE;
                break;
            }
        }
    }

    GLuint OpenGLHelper::sharedPatchSwap (GLuint old_name, GLuint new_name) {
        // Search for this patch name.
        for (int i = 0; i < m_nPatchesAllocated; i++) {
            if (m_TransportPatches[i] == old_name) {
                m_TransportPatches[i]  = new_name;
                return new_name;
            }
        }

        return old_name;
    }
}
