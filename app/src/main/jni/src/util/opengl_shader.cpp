#include "opengl_shader.h"
#include <string.h>
#include <stdio.h>

namespace OPENGLHELPER {

    static const GLchar * g_VERTEX_SHADER_SRC_13 = {
        "centroid out vec2 g_texcoord0;"
        "centroid out vec2 g_texcoord1;"
        "centroid out vec2 g_texcoord2;"
        ""
        "void main (void) {"
        ""
        "   gl_Position = ftransform ();"
        "   g_texcoord0 = gl_MultiTexCoord0.st;"
        "   g_texcoord1 = gl_MultiTexCoord1.st;"
      //"   g_texcoord2 = gl_MultiTexCoord2.st;"
        "}"
    };

    static const GLchar * g_VERTEX_SHADER_SRC_10 = {
        "void main (void) {"
        "   gl_Position    = ftransform ();"
        "   gl_TexCoord[0] = gl_MultiTexCoord0;"
        "   gl_TexCoord[1] = gl_MultiTexCoord1;"
      //"   gl_TexCoord[2] = gl_MultiTexCoord2;"
        "}"
    };

    CShader::CShader (const char * vertpath, const char * fragpath,
                      IOpenGLHelper * helper, ILogger * logger) {
        m_Id       = ISpawnedObject::allocateId ();
        m_Program  = UINT_UNDEFINED;
        m_Vertex   = UINT_UNDEFINED;
        m_Fragment = UINT_UNDEFINED;
        m_Logger   = logger;
        m_Helper   = helper;
        if (m_Helper)
            m_Helper->addRef ();
        if (m_Logger)
            m_Logger->addRef ();

        ISpawnedObject::_register (this);

        m_FragPath[0] = 0;
        m_VertPath[0] = 0;

        if (!fragpath) {
            logMessage (LOGIMPORTANCE_CRITICAL, "No fragment program defined.");
            return;
        }

        if (fragpath) strncpy (m_FragPath, fragpath, 255);
        if (vertpath) strncpy (m_VertPath, vertpath, 255);

        reload ();
    }

    CShader::~CShader (void) {
        clear ();
        ISpawnedObject::_unregister (this);

        logMessage (LOGIMPORTANCE_NEVERMIND, "Removing shader: %s.", m_FragPath);
        SAFE_RELEASE (m_Helper);
        SAFE_RELEASE (m_Logger);
    }

    void CShader::clear (void) {
        if (m_Program != UINT_UNDEFINED) {
            if (GLEW_VERSION_2_0) {
                glDetachShader  (m_Program, m_Vertex);
                glDetachShader  (m_Program, m_Fragment);
                glDeleteShader  (m_Vertex);
                glDeleteShader  (m_Fragment);
                glDeleteProgram (m_Program);
            } else {
                glDetachObjectARB (m_Program, m_Vertex);
                glDetachObjectARB (m_Program, m_Fragment);
                glDeleteObjectARB (m_Vertex);
                glDeleteObjectARB (m_Fragment);
                glDeleteObjectARB (m_Program);
            }
        }
    }

    int CShader::getUniformLocation (const char * name) {
        if (GLEW_VERSION_2_0) {
            return glGetUniformLocation (m_Program, name);
        } else {
            return glGetUniformLocationARB (m_Program, name);
        }
    }

    void CShader::uniform1i (int location, int v0) {
        if (GLEW_VERSION_2_0) {
            glUniform1i    (location, v0);
        } else {
            glUniform1iARB (location, v0);
        }
    }

    void CShader::uniform1f (int location, float v0) {
        if (GLEW_VERSION_2_0) {
            glUniform1f    (location, v0);
        } else {
            glUniform1fARB (location, v0);
        }
    }

    void CShader::uniform2f (int location, float v0, float v1) {
        if (GLEW_VERSION_2_0) {
            glUniform2f    (location, v0, v1);
        } else {
            glUniform2fARB (location, v0, v1);
        }
    }

    void CShader::uniform3f (int location, float v0, float v1, float v2) {
        if (GLEW_VERSION_2_0) {
            glUniform3f    (location, v0, v1, v2);
        } else {
            glUniform3fARB (location, v0, v1, v2);
        }
    }

    void CShader::uniform4f (int location, float v0, float v1, float v2, float v3) {
        if (GLEW_VERSION_2_0) {
            glUniform4f    (location, v0, v1, v2, v3);
        } else {
            glUniform4fARB (location, v0, v1, v2, v3);
        }
    }
    void CShader::uniform4fv (int location, int sz, const float * v) {
        if (GLEW_VERSION_2_0) {
            glUniform4fv    (location, sz, v);
        } else {
            glUniform4fvARB (location, sz, v);
        }
    }

    void CShader::useProgram (void) {
        if (GLEW_VERSION_2_0) {
            glUseProgram (m_Program);
        } else {
            glUseProgramObjectARB (m_Program);
        }
    }

    void CShader::unuseProgram (void) {
        if (GLEW_VERSION_2_0) {
            glUseProgram (0);
        } else {
            glUseProgramObjectARB (0);
        }
    }

    bool CShader::reload (void) {
        if (!m_Helper) {
            logMessage (LOGIMPORTANCE_ERROR, "No helper object.");
            return FALSE;
        }

        if (GLEW_VERSION_2_0) {
            logMessage (LOGIMPORTANCE_NEVERMIND, "Using OpenGL 2.0...");
            return reload_20 ();
        } else
        if (GLEW_VERSION_1_3) {
            logMessage (LOGIMPORTANCE_NEVERMIND, "Using ARB extensions (OpenGL 1.3)...");
            return reload_13 ();
        }

        return FALSE;
    }

    bool CShader::reload_20 (void) {
        char g_ERROR_BUFFER[1024];

        clear ();

        bool success = FALSE;
        GLuint glslProgram    = UINT_UNDEFINED;
        GLuint fragmentShader = UINT_UNDEFINED;
        GLuint vertexShader   = UINT_UNDEFINED;

        if (!m_FragPath[0])
            return FALSE;

        // Load the file.
        FILE * f = fopen (m_FragPath, "rb");
        if (f) {
            fseek (f, 0, 2);
            size_t src_sz = ftell (f);
            fseek (f, 0, 0);

            // FIXME Unused
            int infologLength = 1024;

            GLchar * src = new GLchar[src_sz + 1];
            if (src) {
                fread (src, src_sz, 1, f);
                src[src_sz] = 0;

                // Try to load the shader.
                glslProgram    = glCreateProgram ();
                fragmentShader = glCreateShader (GL_FRAGMENT_SHADER_ARB);
                vertexShader   = glCreateShader (GL_VERTEX_SHADER_ARB);

                GLint status;
                glShaderSource  (fragmentShader, 1, (const GLchar**) &src, NULL);
                glCompileShader (fragmentShader);
                glGetShaderiv   (fragmentShader, GL_COMPILE_STATUS, &status);
                if (status == GL_TRUE) {
                    const GLchar * vert_src = g_VERTEX_SHADER_SRC_10;

                    // Load the vertex shader if defined.
                    if (m_VertPath[0]) {
                        FILE * f1 = fopen (m_VertPath, "rb");
                        if (f1) {
                            fseek (f1, 0, 2);
                            size_t src_sz = ftell (f1);
                            fseek (f1, 0, 0);

                            SAFE_DELETE_A (src);
                            src = new GLchar[src_sz + 1];
                            if (src) {
                                fread (src, src_sz, 1, f1);
                                src[src_sz] = 0;

                                vert_src = src;
                            }

                           fclose (f1);
                        } else {
                            logMessage (LOGIMPORTANCE_CRITICAL, "Shader source file not found.");
                        }
                    }

                    glShaderSource  (vertexShader, 1, &vert_src, NULL);
                    glCompileShader (vertexShader);
                    glGetShaderiv   (vertexShader, GL_COMPILE_STATUS, &status);
                    if (status == GL_TRUE) {
                        glAttachShader (glslProgram, vertexShader);
                        glAttachShader (glslProgram, fragmentShader);
                        glLinkProgram  (glslProgram);
                        glGetProgramiv (glslProgram, GL_LINK_STATUS, &status);

                        if (status == GL_TRUE) {
                            // Good...
                            logMessage (LOGIMPORTANCE_NEVERMIND, "Shader loaded.");
                            success = TRUE;
                        } else {
                            glGetProgramInfoLog (glslProgram, 1024, &infologLength, g_ERROR_BUFFER);
                            logMessage (LOGIMPORTANCE_CRITICAL, "%s: %s", m_FragPath, g_ERROR_BUFFER);
                        }
                    } else {
                        glGetShaderInfoLog (vertexShader, 1024, &infologLength, g_ERROR_BUFFER);
                        logMessage (LOGIMPORTANCE_CRITICAL, "%s: %s", m_VertPath, g_ERROR_BUFFER);
                    }
               } else {
                   glGetShaderInfoLog (fragmentShader, 1024, &infologLength, g_ERROR_BUFFER);
                   logMessage (LOGIMPORTANCE_CRITICAL, "%s: %s", m_FragPath, g_ERROR_BUFFER);
               }

               SAFE_DELETE_A (src);
           } else {
               logMessage (LOGIMPORTANCE_CRITICAL, "Shader source file not found.");
           }

           fclose (f);
       }

       if (success) {
           m_Program  = glslProgram;
           m_Vertex   = vertexShader;
           m_Fragment = fragmentShader;
       }

       return success;
    }

    bool CShader::reload_13 (void) {
        char g_ERROR_BUFFER[1024];

        clear ();

        bool success = FALSE;
        GLhandleARB glslProgram    = UINT_UNDEFINED;
        GLhandleARB fragmentShader = UINT_UNDEFINED;
        GLhandleARB vertexShader   = UINT_UNDEFINED;

        if (!m_FragPath[0])
            return FALSE;

        // Load the file.
        FILE * f = fopen (m_FragPath, "rb");
        if (f) {
            fseek (f, 0, 2);
            size_t src_sz = ftell (f);
            fseek (f, 0, 0);

            // FIXME Unused
            int infologLength = 0;

            GLchar * src = new GLchar[src_sz + 1];
            if (src) {
                fread (src, src_sz, 1, f);
                src[src_sz] = 0;

                // Try to load the shader.
                glslProgram    = glCreateProgramObjectARB ();
                fragmentShader = glCreateShaderObjectARB (GL_FRAGMENT_SHADER_ARB);
                vertexShader   = glCreateShaderObjectARB (GL_VERTEX_SHADER_ARB);

                GLint status;
                glShaderSourceARB  (fragmentShader, 1, (const GLchar**) &src, NULL);
                glCompileShaderARB (fragmentShader);
                glGetObjectParameterivARB (fragmentShader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
                if (status == GL_TRUE) {
                    const GLchar * vert_src = g_VERTEX_SHADER_SRC_10;

                    // Load the vertex shader if defined.
                    if (m_VertPath[0]) {
                        FILE * f1 = fopen (m_VertPath, "rb");
                        if (f1) {
                            fseek (f1, 0, 2);
                            size_t src_sz = ftell (f1);
                            fseek (f1, 0, 0);

                            SAFE_DELETE_A (src);
                            src = new GLchar[src_sz + 1];
                            if (src) {
                                fread (src, src_sz, 1, f1);
                                src[src_sz] = 0;

                                vert_src = src;
                            }

                           fclose (f1);
                        } else {
                            logMessage (LOGIMPORTANCE_CRITICAL, "Shader source file not found.");
                        }
                    }

                    glShaderSourceARB  (vertexShader, 1, &vert_src, NULL);
                    glCompileShaderARB (vertexShader);
                    glGetObjectParameterivARB (vertexShader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
                    if (status == GL_TRUE) {
                        glAttachObjectARB (glslProgram, vertexShader);
                        glAttachObjectARB (glslProgram, fragmentShader);
                        glLinkProgramARB  (glslProgram);
                        glGetObjectParameterivARB (glslProgram, GL_OBJECT_LINK_STATUS_ARB , &status);

                        if (status == GL_TRUE) {
                           // Good...
                           success = TRUE;
                        } else {
                            glGetInfoLogARB (glslProgram, 1024, &infologLength, g_ERROR_BUFFER);
                            logMessage (LOGIMPORTANCE_CRITICAL, "%s: %s", m_FragPath, g_ERROR_BUFFER);
                        }
                    } else {
                        glGetInfoLogARB (vertexShader, 1024, &infologLength, g_ERROR_BUFFER);
                        logMessage (LOGIMPORTANCE_CRITICAL, "%s: %s", m_VertPath, g_ERROR_BUFFER);
                    }
               } else {
                   glGetInfoLogARB (fragmentShader, 1024, &infologLength, g_ERROR_BUFFER);
                   logMessage (LOGIMPORTANCE_CRITICAL, "%s: %s", m_FragPath, g_ERROR_BUFFER);
               }

               SAFE_DELETE_A (src);
           } else {
               logMessage (LOGIMPORTANCE_CRITICAL, "Shader source file not found.");
           }

           fclose (f);
       }

       if (success) {
           m_Program  = glslProgram;
           m_Vertex   = vertexShader;
           m_Fragment = fragmentShader;
       }

       return success;
    }

    bool CShader::operator == (CShader & cmp) {
        return 0 == strcmp (m_FragPath, cmp.path ());
    }

    bool CShader::operator == (const char * path) {
        return 0 == strcmp (m_FragPath, path);
    }
}
