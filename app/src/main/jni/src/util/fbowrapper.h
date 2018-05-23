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

//#define DEBUG__LOG_FBO_SWITCH



class FBOWrapper : public CRefCounter,
                   public IFBOWrapper {
    _VERBOSEOBJECT
    _GLEW_ENABLED

public:
    FBOWrapper (IOpenGLHelper * helper);
    virtual ~FBOWrapper (void);

    /********* IFBOWRAPPER IMPLANTS ***********/

    /**
     * Bind the framebuffer object.
     */
    virtual void bind (void);

    /**
     * Get the FBO name.
     */
    virtual GLuint handle (void) const {
        return m_FBO;
    }

    /**
     * Mark all attachments as free.
     */
    virtual void reset (void);

    /**
     * Attach the texture and return an
     * attachment point used.
     */
    virtual GLenum attachTexture (GLuint tex, int priority);

    /**
     * Attach the texture to the given point.
     */
    virtual bool attachTexture_explicit (GLuint tex, int point);

    /**
     * Detach the texture.
     */
    virtual void detachTexture (GLuint tex);

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

    /********* ISPAWNEDOBJECT IMPLANTS ***********/

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
        return "Common::FBOWrapper";
    }

protected:
    IOpenGLHelper * m_Helper;

    int             m_Id;
    int             m_nAttachments;
    unsigned long   m_Counter;
    unsigned long   m_AttachmentScores[64];
    unsigned long   m_AttachmentHits[64];
    int             m_AttachmentPriority[64];
    GLuint          m_Attached[64];
    GLuint          m_FBO;

    virtual void suicide (void) {
        // NOOP
        // Object's lifetime is controlled by OpenGLHelper.
    }
};
