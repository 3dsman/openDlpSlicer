/*Copyright Sebastien Tricoire (29/05/2011)

3dsman@free.fr

This software is a computer program whose purpose is to slice 3d models in realtime and control hardware of a dlp 3d printer.
You can find more information here: http://forum.allinbox.com/aspectgeek/AutresElectronique/3dprint-logiciel-slicer-sujet_8661_1.htm

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  use,
modify and/or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability.

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or
data to be ensured and,  more generally, to use and operate it in the
same conditions as regards security.

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/


#include "mesh.h"
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>

// Define some usefull functions
#define crossProduct(a,b,c) \
	(a)[0] = (b)[1] * (c)[2] - (c)[1] * (b)[2]; \
	(a)[1] = (b)[2] * (c)[0] - (c)[2] * (b)[0]; \
	(a)[2] = (b)[0] * (c)[1] - (c)[0] * (b)[1];

#define normalize(a) \
normalizator = sqrt((a)[0]*(a)[0]+(a)[1]*(a)[1]+(a)[2]*(a)[2]);\
        (a)[0] /= normalizator;\
        (a)[1] /= normalizator;\
        (a)[2] /= normalizator;

using namespace std;

//----------------------------------------------------------------------
// Mesh() - Constructor
//----------------------------------------------------------------------

Mesh::Mesh(): m_OBJFile(m_faces, m_vertex)
{
}

//----------------------------------------------------------------------
// ~Mesh() - Destructor
//----------------------------------------------------------------------

Mesh::~Mesh()
{
    //delete opengl drawing list
    glDeleteLists(obj_id,1);
}

//----------------------------------------------------------------------
// Load() - Load 3D model from file
//----------------------------------------------------------------------

bool Mesh::Load(const std::string& fileName){
    //load obj file and init normals
    if((!m_OBJFile.Load(fileName))||(!InitNormals()))
        return false;
        ;

    //calculate the mesh bounding box size
    if (m_vertex.size()>0){
        Min.X = m_vertex[0].X;
        Min.Y = m_vertex[0].Y;
        Min.Z = m_vertex[0].Z;
        Max.X = Min.X;
        Max.Y = Min.Y;
        Max.Z = Min.Z;
        for(unsigned int i  = 0; i < m_vertex.size(); i+=3){
            if (m_vertex[i].X<Min.X){Min.X = m_vertex[i].X;}
            if (m_vertex[i+1].Y<Min.Y){Min.Y = m_vertex[i+1].Y;}
            if (m_vertex[i+2].Z<Min.Z){Min.Z = m_vertex[i+2].Z;}
            if (m_vertex[i].X>Max.X){Max.X = m_vertex[i].X;}
            if (m_vertex[i+1].Y>Max.Y){Max.Y = m_vertex[i+1].Y;}
            if (m_vertex[i+2].Z>Max.Z){Max.Z = m_vertex[i+2].Z;}
        }
    }

    //create opengl drawing list
    obj_id = glGenLists(1);

    if(obj_id == 0)
    {
        fprintf(stderr, "erreur lors de la creation de la liste\n");
        return false;
    }

    //compile opengl drawing list
    glNewList(obj_id, GL_COMPILE);
    glBegin( GL_TRIANGLES );
    for(unsigned int i  = 0; i < m_faces.size(); i ++){
        glNormal3d(m_normals[i].X, m_normals[i].Y, m_normals[i].Z);
        glVertex3f( m_vertex[m_faces[i].Vertex[0]].X, m_vertex[m_faces[i].Vertex[0]].Y, m_vertex[m_faces[i].Vertex[0]].Z);
        glVertex3f( m_vertex[m_faces[i].Vertex[1]].X, m_vertex[m_faces[i].Vertex[1]].Y, m_vertex[m_faces[i].Vertex[1]].Z);
        glVertex3f( m_vertex[m_faces[i].Vertex[2]].X, m_vertex[m_faces[i].Vertex[2]].Y, m_vertex[m_faces[i].Vertex[2]].Z);
    }
    glEnd();
    glEndList();
    return true;
}

//----------------------------------------------------------------------
// InitNormals() - Init normals of the 3D model
//----------------------------------------------------------------------

bool Mesh::InitNormals(){
    // init mesh normal using face orientation
    for(unsigned int i  = 0; i < m_faces.size(); i ++){
        double normale[3];
        double vecteur1[3] = {m_vertex[m_faces[i].Vertex[1]].X-m_vertex[m_faces[i].Vertex[0]].X, m_vertex[m_faces[i].Vertex[1]].Y-m_vertex[m_faces[i].Vertex[0]].Y, m_vertex[m_faces[i].Vertex[1]].Z-m_vertex[m_faces[i].Vertex[0]].Z};
        double vecteur2[3] = {m_vertex[m_faces[i].Vertex[2]].X-m_vertex[m_faces[i].Vertex[0]].X, m_vertex[m_faces[i].Vertex[2]].Y-m_vertex[m_faces[i].Vertex[0]].Y, m_vertex[m_faces[i].Vertex[2]].Z-m_vertex[m_faces[i].Vertex[0]].Z};
        double normalizator;
        normalize(vecteur1);
        normalize(vecteur2);
        crossProduct(normale,vecteur1,vecteur2);
        normalize(normale);
        m_normals.push_back(Coord(normale[0],normale[1],normale[2]));
    }
    return true;
}

//----------------------------------------------------------------------
// GetMin() - Get minimum point of the mesh bounding box
//----------------------------------------------------------------------

Coord Mesh::GetMin(){
    return Min;
}

//----------------------------------------------------------------------
// GetMax() - Get maximum point of the mesh bounding box
//----------------------------------------------------------------------

Coord Mesh::GetMax(){
    return Max;
}

//----------------------------------------------------------------------
// DrawPreviz() - Draw the mesh in previz mode
//----------------------------------------------------------------------

void Mesh::DrawPreviz(){
    //draw front in white
    glCullFace(GL_BACK);
    glColor3f (1.0f, 1.0f, 1.0f );
    glCallList(obj_id);
    //and back in red
    glCullFace(GL_FRONT);
    glColor3f (1.0f, 0.0f, 0.0f );
    glCallList(obj_id);

    //draw the mesh bounding box
    glCullFace(GL_BACK);
    glColor3f (1.0f, 1.0f, 1.0f );

    glBegin( GL_LINE_STRIP );
        glVertex3f(Min.X, Min.Y, Min.Z);
        glVertex3f(Min.X, Min.Y, Max.Z);
        glVertex3f(Max.X, Min.Y, Max.Z);
        glVertex3f(Max.X, Min.Y, Min.Z);
        glVertex3f(Min.X, Min.Y, Min.Z);
    glEnd( );
    glBegin( GL_LINE_STRIP );
        glVertex3f(Min.X, Max.Y, Min.Z);
        glVertex3f(Min.X, Max.Y, Max.Z);
        glVertex3f(Max.X, Max.Y, Max.Z);
        glVertex3f(Max.X, Max.Y, Min.Z);
        glVertex3f(Min.X, Max.Y, Min.Z);
    glEnd( );
    glBegin( GL_LINES );
        glVertex3f(Min.X, Min.Y, Min.Z);
        glVertex3f(Min.X, Max.Y, Min.Z);
        glVertex3f(Max.X, Min.Y, Max.Z);
        glVertex3f(Max.X, Max.Y, Max.Z);
        glVertex3f(Max.X, Min.Y, Min.Z);
        glVertex3f(Max.X, Max.Y, Min.Z);
        glVertex3f(Min.X, Min.Y, Max.Z);
        glVertex3f(Min.X, Max.Y, Max.Z);
    glEnd( );


    //draw the bounding box rules
    glBegin( GL_LINES );
    for (int i=1;i<Max.Y-Min.Y;i++)
    {
        glVertex3f(Min.X, Min.Y+i, Min.Z);
        if (i%10)
        {
            glVertex3f(Min.X+1, Min.Y+i, Min.Z);
        }
        else
        if (i%100)
        {
            glVertex3f(Min.X+2, Min.Y+i, Min.Z);
        }else
        {
            glVertex3f(Min.X+4, Min.Y+i, Min.Z);
        }
    }

    for (int i=1;i<Max.X-Min.X;i++)
    {
        glVertex3f(Min.X+i, Min.Y, Min.Z);
        if (i%10)
        {
            glVertex3f(Min.X+i, Min.Y+1, Min.Z);
        }
        else
        if (i%100)
        {
            glVertex3f(Min.X+i, Min.Y+2, Min.Z);
        }else
        {
            glVertex3f(Min.X+i, Min.Y+i+4, Min.Z);
        }
    }

    for (int i=1;i<Max.Z-Min.Z;i++)
    {
        glVertex3f(Min.X, Min.Y, Min.Z+i);
        if (i%10)
        {
            glVertex3f(Min.X, Min.Y+1, Min.Z+i);
        }
        else
        if (i%100)
        {
            glVertex3f(Min.X, Min.Y+2, Min.Z+i);
        }else
        {
            glVertex3f(Min.X, Min.Y+i+4, Min.Z+i);
        }
    }
    glEnd( );
}

//----------------------------------------------------------------------
// DrawRender() - Draw the mesh in render mode
//----------------------------------------------------------------------

void Mesh::DrawRender(){
    glDisable(GL_DEPTH_TEST);

    //glClear (GL_STENCIL_BUFFER_BIT);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
    glColor3f (1.0f, 1.0f, 1.0f );

    //for each back face increment the stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glCullFace(GL_FRONT);
    glCallList(obj_id);

    //for each front face decrement the stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
    glCullFace(GL_BACK);
    glCallList(obj_id);

    //white fill where the stencil is positive
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilFunc(GL_LESS, 0, 0xffffffff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glColor3f (1.0f, 1.0f, 1.0f );
    glCullFace(GL_BACK);
    glBegin( GL_QUADS );
        glVertex3f(Min.X, Min.Y, Min.Z);
        glVertex3f(Min.X, Min.Y, Max.Z);
        glVertex3f(Max.X, Min.Y, Max.Z);
        glVertex3f(Max.X, Min.Y, Min.Z);
    glEnd( );

    glDisable(GL_STENCIL_TEST);
}
