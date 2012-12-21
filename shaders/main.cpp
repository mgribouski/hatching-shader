/* =====================================================================================================================
 * File - main.cpp
 * ---------------------------------------------------------------------------------------------------------------------
 */

/* =====================================================================================================================
 * ---------------------------------------------------------------------------------------------------------------------
 */

// !!!
// BOOP CHANGE THESE BACK TO GL/GL.h
#include <OpenGL/GL.h>
#include <GLUT/GLUT.h>

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <math.h>
#include "Shaders.h"

using namespace std;

ShaderProgram *shader;

const int k_dxWINDOW	= 640;
const int k_dyWINDOW	= 480;

int g_dxWindow;
int g_dyWindow;

// User Defined Structures
typedef struct tagMATRIX										// A Structure To Hold An OpenGL Matrix ( NEW )
{
	float Data[16];												// We Use [16] Due To OpenGL's Matrix Format ( NEW )
}
MATRIX;

typedef struct tagVECTOR										// A Structure To Hold A Single Vector ( NEW )
{
	float X, Y, Z;												// The Components Of The Vector ( NEW )
}
VECTOR;

typedef struct tagVNORM
{
    VECTOR Nor; // final normal
    VECTOR sumnorm; // sum total of normals
    int count; // number of normals added so far
}
VNORM;

typedef struct tagVERTEX										// A Structure To Hold A Single Vertex ( NEW )
{
	VECTOR Nor;													// Vertex Normal ( NEW )
	VECTOR Pos;                                                 // Vertex Position ( NEW )
    int index;
}
VERTEX;

typedef struct tagPOLYGON										// A Structure To Hold A Single Polygon ( NEW )
{
	VERTEX Verts[3];											// Array Of 3 VERTEX Structures ( NEW )
    VECTOR normal;
    // vertex index
}
POLYGON;

// User Defined Variables
bool		outlineDraw		= true;								// Flag To Draw The Outline ( NEW )
bool		outlineSmooth	= false;							// Flag To Anti-Alias The Lines ( NEW )
float		outlineColor[3]	= { 0.0f, 0.0f, 0.0f };				// Color Of The Lines ( NEW )
float		outlineWidth	= 5.0f;								// Width Of The Lines ( NEW )

VECTOR		lightAngle;											// The Direction Of The Light ( NEW )
bool		lightRotate		= false;							// Flag To See If We Rotate The Light ( NEW )

float		modelAngle		= 0.0f;								// Y-Axis Angle Of The Model ( NEW )
bool        modelRotate		= false;							// Flag To Rotate The Model ( NEW )

POLYGON		*polyData		= new POLYGON[3634];				// Polygon Data ( NEW )
int			polyNum			= 3634;                             // Number Of Polygons ( NEW )
int         vertNum         = 1841;

VERTEX*     vertices         = new VERTEX[1841];                  // array of vertices
VNORM       *vnorms         = new VNORM[1841];                   // total of normals for each vertex

GLuint		shaderTexture[1];									// Storage For One Texture ( NEW )

float a = 0;

float lpos[4] = {4.0,2.0,1.0,0.0};

void CrossProduct(float Ax, float Ay,float Az, float Bx, float By, float Bz, VECTOR* out) {
    out->X = Ay * Bz - By*Az;
    out->Y = Bx*Az - Ax*Bz;
    out->Z = Ax*By - Ay*Bx;
}

// Math Functions
inline float DotProduct (VECTOR &V1, VECTOR &V2)				// Calculate The Angle Between The 2 Vectors ( NEW )
{
	return V1.X * V2.X + V1.Y * V2.Y + V1.Z * V2.Z;				// Return The Angle ( NEW )
}

inline float Magnitude (VECTOR &V)								// Calculate The Length Of The Vector ( NEW )
{
	return sqrtf (V.X * V.X + V.Y * V.Y + V.Z * V.Z);			// Return The Length Of The Vector ( NEW )
}

void Normalize (VECTOR &V)										// Creates A Vector With A Unit Length Of 1 ( NEW )
{
	float M = Magnitude (V);									// Calculate The Length Of The Vector  ( NEW )
    
	if (M != 0.0f)												// Make Sure We Don't Divide By 0  ( NEW )
	{
		V.X /= M;												// Normalize The 3 Components  ( NEW )
		V.Y /= M;
		V.Z /= M;
	}
}

void RotateVector (MATRIX &M, VECTOR &V, VECTOR &D)				// Rotate A Vector Using The Supplied Matrix ( NEW )
{
	D.X = (M.Data[0] * V.X) + (M.Data[4] * V.Y) + (M.Data[8]  * V.Z);	// Rotate Around The X Axis ( NEW )
	D.Y = (M.Data[1] * V.X) + (M.Data[5] * V.Y) + (M.Data[9]  * V.Z);	// Rotate Around The Y Axis ( NEW )
	D.Z = (M.Data[2] * V.X) + (M.Data[6] * V.Y) + (M.Data[10] * V.Z);	// Rotate Around The Z Axis ( NEW )
}

void vnorms_init() {
    for (int i = 0; i < vertNum; i++) {
        vnorms[i].sumnorm.X = 0;
        vnorms[i].sumnorm.Y = 0;
        vnorms[i].sumnorm.Z = 0;
        vnorms[i].count = 0;
    }
}

void get_vertex_normals() {
    // zero vnorms
    vnorms_init();
    
    // walk over each vertex in each polygon, check against array of vertices, if match, add normal to total and increase count
    for (int i = 0; i < polyNum; i++) {
        for (int j = 0; j < vertNum; j++) {
            if ((vertices[j].Pos.X == polyData[i].Verts[0].Pos.X) && (vertices[j].Pos.Y  == polyData[i].Verts[0].Pos.Y) && (vertices[j].Pos.Z  == polyData[i].Verts[0].Pos.Z)
                || (vertices[j].Pos.X == polyData[i].Verts[1].Pos.X) && (vertices[j].Pos.Y  == polyData[i].Verts[1].Pos.Y) && (vertices[j].Pos.Z  == polyData[i].Verts[1].Pos.Z)
                || (vertices[j].Pos.X == polyData[i].Verts[2].Pos.X) && (vertices[j].Pos.Y  == polyData[i].Verts[2].Pos.Y) && (vertices[j].Pos.Z  == polyData[i].Verts[2].Pos.Z)
                ) {
                vnorms[j].sumnorm.X = vnorms[j].sumnorm.X + polyData[i].normal.X;
                vnorms[j].sumnorm.Y = vnorms[j].sumnorm.Y + polyData[i].normal.Y;
                vnorms[j].sumnorm.Z = vnorms[j].sumnorm.Z + polyData[i].normal.Z;
                vnorms[j].count++;
            }
        }
    }
    
    /*for (int j = 0; j < vertNum; j++) {
     cout << "X: ";
     cout << vnorms[j].sumnorm.X;
     cout << " Y: ";
     cout << vnorms[j].sumnorm.Y;
     cout << " Z: ";
     cout << vnorms[j].sumnorm.Z;
     cout << '\n';
     cout << "COUNTS:";
     cout << vnorms[j].count;
     cout << '\n';
     cout << '\n';
     }*/
    
    // put final vertex normal into Nor
    for (int j = 0; j < vertNum; j++) {
        vnorms[j].Nor.X = vnorms[j].sumnorm.X / (float)vnorms[j].count;
        vnorms[j].Nor.Y = vnorms[j].sumnorm.Y / (float)vnorms[j].count;
        vnorms[j].Nor.Z = vnorms[j].sumnorm.Z / (float)vnorms[j].count;
    }
    
    // copy calculated normals into polyData
    for (int i = 0; i < polyNum; i++) {
        for (int j = 0; j < 3; j++) {
            polyData[i].Verts[j].Nor.X = vnorms[polyData[i].Verts[j].index].Nor.X;
            polyData[i].Verts[j].Nor.Y = vnorms[polyData[i].Verts[j].index].Nor.Y;
            polyData[i].Verts[j].Nor.Z = vnorms[polyData[i].Verts[j].index].Nor.Z;
        }
    }
    
}

bool readFile () {
    VECTOR* dir = new VECTOR;
    string line;
    ifstream myfile ("tennis_shoe.ply");
    if (myfile.is_open())
    {
        for (int i = 0; i < 9; i++)
        {
            getline (myfile,line);
            //cout << line << endl;
        }
        int counti = 0;
        // read all vertices into an array of vertices
        while (myfile.good() && (counti < vertNum)) {
            string tmp;
            getline(myfile, tmp);
            int posX = tmp.find(" ");
            double tmpX = atof(tmp.substr(0,posX).c_str());
            vertices[counti].Pos.X = tmpX;
            int posY = tmp.find(" ", posX+1);
            
            double tmpY = atof(tmp.substr(posX+1,posY).c_str());
            vertices[counti].Pos.Y = tmpY;
            double tmpZ = atof(tmp.substr(posY+1).c_str());
            vertices[counti].Pos.Z = tmpZ;
            
            vertices[counti].index = counti;
             /*cout << tmpX;
             cout << '\t';
             cout << tmpY;
             cout << '\t';
             cout << tmpZ;
             cout << '\n';*/
             counti++;
            
        }
        // copy correct vertices into polygons while reading in polygons
        int countj = 0;
        while (myfile.good()) {
            string tmp;
            getline(myfile, tmp);
            int posThree = tmp.find(" ");
            int posV1 = tmp.find(" ", posThree+1);
            int V1 = atoi(tmp.substr(posThree+1,posV1).c_str());
            int posV2 = tmp.find(" ",posV1+1);
            int V2 = atoi(tmp.substr(posV1+1,posV2).c_str());
            int V3 = atoi(tmp.substr(posV2+1).c_str());
            
            polyData[countj].Verts[0].Pos.X = vertices[V1].Pos.X; // A
            polyData[countj].Verts[0].Pos.Y = vertices[V1].Pos.Y;
            polyData[countj].Verts[0].Pos.Z = vertices[V1].Pos.Z;
            polyData[countj].Verts[0].index = vertices[V1].index;
            
            polyData[countj].Verts[1].Pos.X = vertices[V2].Pos.X; // B
            polyData[countj].Verts[1].Pos.Y = vertices[V2].Pos.Y;
            polyData[countj].Verts[1].Pos.Z = vertices[V2].Pos.Z;
            polyData[countj].Verts[1].index = vertices[V2].index;
            
            polyData[countj].Verts[2].Pos.X = vertices[V3].Pos.X; // C
            polyData[countj].Verts[2].Pos.Y = vertices[V3].Pos.Y;
            polyData[countj].Verts[2].Pos.Z = vertices[V3].Pos.Z;
            polyData[countj].Verts[2].index = vertices[V3].index;
            countj++;
            
            // call cross prod
            CrossProduct(vertices[V2].Pos.X-vertices[V1].Pos.X, vertices[V2].Pos.Y-vertices[V1].Pos.Y, vertices[V2].Pos.Z-vertices[V1].Pos.Z, vertices[V3].Pos.X-vertices[V1].Pos.X, vertices[V3].Pos.Y-vertices[V1].Pos.Y, vertices[V3].Pos.Z-vertices[V1].Pos.Z, dir);
            Normalize(*dir);
            
            polyData[countj].normal.X = dir->X;
            polyData[countj].normal.Y = dir->Y;
            polyData[countj].normal.Z = dir->Z;
            
            /*cout << '\n';
             cout << "Z: ";
             cout << polyData[countj].normal.Z;*/
            
            /*cout << '/n';
             cout << "normalX";
             cout << polyData[countj].normal.X;*/
            
        }
        
        myfile.close();
    }
    else
    {
        printf("File not found\n");
    }
    
    get_vertex_normals();
}

void ReshapeHandler(int w, int h)
{
	//stores the width and height
	g_dxWindow = w;
	g_dyWindow = h;
	glViewport(0, 0, g_dxWindow, g_dyWindow);
    
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluPerspective(60, g_dxWindow / (float)g_dyWindow, .1, 100);
    
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void plyLoader()
{
    int i, j;													// Looping Variables ( NEW )
    
	float TmpShade;												// Temporary Shader Value ( NEW )
    
	MATRIX TmpMatrix;											// Temporary MATRIX Structure ( NEW )
	VECTOR TmpVector, TmpNormal;
    
    glBegin (GL_TRIANGLES);										// Tell OpenGL That We're Drawing Triangles
    
    for (i = 0; i < polyNum; i++)							// Loop Through Each Polygon ( NEW )
    {
        for (j = 0; j < 3; j++)								// Loop Through Each Vertex ( NEW )
        {
            TmpNormal.X = polyData[i].Verts[j].Nor.X;		// Fill Up The TmpNormal Structure With
            TmpNormal.Y = polyData[i].Verts[j].Nor.Y;		// The Current Vertices' Normal Values ( NEW )
            TmpNormal.Z = polyData[i].Verts[j].Nor.Z;
            //cout << TmpNormal.Y;
            //RotateVector (TmpMatrix, TmpNormal, TmpVector);	// Rotate This By The Matrix ( NEW )
            
            Normalize (TmpVector);							// Normalize The New Normal ( NEW )
            
            TmpShade = DotProduct (TmpVector, lightAngle);	// Calculate The Shade Value ( NEW )
            
            if (TmpShade < 0.0f)
                //cout << " NEG VAL ";
                TmpShade = 0.0f;							// Clamp The Value to 0 If Negative ( NEW )
            
            //glTexCoord1f (TmpShade);						// Set The Texture Co-ordinate As The Shade Value ( NEW )
            //glColor3f(1.0f, 0.0f, 0.0f);
            glNormal3fv(&polyData[i].Verts[j].Nor.X);
            glVertex3fv(&polyData[i].Verts[j].Pos.X);		// Send The Vertex Position ( NEW )
        }
    }
    
    glEnd ();													// Tell OpenGL To Finish Drawing
}

void DisplayHandler(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
    
	glClear(GL_COLOR_BUFFER_BIT);
    
	glViewport(0, 0, g_dxWindow, g_dyWindow);
    
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluPerspective(60, g_dxWindow / (float)g_dyWindow, .1, 100);
    
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    glTranslatef(0, 0, -8);
    
    
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glRotatef(a,0,1,0);
    glEnable (GL_CULL_FACE);
    
    glUseProgram(0);
    
    // draw outline
    if (outlineDraw)											// Check To See If We Want To Draw The Outline ( NEW )
	{
		glEnable (GL_BLEND);									// Enable Blending ( NEW )
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);		// Set The Blend Mode ( NEW )
        
		glPolygonMode (GL_BACK, GL_LINE);						// Draw Backfacing Polygons As Wireframes ( NEW )
		glLineWidth (outlineWidth);								// Set The Line Width ( NEW )
        
		glCullFace (GL_FRONT);									// Don't Draw Any Front-Facing Polygons ( NEW )
        
		glDepthFunc (GL_LEQUAL);								// Change The Depth Mode ( NEW )
        
		glColor3fv (&outlineColor[0]);							// Set The Outline Color ( NEW )
        
        plyLoader();											// Tell OpenGL We've Finished
        //glutSolidTorus(3,4,6,8);
        
		glDepthFunc (GL_LESS);									// Reset The Depth-Testing Mode ( NEW )
        
		glCullFace (GL_BACK);									// Reset The Face To Be Culled ( NEW )
        
		glPolygonMode (GL_BACK, GL_FILL);						// Reset Back-Facing Polygon Drawing Mode ( NEW )
        
		glDisable (GL_BLEND);									// Disable Blending ( NEW )
	}
    
    shader->Use();
    
    plyLoader();
    //glutSolidTorus(3,4,10,10);

	a+=0.01;

    
    // error printing!
    GLenum errCode;
    const GLubyte *errString;
    
    if ((errCode = glGetError()) != GL_NO_ERROR) {
        errString = gluErrorString(errCode);
        fprintf (stderr, "OpenGL Error: %s\n", errString);
    }

    
	glutSwapBuffers();
}

void KeyboardHandler(unsigned char key, int x, int y)
{
	switch ( key )
	{
		case 27:
		case 'q':
		case 'Q':
			exit(0);
	}
}

int main(int argc, char** argv)
{
    readFile();
    
	glutInit(&argc, argv);
    
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    
	glutInitWindowPosition(10, 50);
	glutInitWindowSize(k_dxWINDOW, k_dyWINDOW);
	glutCreateWindow("OpenGL Shader Language Demo");
    
	glutDisplayFunc(DisplayHandler);
	glutIdleFunc(DisplayHandler);
	glutReshapeFunc(ReshapeHandler);
	glutKeyboardFunc(KeyboardHandler);
    
    glDisable(GL_CULL_FACE);
	glClearColor(1.0, 1.0, 1.0, 1.0);
    
    shader = new ShaderProgram("shader.vert", "shader.frag");
    
	glutMainLoop();
    
	return 0;
}