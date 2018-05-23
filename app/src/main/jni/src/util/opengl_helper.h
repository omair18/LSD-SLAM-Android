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
#    include "GL/glew.h"
#    include "iOpenGLHelper.h"
#    include "opengl_shader.h"
#    include "fbowrapper.h"


namespace OPENGLHELPER {

#define MAX_SHADERS             128
#define MAX_TRASNPORT_PATCHES   256

    /**
     * A helper class.
     */
    class OpenGLHelper : public IOpenGLHelper,
                         public CRefCounter {

        OpenGLHelper (void);
        virtual ~OpenGLHelper (void);

        virtual void suicide (void) {
            delete this;
        }

        _VERBOSEOBJECT

    friend IOpenGLHelper * ::g_fac_SPAWN_OPENGL_HELPER (void);
    public:

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
            return "Common::OpenGLHelper";
        }

        /**
         * Init the OpenGL stuff.
         */
        virtual void init (void);

        /**
         * Deinit.
         */
        virtual void deinit (void);

        /**
         * Check the OpenGL status.
         * Returns TRUE if successfully inited.
         */
        virtual bool inited (void) const {
            return m_InitedGLEW;
        }

        /**
         * Set some rendering options.
         */
        virtual void applyRenderingParams (void);

        /**
         * Set the shaders source directory.
         */
        virtual void setShadersPath (const char * path);

        /**
         * Load the shader. A program with
         * a default vertex shader and the given
         * fragment shader is generated,
         */
        virtual IOpenGLShader * loadShader (const char * name);

        /**
         * Load the shader. A program with
         * the given vertex and fragment
         * shaders is generated,
         */
        virtual IOpenGLShader * loadShader (const char * vert, const char * frag);

        /*
         * Check for the GL extension
         */
        virtual bool checkExtension (const char * extension);

#ifdef GLEW_MX
        /**
         * Get the current OpenGL context.
         */
        virtual GLEWContext * context (void) {
            return glewGetContext ();
        }
#endif
        virtual void bindTexture_ex (UINT tex, INT idx, GLuint filter = GL_NEAREST);

        virtual const char * checkFramebufferStatus (void);
        virtual const char * checkGLErrors (void);

        /**
         * Get the vendor of a host video card.
         */
        virtual eGPU_VENDOR gpuVendor (void) const {
            return m_Vendor;
        }

        /**
         * Returns one of the pregenerated
         * shared lists by its id.
         */
        virtual GLuint sharedList (eSharedList id) const {
            if (m_InitedGLEW)
                return m_listQuad[id];
            else
                return UINT_UNDEFINED;
        }

        /**
         * Returns one of the pregenerated
         * shared framebuffers.
         */
        virtual IFBOWrapper * sharedFBO (eSharedFBO id) {
            if (m_FBOs[id])
                return dynamic_cast<IFBOWrapper*>(m_FBOs[id]);
            else
                return NULL;
        }

        /**
         * Returns one of the pregenerated
         * shared textures.
         */
        virtual IArray * sharedTexture (eSharedTexture id) {
            if (m_TEXs[id])
                return m_TEXs[id];
            else
                return NULL;
        }

        /**
         * Get the size of the shared patches
         * in pixels. All processing around here
         * shuold use this patch size.
         */
        virtual int sharedPatchSize (void) const {
            return 512;
        }

        /**
         * Allocates a temporary texture of a
         * predefined patch-size, locks it and
         * returns to caller.
         */
        virtual GLuint sharedPatchLock (void);

        /**
         * Swap the texture name of the given
         * patch with the new name.
         */
        virtual GLuint sharedPatchSwap (GLuint old_name, GLuint new_name);

        /**
         * Mark the texture as free.
         * must be called when the patch is not needed
         * anymore.
         */
        virtual void sharedPatchUnlock (GLuint name);

        /**
         * A cache for a tempopary storage of
         * the patches that are often accessed.
         */
        virtual QHash<unsigned long, GLuint> * sharedPatchCache (void) {
            return &m_PatchCache;
        }

        /**
         * Shared mutex to sync all calls.
         */
        virtual QMutex * mutex (void) {
            return &m_PriorityMutex;
        }

        /**
         * Release all loaded shaders.
         */
        virtual void unloadShaders (void);

        /**
         * Returns the pregenerated
         * PBO for faster patch transfers.
         * /
        virtual GLuint sharedPBO (void) const {
            return m_PBO;
        }
        */

        /********* IVERBOSEOBJECT IMPLANTS ************/

        /**
         * Returns the description of last error
         * or NULL if no error occured.
         */
        virtual const char * lastError (void) const {
            return m_LastError;
        }

        /**
         * Attach the logger to an object.
         */
        virtual void attachLogger (ILogger * logger) {
            SAFE_RELEASE (m_Logger);

            if (logger) {
                m_Logger = logger;
                m_Logger->addRef ();
            }
        }


        /**
         * Enable of disable logging.
         */
        virtual void enableLogging (bool enable) {
            m_LoggingEnabled = enable;
        }

        /**
         * Check if the logging is enabled.
         */
        virtual bool loggingEnabled (void) {
            return m_LoggingEnabled;
        }

    protected:
#ifdef GLEW_MX
        GLEWContext * glewGetContext (void) {
            return & m_Context;
        }

        /**
         * OpenGL context descriptor.
         */
        GLEWContext m_Context;
#endif

        void createLists    (void);
        void createTextures (void);

        /**
         * A list of loaded shaders.
         */
        CShader * m_Shaders[MAX_SHADERS];

        /**
         * Framebuffer objects.
         */
        FBOWrapper * m_FBOs[2];

        /**
         * Pregenerated shared textures.
         */
        IArray * m_TEXs[2];

        /**
         * A path to shaders directory.
         * All shaders will be loaded from
         * here by their names.
         */
        char m_ShadersPath0[256];
        char m_ShadersPath1[256];
        char m_ExtErrorBuffer[1024];

        int  m_Id;
        int  m_nShaders;
        bool m_InitedGLEW;
        bool m_PatchesLocked[MAX_TRASNPORT_PATCHES];
        int  m_nPatchesAllocated;

      //GLuint m_PBO;
        GLuint m_listQuad[5];
        GLuint m_TransportPatches[MAX_TRASNPORT_PATCHES];

        eGPU_VENDOR  m_Vendor;

        /**
         * Shared patches cache.
         */
        QHash<unsigned long, GLuint> m_PatchCache;

        QMutex m_PriorityMutex;
    };
}

//#endif
