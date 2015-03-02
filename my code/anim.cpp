// anim.cpp version 5.0 -- Template code for drawing an articulated figure.  CS 174A.
#include "../CS174a template/Utilities.h"
#include "../CS174a template/Shapes.h"
#include <cstdlib>

std::stack<mat4> mvstack;

int g_width = 800, g_height = 800 ;
float zoom = 1 ;

int animate = 0, recording = 0, basis_to_display = -1;
double TIME = 0;

const unsigned X = 0, Y = 1, Z = 2;

vec4 eye( 0, 0, 15, 1), ref( 0, 0, 0, 1 ), up( 0, 1, 0, 0 );	// The eye point and look-at point.

mat4	orientation, model_view;
ShapeData cubeData, sphereData, coneData, cylData, skyData;				// Structs that hold the Vertex Array Object index and number of vertices of each shape.
GLuint	texture_cube, texture_earth, texture_grass, texture_sky;
GLint   uModelView, uProjection, uView,
		uAmbient, uDiffuse, uSpecular, uLightPos, uShininess,
		uTex, uEnableTex;

void init()
{
#ifdef EMSCRIPTEN
    GLuint program = LoadShaders( "vshader.glsl", "fshader.glsl" );								// Load shaders and use the resulting shader program
    TgaImage coolImage ("challenge.tga");    
    TgaImage earthImage("earth.tga");
    TgaImage grassImage("../my code/grass.tga");
    TgaImage skyImage  ("../my code/sky.tga");

#else
	GLuint program = LoadShaders( "../my code/vshader.glsl", "../my code/fshader.glsl" );		// Load shaders and use the resulting shader program
    TgaImage coolImage ("../my code/challenge.tga");    
    TgaImage earthImage("../my code/earth.tga");
    TgaImage grassImage("../my code/grass.tga");
    TgaImage skyImage  ("../my code/sky.tga");
#endif
    glUseProgram(program);

	generateCube(program, &cubeData);		// Generate vertex arrays for geometric shapes
    generateSphere(program, &sphereData);
    generateCone(program, &coneData);
    generateCylinder(program, &cylData);
    generateSky(program, &skyData);

    uModelView  = glGetUniformLocation( program, "ModelView"  );
    uProjection = glGetUniformLocation( program, "Projection" );
    uView		= glGetUniformLocation( program, "View"       );
    uAmbient	= glGetUniformLocation( program, "AmbientProduct"  );
    uDiffuse	= glGetUniformLocation( program, "DiffuseProduct"  );
    uSpecular	= glGetUniformLocation( program, "SpecularProduct" );
    uLightPos	= glGetUniformLocation( program, "LightPosition"   );
    uShininess	= glGetUniformLocation( program, "Shininess"       );
    uTex		= glGetUniformLocation( program, "Tex"             );
    uEnableTex	= glGetUniformLocation( program, "EnableTex"       );

    glUniform4f( uAmbient,    0.2,  0.2,  0.2, 1 );
    glUniform4f( uDiffuse,    0.6,  0.6,  0.6, 1 );
    glUniform4f( uSpecular,   0.2,  0.2,  0.2, 1 );
    glUniform4f( uLightPos,  15.0, 15.0, 30.0, 0 );
    glUniform1f( uShininess, 100);

    glEnable(GL_DEPTH_TEST);
    
    //challenge accepted texture
    glGenTextures( 1, &texture_cube );
    glBindTexture( GL_TEXTURE_2D, texture_cube );
    
    glTexImage2D(GL_TEXTURE_2D, 0, 4, coolImage.width, coolImage.height, 0,
                 (coolImage.byteCount == 3) ? GL_BGR : GL_BGRA,
                 GL_UNSIGNED_BYTE, coolImage.data );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    

    //earth texture
    glGenTextures( 1, &texture_earth );
    glBindTexture( GL_TEXTURE_2D, texture_earth );
    
    glTexImage2D(GL_TEXTURE_2D, 0, 4, earthImage.width, earthImage.height, 0,
                 (earthImage.byteCount == 3) ? GL_BGR : GL_BGRA,
                 GL_UNSIGNED_BYTE, earthImage.data );
    
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    

    //grass texture
    glGenTextures( 1, &texture_grass );
    glBindTexture( GL_TEXTURE_2D, texture_grass );
    
    glTexImage2D(GL_TEXTURE_2D, 0, 4, grassImage.width, grassImage.height, 0,
                 (grassImage.byteCount == 3) ? GL_BGR : GL_BGRA,
                 GL_UNSIGNED_BYTE, grassImage.data );
    
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    

    // sky texture
    glGenTextures( 1, &texture_sky );
    glBindTexture( GL_TEXTURE_2D, texture_sky );
    
    glTexImage2D(GL_TEXTURE_2D, 0, 4, skyImage.width, skyImage.height, 0,
                 (skyImage.byteCount == 3) ? GL_BGR : GL_BGRA,
                 GL_UNSIGNED_BYTE, skyImage.data );
    
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );


    glUniform1i( uTex, 0);	// Set texture sampler variable to texture unit 0
	glEnable(GL_DEPTH_TEST);
}

struct color{ color( float r, float g, float b) : r(r), g(g), b(b) {} float r, g, b;};
std::stack<color> colors;
void set_color(float r, float g, float b)
{
	colors.push(color(r, g, b));

	float ambient  = 0.2, diffuse  = 0.6, specular = 0.2;
    glUniform4f(uAmbient,  ambient*r,  ambient*g,  ambient*b,  1 );
    glUniform4f(uDiffuse,  diffuse*r,  diffuse*g,  diffuse*b,  1 );
    glUniform4f(uSpecular, specular*r, specular*g, specular*b, 1 );
}

int mouseButton = -1, prevZoomCoord = 0 ;
vec2 anchor;
void myPassiveMotionCallBack(int x, int y) {	anchor = vec2( 2. * x / g_width - 1, -2. * y / g_height + 1 ); }

void myMouseCallBack(int button, int state, int x, int y)	// start or end mouse interaction
{
    mouseButton = button;
   
    if( button == GLUT_LEFT_BUTTON && state == GLUT_UP )
        mouseButton = -1 ;
    if( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
        prevZoomCoord = -2. * y / g_height + 1;

    glutPostRedisplay() ;
}

void myMotionCallBack(int x, int y)
{
	vec2 arcball_coords( 2. * x / g_width - 1, -2. * y / g_height + 1 );
	 
    if( mouseButton == GLUT_LEFT_BUTTON )
    {
	   orientation = RotateX( -10 * (arcball_coords.y - anchor.y) ) * orientation;
	   orientation = RotateY(  10 * (arcball_coords.x - anchor.x) ) * orientation;
    }
	
	if( mouseButton == GLUT_RIGHT_BUTTON )
		zoom *= 1 + .1 * (arcball_coords.y - anchor.y);
    glutPostRedisplay() ;
}

void idleCallBack(void)
{
    if( !animate ) return;
    else{
        static int counter = 0;
    	double prev_time = TIME;
        int last_whole_sec = (int) TIME;
        TIME = TM.GetElapsedTime() ;
    	if( prev_time == 0 ) TM.Reset();
        if (TIME - last_whole_sec >= 1){
            printf("FPS: %d\n", ++counter);
            counter = 0;
        }
        else counter++;
        glutPostRedisplay() ;
    }
}
//Cylinder
void drawCylinder()	//render a solid cylinder oriented along the Z axis; bases are of radius 1, placed at Z = 0, and at Z = 1.
{
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(model_view) );
    glBindVertexArray( cylData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cylData.numVertices );
}

void drawCylinder(GLuint texture)
{
    glBindTexture( GL_TEXTURE_2D, texture );
    glUniform1i( uEnableTex, 1);
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(model_view) );
    glBindVertexArray( cylData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cylData.numVertices );
    glUniform1i( uEnableTex, 0);
}

//Cone
void drawCone()	//render a solid cone oriented along the Z axis; bases are of radius 1, placed at Z = 0, and at Z = 1.
{
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(model_view) );
    glBindVertexArray( coneData.vao );
    glDrawArrays( GL_TRIANGLES, 0, coneData.numVertices );
}

void drawCone(GLuint texture)
{
    glBindTexture( GL_TEXTURE_2D, texture );
    glUniform1i( uEnableTex, 1 );
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(model_view) );
    glBindVertexArray( coneData.vao );
    glDrawArrays( GL_TRIANGLES, 0, coneData.numVertices );
    glUniform1i( uEnableTex, 0);
}

//Cube
void drawCube()
{
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( cubeData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cubeData.numVertices );
}

void drawCube(GLuint texture)
{
    glBindTexture( GL_TEXTURE_2D, texture );
    glUniform1i( uEnableTex, 1 );
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( cubeData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cubeData.numVertices );
    glUniform1i( uEnableTex, 0 );
}

//Sky
void drawSky()
{
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( skyData.vao );
    glDrawArrays( GL_TRIANGLES, 0, skyData.numVertices );
}

void drawSky(GLuint texture)
{
    glBindTexture( GL_TEXTURE_2D, texture );
    glUniform1i( uEnableTex, 1 );
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( skyData.vao );
    glDrawArrays( GL_TRIANGLES, 0, skyData.numVertices );
    glUniform1i( uEnableTex, 0 );
}

//Sphere
void drawSphere()
{
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(model_view) );
    glBindVertexArray( sphereData.vao );
    glDrawArrays( GL_TRIANGLES, 0, sphereData.numVertices );
}
void drawSphere(GLuint texture)	// draw a sphere with radius 1 centered around the origin.
{
	glBindTexture( GL_TEXTURE_2D, texture);
    glUniform1i( uEnableTex, 1);
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(model_view) );
    glBindVertexArray( sphereData.vao );
    glDrawArrays( GL_TRIANGLES, 0, sphereData.numVertices );
    glUniform1i( uEnableTex, 0 );
}


int basis_id = 0;
void drawOneAxis()
{
	mat4 origin = model_view;
	model_view *= Translate	( 0, 0, 4 );
	model_view *= Scale(.25) * Scale( 1, 1, -1 );
	drawCone();
	model_view = origin;
	model_view *= Translate	( 1,  1, .5 );
	model_view *= Scale		( .1, .1, 1 );
	drawCube();
	model_view = origin;
	model_view *= Translate	( 1, 0, .5 );
	model_view *= Scale		( .1, .1, 1 );
	drawCube();
	model_view = origin;
	model_view *= Translate	( 0,  1, .5 );
	model_view *= Scale		( .1, .1, 1 );
	drawCube();
	model_view = origin;
	model_view *= Translate	( 0,  0, 2 );
	model_view *= Scale(.1) * Scale(   1, 1, 20);
    drawCylinder();	
	model_view = origin;
}

void drawAxes(int selected)
{
	if( basis_to_display != selected ) 
		return;
	mat4 given_basis = model_view;
	model_view *= Scale		(.25);
	drawSphere();
	model_view = given_basis;
	set_color( 0, 0, 1 );
	drawOneAxis();
	model_view *= RotateX	(-90);
	model_view *= Scale		(1, -1, 1);
	set_color( 1, 1, 1);
	drawOneAxis();
	model_view = given_basis;
	model_view *= RotateY	(90);
	model_view *= Scale		(-1, 1, 1);
	set_color( 1, 0, 0 );
	drawOneAxis();
	model_view = given_basis;
	
	colors.pop();
	colors.pop();
	colors.pop();
	set_color( colors.top().r, colors.top().g, colors.top().b );
}

//END PRIMITIVE SHAPES





void drawGround(point3 spawn = point3(0,0,0)){
	mvstack.push(model_view);
    model_view *= Translate(spawn.x, spawn.y, spawn.z);
    model_view *= Scale(10000, 0.1, 10000);							        drawAxes(basis_id++);
    set_color(0,1,0);
    drawCube(texture_grass);
	model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
}

void drawSkybox(point3 scale = point3(1,1,1), point3 spawn = point3(0,0,0), GLuint texture = texture_sky){
    mvstack.push(model_view);
        model_view *= Translate(spawn.x,spawn.y,spawn.z);
        model_view *= Scale(scale.x,scale.y,scale.z);                           drawAxes(basis_id++);
        set_color(1,1,1);
        drawSky(texture);
    model_view = mvstack.top(); mvstack.pop();                              drawAxes(basis_id++);
}

void drawPlane(point3 spawn = point3(0,0,0)){
    set_color(1,1,1);
    //draw body
    mvstack.push(model_view);
        model_view *= Translate(spawn.x,spawn.y,spawn.z);
        mvstack.push(model_view);
            model_view *= Scale(14,3,2);
            drawSphere();
        model_view = mvstack.top(); mvstack.pop();
        //draw tail
        mvstack.push(model_view);
            model_view *= Translate(-13.75,0,0); 
                mvstack.push(model_view); 
                model_view *= RotateZ(20);
                model_view *= Translate(3/sin(20),4/sin(20),0);
                model_view *= Scale(6,8,0.5);
                drawCube();
            model_view = mvstack.top(); mvstack.pop();
            model_view *= Translate(3,3,0);
            model_view *= Scale(3,0.5,10);
            drawCube();
        model_view = mvstack.top(); mvstack.pop();
        //draw wings
        mvstack.push(model_view);
            model_view *= Translate(3,3,0);
            model_view *= Scale(6,0.25,25);
            drawCube();
        model_view = mvstack.top(); mvstack.pop();
    model_view = mvstack.top(); mvstack.pop();
}


void drawParachute() {
    mvstack.push(model_view);
        model_view *= Translate(0,15,0);
        model_view *= Scale(10,3,10);
        set_color(1,0,1);
        drawCube();
    model_view = mvstack.top(); mvstack.pop();
    mvstack.push(model_view);
        model_view *= Translate(0,7.5,-0.75);
        model_view *= Scale(0.1,15,0.1);
        set_color(0,0,0);
        drawCube();
    model_view = mvstack.top(); mvstack.pop();
}

struct Rotxyz{
    double x,y,z;

    Rotxyz() : x(0), y(0), z(0){}
    Rotxyz(double xx, double yy, double zz): x(xx), y(yy), z(zz){}
};
const Rotxyz zr = Rotxyz();

void drawDiver( point3 spawn = point3(0,0,0),
                Rotxyz rotateWhole = zr,
                Rotxyz rotateLeftForearm = zr, Rotxyz rotateRightForearm = zr, 
                Rotxyz rotateLeftBicep   = zr, Rotxyz rotateRightBicep   = zr,
                Rotxyz rotateLeftThigh   = zr, Rotxyz rotateRightThigh   = zr,
                Rotxyz rotateLeftCalf    = zr, Rotxyz rotateRightCalf    = zr,
                bool drawPar= false){

    model_view *= Translate(spawn.x, spawn.y,spawn.z);
    model_view *= RotateX(rotateWhole.x) * RotateY(rotateWhole.y) * RotateZ(rotateWhole.z);
    //center around torso cylinder
    mvstack.push(model_view);                                                                          
        model_view *= RotateX(90);
        model_view *= Scale(0.75,0.75,1.5);
        set_color(1,0,0);
        drawCylinder();
    model_view = mvstack.top(); mvstack.pop();
    //draw parachute pack
    mvstack.push(model_view);
        if (drawPar) drawParachute();
        model_view *= Translate(0,0,-0.75);
        model_view *= Scale(2,2,0.4);
        set_color(0,1,0);
        drawCube();
    model_view = mvstack.top(); mvstack.pop();
    //draw head
    mvstack.push(model_view);
        model_view *= Translate(0,1.5+1,0);
        set_color(1,1,0);
        drawSphere();
    model_view = mvstack.top(); mvstack.pop();
    //draw left arm
    mvstack.push(model_view);
        //draw shoulder sphere at top of other cylinder
        model_view *= Translate(1,1.2,0);
        mvstack.push(model_view);
            model_view *= Scale(0.25,0.25,0.25);
            set_color(1,1,1);
            drawSphere();
        model_view = mvstack.top(); mvstack.pop();
        //draw left bicep
        mvstack.push(model_view);
            model_view *= RotateX(rotateLeftBicep.x) * 
                          RotateY(rotateLeftBicep.y) * 
                          RotateZ(rotateLeftBicep.z);
            mvstack.push(model_view);
                model_view *= Translate(0.3,0,0);
                model_view *= RotateY(90);
                model_view *= Scale(0.25,0.25,0.3);
                set_color(1,1,0);
                drawCylinder();
            model_view = mvstack.top(); mvstack.pop();
            //move coordinates to new joint basis
            //JOINT //////////////////////////////////////// ELBOW ELBOW JOINT JOINT
            model_view *= Translate(0.6,0,0);
            //draw elbow joint
            mvstack.push(model_view);
                model_view *= Scale(0.25,0.25,0.25);
                set_color(1,1,1);
                drawSphere();
            model_view = mvstack.top(); mvstack.pop();
            //rotate forearm and its connections
            model_view *= RotateX(rotateLeftForearm.x) * 
                          RotateY(rotateLeftForearm.y) * 
                          RotateZ(rotateLeftForearm.z);
            //draw forearm
            mvstack.push(model_view);
                model_view *= Translate(0.3,0,0);
                model_view *= RotateY(90);
                model_view *= Scale(0.25,0.25,0.3);
                set_color(1,1,0);
                drawCylinder();
            model_view = mvstack.top(); mvstack.pop();
            //draw hand
            mvstack.push(model_view);
                model_view *= Translate(0.6+0.4,0,0);
                model_view *= Scale(0.4,0.4,0.4);
                set_color(0,0,1);
                drawSphere();
            model_view = mvstack.top(); mvstack.pop();
        model_view = mvstack.top(); mvstack.pop();
    model_view = mvstack.top(); mvstack.pop();
    //draw Right arm
    mvstack.push(model_view);
        //draw shoulder sphere at top of other 
        model_view *= Translate(-1,1.2,0);
        mvstack.push(model_view);
            model_view *= Scale(0.25,0.25,0.25);
            set_color(1,1,1);
            drawSphere();
        model_view = mvstack.top(); mvstack.pop();
        //draw Right bicep
        mvstack.push(model_view);
            model_view *= RotateX(rotateRightBicep.x) * 
                          RotateY(rotateRightBicep.y) * 
                          RotateZ(rotateRightBicep.z);
            mvstack.push(model_view);
                model_view *= Translate(-0.3,0,0);
                model_view *= RotateY(90);
                model_view *= Scale(0.25,0.25,0.3);
                set_color(1,1,0);
                drawCylinder();
            model_view = mvstack.top(); mvstack.pop();
            //move coordinates to new joint basis
            //JOINT //////////////////////////////////////// ELBOW ELBOW JOINT JOINT
            model_view *= Translate(-0.6,0,0);
            //draw elbow joint
            mvstack.push(model_view);
                model_view *= Scale(0.25,0.25,0.25);
                set_color(1,1,1);
                drawSphere();
            model_view = mvstack.top(); mvstack.pop();
            //rotate forearm and its connections
            model_view *= RotateX(rotateRightForearm.x) * 
                          RotateY(rotateRightForearm.y) * 
                          RotateZ(rotateRightForearm.z);
            //draw forearm
            mvstack.push(model_view);
                model_view *= Translate(-0.3,0,0);
                model_view *= RotateY(90);
                model_view *= Scale(0.25,0.25,0.3);
                set_color(1,1,0);
                drawCylinder();
            model_view = mvstack.top(); mvstack.pop();
            //draw hand
            mvstack.push(model_view);
                model_view *= Translate(-0.6-0.4,0,0);
                model_view *= Scale(0.4,0.4,0.4);
                set_color(0,0,1);
                drawSphere();
            model_view = mvstack.top(); mvstack.pop();
        model_view = mvstack.top(); mvstack.pop();
    model_view = mvstack.top(); mvstack.pop();

    //draw left leg
    mvstack.push(model_view);
        //draw pelvis sphere at top of other cylinder
        model_view *= Translate(0.5,-1.5,0);
        mvstack.push(model_view);
            model_view *= Scale(0.25,0.25,0.25);
            set_color(1,1,1);
            drawSphere();
        model_view = mvstack.top(); mvstack.pop();
        //draw left Thigh
        mvstack.push(model_view);
            model_view *= RotateX(rotateLeftThigh.x) * 
                          RotateY(rotateLeftThigh.y) * 
                          RotateZ(rotateLeftThigh.z);
            mvstack.push(model_view);
                model_view *= Translate(0,-0.4,0);
                model_view *= RotateX(90);
                model_view *= Scale(0.25,0.25,0.4);
                set_color(1,1,0);
                drawCylinder();
            model_view = mvstack.top(); mvstack.pop();
            //move coordinates to new joint basis
            //JOINT //////////////////////////////////////// KNEE KNEE JOINT JOINT
            model_view *= Translate(0,-0.8,0);
            //draw elbow joint
            mvstack.push(model_view);
                model_view *= Scale(0.25,0.25,0.25);
                set_color(1,1,1);
                drawSphere();
            model_view = mvstack.top(); mvstack.pop();
            //rotate Calf and its connections
            model_view *= RotateX(rotateLeftCalf.x) * 
                          RotateY(rotateLeftCalf.y) * 
                          RotateZ(rotateLeftCalf.z);
            //draw Calf
            mvstack.push(model_view);
                model_view *= Translate(0,-0.4,0);
                model_view *= RotateX(90);
                model_view *= Scale(0.25,0.25,0.4);
                set_color(1,1,0);
                drawCylinder();
            model_view = mvstack.top(); mvstack.pop();
            //draw hand
            mvstack.push(model_view);
                model_view *= Translate(0,-1,0);
                model_view *= Scale(0.4,0.4,0.4);
                set_color(0,0,1);
                drawSphere();
            model_view = mvstack.top(); mvstack.pop();
        model_view = mvstack.top(); mvstack.pop();
    model_view = mvstack.top(); mvstack.pop();

    //draw Right leg
    mvstack.push(model_view);
        //draw pelvis sphere at top of other cylinder
        model_view *= Translate(-0.5,-1.5,0);
        mvstack.push(model_view);
            model_view *= Scale(0.25,0.25,0.25);
            set_color(1,1,1);
            drawSphere();
        model_view = mvstack.top(); mvstack.pop();
        //draw Right Thigh
        mvstack.push(model_view);
            model_view *= RotateX(rotateRightThigh.x) * 
                          RotateY(rotateRightThigh.y) * 
                          RotateZ(rotateRightThigh.z);
            mvstack.push(model_view);
                model_view *= Translate(0,-0.4,0);
                model_view *= RotateX(90);
                model_view *= Scale(0.25,0.25,0.4);
                set_color(1,1,0);
                drawCylinder();
            model_view = mvstack.top(); mvstack.pop();
            //move coordinates to new joint basis
            //JOINT //////////////////////////////////////// KNEE KNEE JOINT JOINT
            model_view *= Translate(0,-0.8,0);
            //draw elbow joint
            mvstack.push(model_view);
                model_view *= Scale(0.25,0.25,0.25);
                set_color(1,1,1);
                drawSphere();
            model_view = mvstack.top(); mvstack.pop();
            //rotate Calf and its connections
            model_view *= RotateX(rotateRightCalf.x) * 
                          RotateY(rotateRightCalf.y) * 
                          RotateZ(rotateRightCalf.z);
            //draw Calf
            mvstack.push(model_view);
                model_view *= Translate(0,-0.4,0);
                model_view *= RotateX(90);
                model_view *= Scale(0.25,0.25,0.4);
                set_color(1,1,0);
                drawCylinder();
            model_view = mvstack.top(); mvstack.pop();
            //draw hand
            mvstack.push(model_view);
                model_view *= Translate(0,-1,0);
                model_view *= Scale(0.4,0.4,0.4);
                set_color(0,0,1);
                drawSphere();
            model_view = mvstack.top(); mvstack.pop();
        model_view = mvstack.top(); mvstack.pop();
    model_view = mvstack.top(); mvstack.pop();
}

// void drawShapes()
// {

// 	mvstack.push(model_view);q

//     model_view *= Translate	( 0, 3, 0 );									drawAxes(basis_id++);
//     model_view *= Scale		( 3, 3, 3 );									drawAxes(basis_id++);
//     set_color( .8, .0, .8 );
//     drawCube();

//     model_view *= Scale		( 1/3.0f, 1/3.0f, 1/3.0f );						drawAxes(basis_id++);
//     model_view *= Translate	( 0, 3, 0 );									drawAxes(basis_id++);
//     set_color( 0, 1, 0 );
//     drawCone();

//     model_view *= Translate	( 0, -9, 0 );									drawAxes(basis_id++);
//     set_color( 1, 1, 0 );
//     drawCylinder();

// 	model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
	
//     model_view *= Scale		( 1/3.0f, 1/3.0f, 1/3.0f );						drawAxes(basis_id++);

// 	drawGround();
// }

// void drawPlanets()
// {
//     set_color( .8, .0, .0 );	//model sun
//     mvstack.push(model_view);
//     model_view *= Scale(3);													drawAxes(basis_id++);
//     drawSphere(texture_earth);
//     model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
    
//     set_color( .0, .0, .8 );	//model earth
//     model_view *= RotateY	( 10*TIME );									drawAxes(basis_id++);
//     model_view *= Translate	( 15, 5*sin( 30*DegreesToRadians*TIME ), 0 );	drawAxes(basis_id++);
//     mvstack.push(model_view);
//     model_view *= RotateY( 300*TIME );										drawAxes(basis_id++);
//     drawCube();;
//     model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
    
//     set_color( .8, .0, .8 );	//model moon
//     model_view *= RotateY	( 30*TIME );									drawAxes(basis_id++);
//     model_view *= Translate	( 2, 0, 0);										drawAxes(basis_id++);
//     model_view *= Scale(0.2);												drawAxes(basis_id++);
//     drawCylinder();
	
// }

// void drawMidterm()
// {
// 	mvstack.push(model_view);
// 	mvstack.push(model_view);
// 	model_view *= Translate	( -1, 0, 0 );									drawAxes(basis_id++);
// 	model_view *= Scale		( 2, 1, 1 );									drawAxes(basis_id++);
// 	drawCube();
// 	model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
	
// 	model_view *= Scale		( 2, 1, 1 );									drawAxes(basis_id++);
// 	model_view *= Translate	( 1, 0, 0 );									drawAxes(basis_id++);
// 	drawCube();

	
// 	model_view *= Translate	( 0, 2, 0 );									drawAxes(basis_id++);
// 	model_view *= RotateZ	( 90 + 360 * TIME );							drawAxes(basis_id++);
// 	drawCube();
// 	model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
// }

const double scenePlaneTime     = 8,
             sceneJumpSpawnTime = scenePlaneTime + 3,
             sceneJumpTime      = 7+scenePlaneTime,
             sceneAirTime       = 10 + sceneJumpTime,
             scenePullTime      = 3 + sceneJumpTime,
             sceneParachuteTime = 2 + scenePullTime,
             sceneVerticalTime  = 1 + sceneParachuteTime;

void display(void)
{
	basis_id = 0;
    glClearColor( .1, .1, .2, 1 );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	set_color( .6, .6, .6 );

    model_view = LookAt( eye, ref, up );
	model_view *= orientation;
    model_view *= Scale(zoom);												drawAxes(basis_id++);

    //assign for scope
    static double bTime, 
                  lfTime,
                  rfTime,
                  lbTime,
                  rbTime,
                  ltTime,
                  rtTime,
                  lcTime,
                  rcTime,
                  pTime;
    static Rotxyz wb, lf, rf,
                  lb, rb, lt,
                  rt, lc, rc;


    if (TIME < scenePlaneTime){ //change back to 8 later
        if (TIME < 5){
            eye = RotateY(72*TIME+45) * RotateX(-20) * Translate(0,0,40) * ref;
            model_view = LookAt( eye, ref, up );
            model_view *= orientation;
        }
        drawPlane();
        drawSkybox(point3(1000,1000,1000), point3(-50*TIME,0,0));
        drawGround(point3(-50*TIME,-500,0));
    }
    else if (TIME < sceneJumpTime){
        model_view = LookAt(Translate(0,0,40)*eye, ref, up );
        model_view *= orientation;
        drawSkybox(point3(1000,1000,1000));
        drawGround(point3(0,-500,0));

        //in 6 seconds move 300 units: 50m/s
        drawPlane(point3(50*(TIME - scenePlaneTime)-150,0,0));
            //set up variables for diver
        if (TIME > sceneJumpSpawnTime){
            if (TIME - sceneJumpSpawnTime < 90 * DegreesToRadians) {
                bTime = TIME-sceneJumpSpawnTime, 
                lfTime = 0,
                rfTime = 0,
                lbTime = TIME-sceneJumpSpawnTime,
                rbTime = TIME-sceneJumpSpawnTime,
                ltTime = TIME-sceneJumpSpawnTime,
                rtTime = TIME-sceneJumpSpawnTime,
                lcTime = TIME-sceneJumpSpawnTime,
                rcTime = TIME-sceneJumpSpawnTime;

                wb = Rotxyz(90*sin(bTime),0,0), 
                lf = Rotxyz(),
                rf = Rotxyz(),
                lb = Rotxyz(0,0,-75+120*sin(lbTime)),
                rb = Rotxyz(0,0, 75-120*sin(rbTime)),
                lt = Rotxyz(0,0, 5+15*sin(ltTime)),
                rt = Rotxyz(0,0,-5-15*sin(rtTime)),
                lc = Rotxyz( 75*sin(lcTime),0,0),
                rc = Rotxyz( 75*sin(rcTime),0,0);


            }

            drawDiver(point3(25*(TIME - sceneJumpSpawnTime),-15*(TIME-sceneJumpSpawnTime)*(TIME-sceneJumpSpawnTime),3+TIME-sceneJumpSpawnTime), //spawn point
                  wb,           //whole body
                  lf,           //left forearm
                  rf,           //right forearm
                  lb,           //left bicep
                  rb,           //right bicep
                  lt,           //left thigh
                  rt,           //right thigh
                  lc,           //left calf
                  rc);          //right calf}
        }
    }
    else if (TIME < sceneAirTime){
        model_view = LookAt(RotateY(25)*RotateX(20)*Translate(0,0,10)*eye, ref, up);
        model_view *= orientation;

        if (TIME < sceneParachuteTime && TIME > scenePullTime){
             bTime = 0, 
            lfTime = TIME-scenePullTime,
            rfTime = 0,
            lbTime = TIME-scenePullTime,
            rbTime = 0,
            ltTime = 0,
            rtTime = 0,
            lcTime = 0,
            rcTime = 0;

            wb = Rotxyz(90,0,0), 
            lf = Rotxyz(0,0,115*sin(lfTime)),
            rf = Rotxyz(),
            lb = Rotxyz(90*sin(lbTime),0,-75+120+30*sin(lbTime)),
            rb = Rotxyz(0,0, 75-120),
            lt = Rotxyz(0,0, 5+15),
            rt = Rotxyz(0,0,-5-15),
            lc = Rotxyz(75,0,0),
            rc = Rotxyz(75,0,0);

        }
        else if (TIME > sceneParachuteTime && TIME < sceneVerticalTime){
             bTime = (TIME-sceneParachuteTime)*90*DegreesToRadians, 
            lfTime = (TIME-sceneParachuteTime)*90*DegreesToRadians,
            rfTime = 0,
            lbTime = (TIME-sceneParachuteTime)*90*DegreesToRadians,
            rbTime = 0,
            ltTime = 0,
            rtTime = 0,
            lcTime = 0,
            rcTime = 0;

            wb = Rotxyz(90-90*sin(bTime),0,0), 
            lf = Rotxyz(0,0,115-115*sin(lfTime)),
            rf = Rotxyz(),
            lb = Rotxyz(90-90*sin(lbTime),0,-75+120-30*sin(lbTime)),
            rb = Rotxyz(0,0, 75-120),
            lt = Rotxyz(0,0, 5+15),
            rt = Rotxyz(0,0,-5-15),
            lc = Rotxyz();
            rc = Rotxyz();

        }

        static bool drawPar = false;
        if (TIME > sceneParachuteTime) drawPar = true;
        drawSkybox(point3(1000,1000,1000), point3(0,-300+30*(TIME-scenePullTime),0));
        drawDiver(point3(0,0,0), wb ,lf, rf, lb, rb ,lt, rt, lc, rc, drawPar);

    }
    glutSwapBuffers();
}

void myReshape(int w, int h)	// Handles window sizing and resizing.
{    
    mat4 projection = Perspective( 50, (float)w/h, 1, 1000 );
    glUniformMatrix4fv( uProjection, 1, GL_FALSE, transpose(projection) );
	glViewport(0, 0, g_width = w, g_height = h);	
}		

void instructions() {	 std::cout <<	"Press:"									<< '\n' <<
										"  r to restore the original view."			<< '\n' <<
										"  0 to restore the original state."		<< '\n' <<
										"  a to toggle the animation."				<< '\n' <<
										"  b to show the next basis's axes."		<< '\n' <<
										"  B to show the previous basis's axes."	<< '\n' <<
										"  q to quit."								<< '\n';	}

void myKey(unsigned char key, int x, int y)
{
    switch (key) {
        case 'q':   case 27:				// 27 = esc key
            exit(0); 
		case 'b':
			std::cout << "Basis: " << ++basis_to_display << '\n';
			break;
		case 'B':
			std::cout << "Basis: " << --basis_to_display << '\n';
			break;
        case 'a':							// toggle animation           		
            animate = 1 - animate ; 
            break ;
		case '0':							// Add code to reset your object here.
			TIME = 0;	TM.Reset() ;											
        case 'r':
			orientation = mat4();			
            break ;
    }
    glutPostRedisplay() ;
}

int main() 
{
	char title[] = "Title";
	int argcount = 1;	 char* title_ptr = title;
	glutInit(&argcount,		 &title_ptr);
	glutInitWindowPosition (230, 70);
	glutInitWindowSize     (g_width, g_height);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow(title);
	#if !defined(__APPLE__) && !defined(EMSCRIPTEN)
		glewExperimental = GL_TRUE;
		glewInit();
	#endif
    std::cout << "GL version " << glGetString(GL_VERSION) << '\n';
	instructions();
	init();

	glutDisplayFunc(display);
    glutIdleFunc(idleCallBack) ;
    glutReshapeFunc (myReshape);
    glutKeyboardFunc( myKey );
    glutMouseFunc(myMouseCallBack) ;
    glutMotionFunc(myMotionCallBack) ;
    glutPassiveMotionFunc(myPassiveMotionCallBack) ;

	glutMainLoop();
}
