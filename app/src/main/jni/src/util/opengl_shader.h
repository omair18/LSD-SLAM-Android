/***************************************************************************
 *   Copyright (C) 2009 by Anton R. <commanderkyle@gmail.com>              *
 *                                                                         *
 *   This file is a part of QAquarelle project.                            *
 *                                                                         *
 *   QAquarelle is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#pragma once
#include "commdefs.h"
#include "commtypes.h"
#include "refcounter.h"

//#ifdef QT_OPENGL_LIB
#    include "../../../glew/include/GL/glew.h"
#    include "iOpenGLHelper.h"


namespace OPENGLHELPER {

    class CShader : public IOpenGLShader,
                    public CRefCounter {

        _GLEW_ENABLED

        int       m_Id;
        char      m_FragPath[256];
        char      m_VertPath[256];
        GLuint    m_Program;
        GLuint    m_Vertex;
        GLuint    m_Fragment;

        ILogger       * m_Logger;
        IOpenGLHelper * m_Helper;

        void logMessage (eLOG_IMPORTANCE importance, const char * msg, ...) {
            if (msg && m_Logger) {
                va_list val;
                va_start (val, msg);
                m_Logger->write (importance, this, msg, val);
            }
        }

        void clear     (void);
        bool reload_13 (void);
        bool reload_20 (void);

        virtual void suicide (void) {
            delete this;
        }

    public:
         CShader (const char * vertpath, const char * fragpath,
                  IOpenGLHelper * helper, ILogger * logger);
        virtual ~CShader (void);

        const char * path (void) const {
            return m_FragPath;
        }

        bool isLoaded (void) const {
            return m_Program != UINT_UNDEFINED;
        }

        bool operator == (CShader & cmp);
        bool operator == (const char * path);

        /*********** IOPENGLSHADER IMPLANTS *************/

        /**
         * Get the OpenGL identifier
         * of program.
         */
        virtual GLuint handle (void) const {
            return m_Program;
        }

        virtual int getUniformLocation (const char * name);
        virtual void uniform1i         (int location, int v0);
        virtual void uniform1f         (int location, float v0);
        virtual void uniform2f         (int location, float v0, float v1);
        virtual void uniform3f         (int location, float v0, float v1, float v2);
        virtual void uniform4f         (int location, float v0, float v1, float v2, float v3);
        virtual void uniform4fv        (int location, int sz, const float * v);
        virtual void useProgram        (void);
        virtual void unuseProgram      (void);

        /**
         * Read the shader source once again
         * and recompile it.
         */
        virtual bool reload (void);

        /*********** ISPAWNEDOBJECT IMPLANTS *************/

        /**
         * Get the Id of current allocated object.
         */
        virtual int objectId (void) {
            return m_Id;
        }

        /**
         * Increase the object's
         * reference counter.
         */
        virtual void addRef (void) {
            CRefCounter::addRef ();
        }

        /**
         * Release object.
         */
        virtual void release (void) {
            CRefCounter::release ();
        }

        /**
         * Ask the object to emit all
         * notification signals if any.
         */
        virtual void pokeObject (void) {
        }

        /**
         * Get the classname for this object.
         */
        virtual const char * className (void) {
            return "Common::OpenGLShader";
        }
    };
}

