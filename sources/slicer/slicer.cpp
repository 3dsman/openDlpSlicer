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


#include <stdlib.h>    // For malloc() etc.
#include <stdio.h>     // For printf(), fopen() etc.
#include <math.h>      // For sin(), cos() etc.

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <glfw3.h>   // For GLFW, OpenGL and GLU

#include "prefs.h"

#include "tinyxml2.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <string.h>
#include "mesh.h"
#include <tinythread.h>


using namespace std;

//init some variables
int mouseX, mouseY = -1;
int wheelPos = 0;

//current slice
int curSlice = 0;

//roll pitch, zoom and view target init
float roll, pitch = 0.0;
float zoom = 200;
Coord view_target(0.0,0.0,0.0);

//curent transformation mode 0=static 1=turn 2=move 3=zoom
char mov = 0;

//pause and render flag
bool pause_flag = false;
bool render = false;

//user preferences structure
Prefs prefs;

// Window dimensions
int width, height;

GLFWwindow* interface_window;


//----------------------------------------------------------------------
// hexec() - Execute hardware-dependent executable and check for errors
//----------------------------------------------------------------------

string hexec(string cmd)
{
    fflush(NULL);
    string commandLine(prefs.GetCommand()+" ");

    //check if the file exist
    commandLine += cmd;
    FILE *stream;
    /*stream=fopen (prefs.GetCommand().c_str(),"rb");
    if (stream==NULL)
    {
        perror ("There is a problem with your com executable");
        return "";
    }
    else
        fclose (stream);*/

    stream = popen(prefs.GetCommand().c_str(), "r");
    if(stream==NULL)
    {
        perror("There is a problem with your com executable");
        return "";
    }
    string data;
    data.reserve(1024);

    while (true)
    {
        int c=fgetc(stream);
        if(c==EOF) //second time used, always first read is EOF
        {
            //perror("after eof");
            break;
        }
        data.push_back((char)c);
    }
    if(ferror(stream))
    {
        perror("Read stream:");
    }
    pclose(stream);
    if (data.empty())
    {
        data="";
    }

    return data;
}

//----------------------------------------------------------------------
// com() - Call hexec command and display result
//----------------------------------------------------------------------

int com(const char* param)
{
    string data(param);
    data = hexec(data);
    cout<<">> "<< data<<"\n";
    return 1;
}

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

//----------------------------------------------------------------------
// My_Key_Callback() - GLFW key callback
//----------------------------------------------------------------------

static void My_Key_Callback(GLFWwindow* wind, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        //exit
        if (key == GLFW_KEY_ESCAPE)
            glfwWindowShouldClose(wind);

        if (key == GLFW_KEY_SPACE)
        {
            if (!render)
            {
                if (!pause_flag) com("-start");
                render = true;
            }else
            {
                pause_flag = !pause_flag;
            }
        }
    }
}

//----------------------------------------------------------------------
// My_Mouse_Pos_Callback() - GLFW mouse position callback
//----------------------------------------------------------------------

static void My_Cursor_Pos_Callback(GLFWwindow* wind, double x, double y)
{
    switch (mov) {

        case 1 : //rotation
            {
                roll += (float)(x - mouseX)/2;
                pitch += (float)(y - mouseY)/2;
            }
        	break;

        case 2 : //translation
            {
                float horiz = (float)(x - mouseX)*zoom/300;
                float vert = (float)(y - mouseY)*zoom/300;
                float radpitch = pitch*M_PI/180;
                float radroll = roll*M_PI/180;

                view_target.X += vert*(-sin(radroll)*sin(radpitch)) + horiz*(cos(radroll));
                view_target.Y += vert*cos(radpitch);
                view_target.Z += vert*(cos(radroll)*sin(radpitch)) + horiz*(sin(radroll));
            }
        	break;

        case 3 : //zoom
            {
                float vert = (float)(y - mouseY);
                if (vert>0) {zoom *=1.0+0.01*vert;}else{zoom *=1.0/(1.0+0.01*-vert);};
            }
        	break;
    }

    mouseX = x;
    mouseY = y;
}


//----------------------------------------------------------------------
// My_Mouse_Wheel_Callback() - GLFW mouse wheel callback
//----------------------------------------------------------------------

static void My_Mouse_Wheel_Callback(GLFWwindow* wind, double xoffset, double yoffset)
{
    float offset = yoffset;//(float)(pos - wheelPos);
    if (offset>0) {zoom *=1.0+0.05*offset;}else{zoom *=1.0/(1.0+0.05*-offset);};
    //wheelPos = pos;
}


//----------------------------------------------------------------------
// My_Mouse_Button_Callback() - GLFW mouse button callback
//----------------------------------------------------------------------

static void My_Mouse_Button_Callback(GLFWwindow* wind, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
            mov = 1;
        else
            mov = 0;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
            mov = 2;
        else
            mov = 0;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        if (action == GLFW_PRESS)
            mov = 3;
        else
            mov = 0;
    }
}

static void My_Framebuffer_Size_Callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}



//----------------------------------------------------------------------
// DrawPreviz() - Draw previz scene with plane and printing space
//----------------------------------------------------------------------

void DrawPreviz(Mesh* myMesh, Coord volume )
{
    //init light pos and color then enable it
    float Light1Pos[4] = {.0f,.5f,0.5f,0.f};
    float Light2Pos[4] = {.0f,-.5f,-.5f,0.f};
    float Light1Dif[4] = {.8f,.8f,.8f,1.f};
    float Light2Dif[4] = {.3f,.3f,.4f,1.f};

    glEnable(GL_LIGHTING);
  	glEnable(GL_LIGHT0);
  	glEnable(GL_LIGHT1);

  	//enable materials
  	glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
    glEnable ( GL_COLOR_MATERIAL ) ;

    //init transformation matrix
    glMatrixMode( GL_PROJECTION );    // Select projection matrix
    glLoadIdentity();                 // Start with an identity matrix
    gluPerspective(                   // Set perspective view
        65.0,                         // Field of view = 65 degrees
        (double)width/(double)height, // Window aspect (assumes square pixels)
        1.0,                          // Near Z clipping plane
        1000.0                         // Far Z clippling plane
    );

    // Set up modelview matrix
    glMatrixMode( GL_MODELVIEW );     // Select modelview matrix
    glLoadIdentity();                 // Start with an identity matrix

    glTranslatef(0 , 0, -zoom );
    glRotatef(pitch,1.0,0.0,0.0);
    glRotatef(roll,0.0,1.0,0.0);
    glTranslatef(view_target.X , -view_target.Y, view_target.Z );

    // Set up lights
    glLightfv(GL_LIGHT0,GL_POSITION,Light1Pos);
    glLightfv(GL_LIGHT1,GL_POSITION,Light2Pos);

    glLightfv(GL_LIGHT0,GL_DIFFUSE,Light1Dif);
    glLightfv(GL_LIGHT1,GL_DIFFUSE,Light2Dif);

    //call mesh for drawing
    if (myMesh) myMesh->DrawPreviz();

    //draw plane...
    glCullFace(GL_BACK);
    glColor3f (0.4f, 0.4f, 0.8f );
    volume.X=volume.X/2;
    volume.Z=volume.Z/2;
    glBegin( GL_QUADS );
        glVertex3f(-volume.X, 0, -volume.Z);
        glVertex3f(-volume.X, 0, volume.Z);
        glVertex3f(volume.X, 0, volume.Z);
        glVertex3f(volume.X, 0, -volume.Z);
    glEnd( );
    //and printing space
    glColor3f (0.5f, 0.5f, 1.0f );
    glLineWidth(2);
    glBegin( GL_LINE_STRIP );
        glVertex3f(volume.X, 0, -volume.Z);
        glVertex3f(volume.X, 0, volume.Z);
        glVertex3f(-volume.X, 0, volume.Z);
        glVertex3f(-volume.X, 0, -volume.Z);
        glVertex3f(volume.X, 0, -volume.Z);
    glEnd( );
    glBegin( GL_LINE_STRIP );
        glVertex3f(-volume.X, volume.Y, -volume.Z);
        glVertex3f(-volume.X, volume.Y, volume.Z);
        glVertex3f(volume.X, volume.Y, volume.Z);
        glVertex3f(volume.X, volume.Y, -volume.Z);
        glVertex3f(-volume.X, volume.Y, -volume.Z);
    glEnd( );
    glBegin( GL_LINES );
        glVertex3f(-volume.X, 0, -volume.Z);
        glVertex3f(-volume.X, volume.Y, -volume.Z);
        glVertex3f(volume.X, 0, volume.Z);
        glVertex3f(volume.X, volume.Y, volume.Z);
        glVertex3f(volume.X, 0, -volume.Z);
        glVertex3f(volume.X, volume.Y, -volume.Z);
        glVertex3f(-volume.X, 0, volume.Z);
        glVertex3f(-volume.X, volume.Y, volume.Z);
    glEnd( );
    glLineWidth(1);
    glColor3f (1.0f, 1.0f, 1.0f );

}

//----------------------------------------------------------------------
// DrawRender() - Draw scene in slicing mode
//----------------------------------------------------------------------

void DrawRender( Mesh* myMesh, double yOffset, Coord volume)
{
    //glEnable(GL_MULTISAMPLE);
    // Set up projection matrix
    glDisable(GL_LIGHTING);
    glMatrixMode( GL_PROJECTION );    // Select projection matrix
    glLoadIdentity();                 // Start with an identity matrix

    //glOrtho(-width/ratio, width/ratio, -height/ratio, height/ratio, 0, 100.0);
    glOrtho(-volume.X/2, volume.X/2, -volume.Z/2, volume.Z/2, 0, 100.0);
    glRotatef(90.0,1.0,0.0,0.0);

    // Set up modelview matrix
    glMatrixMode( GL_MODELVIEW );     // Select modelview matrix
    glLoadIdentity();                 // Start with an identity matrix

    glTranslatef(0,-yOffset,0);

    //call mesh for render
    if (myMesh) myMesh->DrawRender();
}

//----------------------------------------------------------------------
// InitView() - Init windows and viewport
//----------------------------------------------------------------------

void InitView( void )
{
    // Get window size
    glfwGetWindowSize(interface_window, &width, &height );

    // Make sure that height is non-zero to avoid division by zero
    height = height < 1 ? 1 : height;

    // Set viewport
    glViewport( 0, 0, width, height );

    // Clear color and depht buffers then enable it
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
    glEnable (GL_CULL_FACE);

    //glDisable(GL_LIGHTING);
    //glDisable ( GL_COLOR_MATERIAL ) ;
	//glShadeModel(GL_FLAT);
	//glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

//----------------------------------------------------------------------
// main() - Program entry point
//----------------------------------------------------------------------

int main( int argc, char **argv )
{

    cout<< "Slicer for UV 3D printing\n";
    cout<< "Copyright 3dsman 2011-2012\n";
    cout<< "project thread here:\n";
    cout<< "http://forum.allinbox.com/aspectgeek/AutresElectronique/3dprint-logiciel-slicer-sujet_8661_1.htm\n\n";

    //load preference file
    if (prefs.Load()!=0){exit(1);};

    float exposure = prefs.GetExposure();
    float thickness = prefs.GetThickness();
    Coord volume = prefs.GetVolume();
    int steps = prefs.GetSteps();
    std::string fileName;
    //get comandline parameters (overwrite preferences if necessary)
    for(int i=1; i < argc; i++) {
        if ((strcmp(argv[i],"--help")==0)||(strcmp(argv[i],"-help")==0)||(strcmp(argv[i],"--h")==0)||(strcmp(argv[i],"-h")==0))
        {
           cout<<"exemple: slicer toto.obj\n\n";
           cout<<"-e or -exposure in second/mm\n";
           cout<<"-t or -thikness in mm\n\n";
           cout<<"-v or -volume image size in mm\n\n";
           cout<<"the scale of your obj model must be 1 unit = 1 milimeter\n";
           return 0;
        }
        if (((strcmp(argv[i],"-exposure")==0)||(strcmp(argv[i],"-e")==0))&&(argc>i+1))
        {
            exposure = atof(argv[++i]);
	        cout<< "commandline->exposure = "<< exposure <<"\n";
        }else
        if (((strcmp(argv[i],"-thikness")==0)||(strcmp(argv[i],"-t")==0))&&(argc>i+1)) {
            thickness = atof(argv[++i]);
	        cout<< "commandline->thickness = "<< thickness <<"\n";
        }else
        if (((strcmp(argv[i],"-volume")==0)||(strcmp(argv[i],"-v")==0))&&(argc>i+3)) {
            volume.X = atof(argv[++i]);
            volume.Y = atof(argv[++i]);
            volume.Z = atof(argv[++i]);
	        cout<< "commandline->volume = "<< volume.X <<" "<< volume.Y <<" "<< volume.Z <<"\n";
            }else
        fileName.assign(argv[i]);
    }
    double sliceTime = exposure;
    view_target.X = 0;
    view_target.Y = volume.Y/2;
    view_target.Z = 0;

    glfwSetErrorCallback(error_callback);
    // Initialize GLFW
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint( GLFW_SAMPLES,4);

    // Open window
    interface_window = glfwCreateWindow(800, 600,"slicer V0.1B_com",NULL,NULL);
    // If we could not open a window, exit now
    if( !interface_window )
    {
        glfwTerminate();
        return 0;
    }
    // Set window title
    //glfwSetWindowTitle( "slicer V0.08B_com" );
    InitView();
    DrawPreviz(NULL, volume);
    glfwSwapBuffers(interface_window);

    // Enable sticky keys
    //glfwEnable( GLFW_STICKY_KEYS );
    glfwSetInputMode(interface_window,GLFW_STICKY_KEYS,GL_TRUE);
    glfwSetKeyCallback(interface_window, My_Key_Callback);
    glfwSetCursorPosCallback(interface_window, My_Cursor_Pos_Callback);
    glfwSetMouseButtonCallback(interface_window,  My_Mouse_Button_Callback);
    glfwSetScrollCallback(interface_window,  My_Mouse_Wheel_Callback);
    glfwSetFramebufferSizeCallback(interface_window,  My_Framebuffer_Size_Callback);
    //glfwSetMouseWheel(0);

    glfwMakeContextCurrent(interface_window);
    //Load mesh
    Mesh myMesh;
    myMesh.Load(fileName.c_str());

    //calculate mesh bounding box
    Coord Min = myMesh.GetMin();
    Coord Max = myMesh.GetMax();

    //setup zoom init
    view_target.X=(Min.X+Max.X)/2;
    view_target.Y=(Min.Y+Max.Y)/2;
    view_target.Z=(Min.Z+Max.Z)/2;
    zoom = fmaxf(fmaxf((Max.X-Min.X),(Max.Y-Min.Y)),(Max.Z-Min.Z))*1.5;

    //calculate the number of slice needed by the mesh
    int totalSlice = int(Max.Y/thickness);

    //send info
    char infos[ 255 ];
    sprintf(infos,"-infos %i %i %i",curSlice, totalSlice, int((totalSlice - curSlice)*sliceTime));
    com(infos);

    //send steps/layer
    sprintf(infos,"-steps %i", (int)(steps*thickness));
    com(infos);

    //send init instruction
    com("-init");

    double  t = 0;
    glfwSetTime(0);
    // Main rendering loop
    do
    {
        InitView();
        if (render)
        {
            // Call our rendering function
            DrawRender(&myMesh,curSlice*thickness,volume);
            glfwSwapBuffers(interface_window);
            t = glfwGetTime();

            if ((!pause_flag)&&(t>sliceTime))
            {
                // clear image buffer and put it on screen (black image during hardware move)
                glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                glfwSwapBuffers(interface_window);
                //increase slice counter
                curSlice++;
                //if we are a the end of the print
                if (curSlice > totalSlice)
                {
                    // put end signal to hardware driver
                    com("-end");
                    render = false;
                    curSlice = 0;
                }
                else
                {
                    //send info
                    char infos[ 255 ];
                    sprintf(infos,"-infos %i %i %i",curSlice, totalSlice, int((totalSlice - curSlice)*sliceTime));
                    com(infos);
                    // put next slice signal to hardware driver
                    com("-nextSlice");
                }
                glfwSetTime(0);
            }
        }
        else
        {

            DrawPreviz(&myMesh, volume);
            // Swap front and back buffers (we use a double buffered display)
            glfwSwapBuffers(interface_window);
        }

        //process the events (keyboard, mouse,...)
        glfwPollEvents();

        // Check if the window must close
    }
    while( !glfwWindowShouldClose(interface_window ) );

    // Terminate GLFW
    glfwDestroyWindow(interface_window);
    glfwTerminate();
    exit(EXIT_SUCCESS);

    // Exit program
    return 0;
}
