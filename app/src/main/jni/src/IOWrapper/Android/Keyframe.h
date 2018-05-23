/*
 * Keyframe.h
 *
 *  Created on: 17 Oct 2014
 *      Author: thomas
 */

#ifndef KEYFRAME_H_
#define KEYFRAME_H_

#ifdef NATIVE_DRAW
#include <pangolin/gl/glplatform.h> // aarontang added
#include <pangolin/gl/gldraw.h>
#endif
#include "util/settings.h"
#include "sophus/sim3.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include "util/logger.h"

struct InputPointDense
{
    float idepth;
    float idepth_var;
    unsigned char color[4];
};

struct GraphFramePose
{
    int id;
    float camToWorld[7];
};

class Keyframe
{
    public:
        Keyframe()
         : pointData(0),
           hasVbo(false),
           points(0),
           needsUpdate(false),
           vertices(NULL)
        {}

        virtual ~Keyframe()
        {
            if(pointData)
                delete [] pointData;

            if(hasVbo)
                glDeleteBuffers(1, &vbo);

            if (vertices)
                delete vertices;
        }

        struct MyVertex
        {
            float point[3];
            unsigned char color[4];
        };

        void updatePoints(Keyframe * newFrame)
        {
            if(pointData == 0)
            {
                pointData = new unsigned char[width * height * sizeof(InputPointDense)];
            }

            LOGD("updatePoints: initId=%d, id=%d\n", initId, id);
            memcpy(pointData, newFrame->pointData, width * height * sizeof(InputPointDense));

            needsUpdate = true;
        }

        void computeVbo()
        {
#ifdef NATIVE_DRAW
            assert(!(hasVbo && !needsUpdate));

            if(hasVbo && needsUpdate)
            {
                glDeleteBuffers(1, &vbo);
                points = 0;
            }

            MyVertex * tmpBuffer = new MyVertex[width * height];

            float my_scaledTH = 1e-3;
            float my_absTH = 1e-1;
            float my_scale = camToWorld.scale();
            int my_minNearSupport = 9;
            int my_sparsifyFactor = 1;

            InputPointDense * originalInput = (InputPointDense *)pointData;

            float fxi = 1/fx;
            float fyi = 1/fy;
            float cxi = -cx / fx;
            float cyi = -cy / fy;

            for(int y = 1; y < height - 1; y++)
            {
                for(int x = 1; x < width - 1; x++)
                {
                    if(originalInput[x + y * width].idepth <= 0)
                        continue;

                    if(my_sparsifyFactor > 1 && rand() % my_sparsifyFactor != 0)
                        continue;

                    float depth = 1 / originalInput[x + y * width].idepth;

                    float depth4 = depth * depth;

                    depth4 *= depth4;

                    if(originalInput[x + y * width].idepth_var * depth4 > my_scaledTH)
                        continue;

                    if(originalInput[x + y * width].idepth_var * depth4 * my_scale * my_scale > my_absTH)
                        continue;

                    if(my_minNearSupport > 1)
                    {
                        int nearSupport = 0;
                        for(int dx = -1; dx < 2; dx++)
                        {
                            for(int dy = -1; dy < 2; dy++)
                            {
                                int idx = x + dx + (y + dy) * width;
                                if(originalInput[idx].idepth > 0)
                                {
                                    float diff = originalInput[idx].idepth - 1.0f / depth;
                                    if(diff * diff < 2 * originalInput[x + y * width].idepth_var)
                                        nearSupport++;
                                }
                            }
                        }

                        if(nearSupport < my_minNearSupport)
                            continue;
                    }

                    tmpBuffer[points].point[0] = (x * fxi + cxi) * depth;
                    tmpBuffer[points].point[1] = (y * fyi + cyi) * depth;
                    tmpBuffer[points].point[2] = depth;
                    tmpBuffer[points].color[3] = 100;
                    tmpBuffer[points].color[2] = originalInput[x + y * width].color[0];
                    tmpBuffer[points].color[1] = originalInput[x + y * width].color[1];
                    tmpBuffer[points].color[0] = originalInput[x + y * width].color[2];
                    points++;
                }
            }

            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(MyVertex) * points, tmpBuffer, GL_STATIC_DRAW);

            delete [] tmpBuffer;

            delete [] pointData;

            pointData = 0;

            hasVbo = true;

            needsUpdate = false;
#endif
        }

        MyVertex * computeVertices(bool toWorldCoord) {
            if (vertices != NULL && !needsUpdate)
                return vertices;
        
            if (vertices != NULL) {
                delete vertices;
                vertices = NULL;
            }

            points = 0;
            LOGD("inidId=%d, id=%d\n", initId, id);
            MyVertex * tmpBuffer = new MyVertex[width * height];
            int num = computeVertices(tmpBuffer, toWorldCoord);
            points += num;

            needsUpdate = false;
            vertices = new MyVertex[points];
            memcpy(vertices, tmpBuffer, 3 * sizeof(float) * points);
            delete tmpBuffer;
            
            return vertices;
        }

        int flushPC(std::ofstream* f, int format) {
            LOGD("initId=%d, id=%d\n", initId, id);
            MyVertex* tmpBuffer = new MyVertex[width*height];
            int num = computeVertices(tmpBuffer, true);

        	for(int i=0; i<num; i++)
        	{
        	    if (format == 0) {    // binary        	    
            		f->write((const char *)tmpBuffer[i].point, 3*sizeof(float));
            		float color = tmpBuffer[i].color[0] / 255.0;
            		f->write((const char *)&color,sizeof(float));
        		} else {
        		    char buf[512] = {0};
                    sprintf(buf, "%f %f %f\n", tmpBuffer[i].point[0], tmpBuffer[i].point[1], tmpBuffer[i].point[2]);
        		    f->write(buf, strlen(buf));
        		}
        	}
        	delete tmpBuffer;

        	LOGD("Done flushing frame %d (%d points)!\n", this->id, num);
        	return num;
        }

        int computeVertices(MyVertex* vertices, bool toWorldCoord) {
            float my_scaledTH = 1e-3;
            float my_absTH = 1e-1;
            float my_scale = camToWorld.scale();
            int my_minNearSupport = 9;
            int my_sparsifyFactor = 1;

            InputPointDense * originalInput = (InputPointDense *)pointData;

            float fxi = 1/fx;
            float fyi = 1/fy;
            float cxi = -cx / fx;
            float cyi = -cy / fy;

            int num = 0;
            for(int y = 1; y < height - 1; y++)
            {
                for(int x = 1; x < width - 1; x++)
                {
                    if(originalInput[x + y * width].idepth <= 0)
                        continue;

                    if(my_sparsifyFactor > 1 && rand() % my_sparsifyFactor != 0)
                        continue;

                    float depth = 1 / originalInput[x + y * width].idepth;

                    float depth4 = depth * depth;

                    depth4 *= depth4;

                    if(originalInput[x + y * width].idepth_var * depth4 > my_scaledTH)
                        continue;

                    if(originalInput[x + y * width].idepth_var * depth4 * my_scale * my_scale > my_absTH)
                        continue;

                    if(my_minNearSupport > 1)
                    {
                        int nearSupport = 0;
                        for(int dx = -1; dx < 2; dx++)
                        {
                            for(int dy = -1; dy < 2; dy++)
                            {
                                int idx = x + dx + (y + dy) * width;
                                if(originalInput[idx].idepth > 0)
                                {
                                    float diff = originalInput[idx].idepth - 1.0f / depth;
                                    if(diff * diff < 2 * originalInput[x + y * width].idepth_var)
                                        nearSupport++;
                                }
                            }
                        }

                        if(nearSupport < my_minNearSupport)
                            continue;
                    }

                    if (toWorldCoord) {
                        Sophus::Vector3f pt = camToWorld * (Sophus::Vector3f((x*fxi + cxi), (y*fyi + cyi), 1) * depth);
                        vertices[num].point[0] = pt[0];
                        vertices[num].point[1] = pt[1];
                        vertices[num].point[2] = pt[2];
                    } else {
                        vertices[num].point[0] = (x * fxi + cxi) * depth;
                        vertices[num].point[1] = (y * fyi + cyi) * depth;
                        vertices[num].point[2] = depth;
                    }
                    vertices[num].color[3] = 100;
                    vertices[num].color[2] = originalInput[x + y * width].color[0];
                    vertices[num].color[1] = originalInput[x + y * width].color[1];
                    vertices[num].color[0] = originalInput[x + y * width].color[2];
                    num++;
                }
            }

            return num;
        }

        void drawPoints()
        {
#ifdef NATIVE_DRAW
            assert(hasVbo);

            glPushMatrix();

            Sophus::Matrix4f m = camToWorld.matrix();
            glMultMatrixf((GLfloat*)m.data());

            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glVertexPointer(3, GL_FLOAT, sizeof(MyVertex), 0);
            glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(MyVertex), (const void*) (3*sizeof(float)));

            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_COLOR_ARRAY);

            glDrawArrays(GL_POINTS, 0, points);

            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glPopMatrix();
#endif
        }

        void drawCamera()
        {
#ifdef NATIVE_DRAW
            glPushMatrix();
            Sophus::Matrix4f m = camToWorld.matrix();
            glMultMatrixf((GLfloat*) m.data());
#ifdef ANDROID
            glColor3f(1, 0, 0);
            pangolin::glDrawLine(0, 0, 0,
                                 0.05 * (0 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05);
            pangolin::glDrawLine(0, 0, 0,
                                 0.05 * (0 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05);
            pangolin::glDrawLine(0, 0, 0,
                                 0.05 * (width - 1 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05);
            pangolin::glDrawLine(0, 0, 0,
                                 0.05 * (width - 1 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05);
            pangolin::glDrawLine(0.05 * (width - 1 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05,
                                 0.05 * (width - 1 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05);
            pangolin::glDrawLine(0.05 * (width - 1 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05,
                                 0.05 * (0 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05);
            pangolin::glDrawLine(0.05 * (0 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05,
                                 0.05 * (0 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05);
            pangolin::glDrawLine(0.05 * (0 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05,
                                 0.05 * (width - 1 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05);
#else
            glColor3f(1, 0, 0);
            glBegin(GL_LINES);
                glVertex3f(0, 0, 0);
                glVertex3f(0.05 * (0 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05);
                glVertex3f(0, 0, 0);
                glVertex3f(0.05 * (0 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05);
                glVertex3f(0, 0, 0);
                glVertex3f(0.05 * (width - 1 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05);
                glVertex3f(0, 0, 0);
                glVertex3f(0.05 * (width - 1 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05);
                
                glVertex3f(0.05 * (width - 1 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05);
                glVertex3f(0.05 * (width - 1 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05);
                
                glVertex3f(0.05 * (width - 1 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05);
                glVertex3f(0.05 * (0 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05);
                
                glVertex3f(0.05 * (0 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05);
                glVertex3f(0.05 * (0 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05);
                
                glVertex3f(0.05 * (0 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05);
                glVertex3f(0.05 * (width - 1 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05);
            glEnd();
#endif
            glPopMatrix();
            glColor3f(1, 1, 1);
#endif
        }

        int id;
        int initId;
        uint64_t time;
        bool isKeyframe;

        Sophus::Sim3f camToWorld;

        float fx;
        float fy;
        float cx;
        float cy;
        int height;
        int width;

        unsigned char * pointData;

        bool hasVbo;
        GLuint vbo;
        int points;
        bool needsUpdate;
        MyVertex * vertices;
};


#endif /* KEYFRAME_H_ */
