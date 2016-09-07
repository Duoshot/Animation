// CPSC 587 Created By: Andrew Owens
// This is a (very) basic program to
// 1) load shaders from external files, and make a shader program
// 2) make Vertex Array Object and Vertex Buffer Object for the triangle

// take a look at the following sites for further readings:
// opengl-tutorial.org -> The first triangle (New OpenGL, great start)
// antongerdelan.net -> shaders pipeline explained
// ogldev.atspace.co.uk -> good resource 


// NOTE: this dependencies (include/library files) will need to be tweaked if
// you wish to run this on a non lab computer

#include<iostream>
#include<string>
#include<sstream>
#include<cmath>

#include<GL/glew.h> // include BEFORE glut
#include<GL/glut.h>

#include "ShaderTools.h"
#include "Vec3f.h"
#include "Mat4f.h"
#include "OpenGLMatrixTools.h"

using std::cout;
using std::endl;
using std::cerr;
using namespace std;

GLuint vaoID;
GLuint basicProgramID;

// Could store these two in an array GLuint[]
GLuint vertBufferID;
GLuint colorBufferID;

Mat4f MVP;
Mat4f M;
Mat4f V;
Mat4f P;

int WIN_WIDTH = 800, WIN_HEIGHT = 600;

//Particle struct
struct Particle
{
	Vec3f position;			//position of the particle
	Vec3f velocity;			//velocity of the particle
	Vec3f force;			//force on the particle
	float mass;				//mass of the particle
	int index;				//index of the particle
};

//Spring struct
struct Spring
{
	int Pi;					//index of the first particle
	int Pj;					//index of the second particle
	float restLength;		//rest length of the spring
	float springConstant;	//spring constant
};

vector <Particle> particles;	//array of particles
vector <Spring> springs;		//array of springs
vector <Vec3f> particleVerts;	//array of particle vertex
vector <float> colors;			//colors

Vec3f globalForce(0, -9.81, 0);		//gravity
float deltaTime = 0.004;					//delta time
float spring_friction = 0.5;			//damping force

string choice;		//Choose what to display. (1)Mass on Spring, (2)Pendulum, (3)Jello Cube, (4)Hanging Cloth, (5) Flapping flag

void update();						//update
void updateTimes(int times);

// function declarations... just to keep tho ings kinda organized.
void displayFunc();
void resizeFunc();
void idleFunc();
void init();
void generateIDs();
void deleteIDs();
void setupVAO();
void reloadVertexBuffer();
void loadBuffer();
void loadProjectionMatrix();
void loadModelViewMatrix();
void setupModelViewProjectionTransform();
void reloadMVPUniform();
int main(int, char**);
// function declarations

void displayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use our shader
	glUseProgram(basicProgramID);

	// Use VAO that holds buffer bindings
	// and attribute config of buffers
	glBindVertexArray(vaoID);
	// Draw Quads, start at vertex 0, draw 4 of them (for a quad)
	glDrawArrays(GL_LINES, 0, particleVerts.size());

	glutSwapBuffers();
}

float changeY = 0;

void idleFunc()
{
	// every frame refresh, rotate quad around y axis by 1 degree
	//	MVP = MVP * RotateAboutYMatrix( 1.0 );
	//M = M * RotateAboutYMatrix(1.0);
	
	//cout << particles.at(1).position.y() << endl;
	
	//changeY -= 0.001;

	//cout << changeY << endl;
	//particles.at(1).position.y(changeY);

	if(choice == "3" || choice == "4" || choice == "5" || choice == "6")
		update();
	else
		updateTimes(3);
	
	
	reloadVertexBuffer();
	
	setupModelViewProjectionTransform();

	// send changes to GPU
	reloadMVPUniform();

	glutPostRedisplay();
}

void resizeFunc(int width, int height)
{
	WIN_WIDTH = width;
	WIN_HEIGHT = height;

	glViewport(0, 0, width, height);

	loadProjectionMatrix();
	reloadMVPUniform();

	glutPostRedisplay();
}

void generateIDs()
{
	std::string vsSource = loadShaderStringfromFile("./basic_vs.glsl");
	std::string fsSource = loadShaderStringfromFile("./basic_fs.glsl");
	basicProgramID = CreateShaderProgram(vsSource, fsSource);

	// load IDs given from OpenGL
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vertBufferID);
	glGenBuffers(1, &colorBufferID);
}

void deleteIDs()
{
	glDeleteProgram(basicProgramID);
	glDeleteVertexArrays(1, &vaoID);
	glDeleteBuffers(1, &vertBufferID);
	glDeleteBuffers(1, &colorBufferID);
}

void loadProjectionMatrix()
{
	// Perspective Only

	// field of view angle 60 degrees
	// window aspect ratio
	// near Z plane > 0
	// far Z plane

	P = PerspectiveProjection(60, // FOV
		static_cast<float>(WIN_WIDTH) / WIN_HEIGHT, // Aspect
		0.01,   // near plane
		500); // far plane depth
}

void loadModelViewMatrix()
{
	M = UniformScaleMatrix(0.25);	// scale Quad First
	if(choice == "4" || choice == "5" || choice == "6")
		M = TranslateMatrix(0,0,-6.0) * M;
	else	
		M = TranslateMatrix(0, 0, -3.5) * M;	// translate away from (0,0,0)
	

											// view doesn't change, but if it did you would use this
	V = IdentityMatrix();
}

void setupModelViewProjectionTransform()
{
	MVP = P * V * M; // transforms vertices from right to left (odd huh?)
}

void reloadMVPUniform()
{
	GLint mvpID = glGetUniformLocation(basicProgramID, "MVP");

	glUseProgram(basicProgramID);
	glUniformMatrix4fv(mvpID,		// ID
		1,		// only 1 matrix
		GL_TRUE,	// transpose matrix, Mat4f is row major
		MVP.data()	// pointer to data in Mat4f
		);
}

void setupVAO()
{
	glBindVertexArray(vaoID);

	glEnableVertexAttribArray(0); // match layout # in shader
	glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
	glVertexAttribPointer(
		0,		// attribute layout # above
		3,		// # of components (ie XYZ )
		GL_FLOAT,	// type of components
		GL_FALSE,	// need to be normalized?
		0,		// stride
		(void*)0	// array buffer offset
		);

	glEnableVertexAttribArray(1); // match layout # in shader
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
	glVertexAttribPointer(
		1,		// attribute layout # above
		3,		// # of components (ie XYZ )
		GL_FLOAT,	// type of components
		GL_FALSE,	// need to be normalized?
		0,		// stride
		(void*)0	// array buffer offset
		);

	glBindVertexArray(0); // reset to default		
}

void createMassOnASpring()
{
	//create particles
	Particle p1;
	p1.index = 0;
	p1.position = Vec3f(0,0,0);
	p1.force = Vec3f(0, 0, 0);
	p1.mass = 1000000000;
	p1.velocity = Vec3f(0, 0, 0);
	particles.push_back(p1);

	Particle p2;
	p2.index = 1;
	p2.position = Vec3f(5,-5,0);
	p2.force = Vec3f(0, 0, 0);
	p2.mass = 1;
	p2.velocity = Vec3f(0, 0, 0);
	particles.push_back(p2);

	//attach springs
	Spring p1p2;
	p1p2.Pi = p1.index;
	p1p2.Pj = p2.index;
	p1p2.springConstant = 20;
	p1p2.restLength = 0.5;
	springs.push_back(p1p2);

	particleVerts.clear();

	for (int i = 0; i < springs.size(); i++)
	{
		int s1 = springs.at(i).Pi;
		int s2 = springs.at(i).Pj;

		particleVerts.push_back(particles.at(s1).position);
		particleVerts.push_back(particles.at(s2).position);
	}
}

void createPendulum()
{
	//create particles
	Particle p1;
	p1.index = 0;
	p1.position = Vec3f(0, 2, 0);
	p1.force = Vec3f(0, 0, 0);
	p1.mass = 1000000000;
	p1.velocity = Vec3f(0, 0, 0);
	particles.push_back(p1);

	Particle p2;
	p2.index = 1;
	p2.position = Vec3f(0, 0, 0);
	p2.force = Vec3f(0, 0, 0);
	p2.mass = 0.1;
	p2.velocity = Vec3f(0, 0, 0);
	particles.push_back(p2);

	Particle p3;
	p3.index = 2;
	p3.position = Vec3f(2, -2, 0);
	p3.force = Vec3f(0, 0, 0);
	p3.mass = 0.1;
	p3.velocity = Vec3f(0, 0, 0);
	particles.push_back(p3);

	Particle p4;
	p4.index = 3;
	p4.position = Vec3f(-2, -4, 0);
	p4.force = Vec3f(0, 0, 0);
	p4.mass = 0.1;
	p4.velocity = Vec3f(0, 0, 0);
	particles.push_back(p4);

	Particle p5;
	p5.index = 4;
	p5.position = Vec3f(4, -4, 0);
	p5.force = Vec3f(0, 0, 0);
	p5.mass = 0.1;
	p5.velocity = Vec3f(0, 0, 0);
	particles.push_back(p5);

	//attach springs
	Spring s1;
	s1.Pi = p1.index;
	s1.Pj = p2.index;
	s1.springConstant = 20;
	s1.restLength = 1;
	springs.push_back(s1);

	Spring s2;
	s2.Pi = p2.index;
	s2.Pj = p3.index;
	s2.springConstant = 20;
	s2.restLength = 1;
	springs.push_back(s2);
	
	Spring s3;
	s3.Pi = p3.index;
	s3.Pj = p4.index;
	s3.springConstant = 20;
	s3.restLength = 1;
	springs.push_back(s3);
	
	Spring s4;
	s4.Pi = p4.index;
	s4.Pj = p5.index;
	s4.springConstant = 20;
	s4.restLength = 1;
	springs.push_back(s4);

	particleVerts.clear();

	for (int i = 0; i < springs.size(); i++)
	{
		int s1 = springs.at(i).Pi;
		int s2 = springs.at(i).Pj;

		particleVerts.push_back(particles.at(s1).position);
		particleVerts.push_back(particles.at(s2).position);
	}
}

//distance adding
void attachJelloSprings()
{
	for (int i = 0; i < particles.size(); i++)
	{
		for (int j = i+1; j < particles.size(); j++)
		{
			if (particles.at(i).position.distance(particles.at(j).position) < 1.6)	//1.6 is diagonal distance
			{
				Spring newSpring;
				newSpring.Pi = i;
				newSpring.Pj = j;
				newSpring.restLength = particles.at(i).position.distance(particles.at(j).position) * 0.8;
				newSpring.springConstant = 200;
				springs.push_back(newSpring);
			}
		}
	}
}

void createJelloCube()
{
	float startX = -3;
	float startY = -3;
	float startZ = 0;

	//loop at creates particles base on boundaries
	for (int i = 0; i < 7*7*7; i++)
	{
		Particle part;
		part.index = i;
		part.position = Vec3f(startX, startY, startZ);
		part.force = Vec3f(0, 0, 0);
		part.mass = 0.1;
		part.velocity = Vec3f(0, 0, 0);
		particles.push_back(part);


		startX += 1;
		if (startX > 3)
		{
			startX = -3;
			startY += 1;
		}
		if (startY > 3)
		{
			startX = -3;
			startY = -3;
			startZ -= 1;
		}
	}

	attachJelloSprings();

	particleVerts.clear();

	for (int i = 0; i < springs.size(); i++)
	{
		int s1 = springs.at(i).Pi;
		int s2 = springs.at(i).Pj;

		particleVerts.push_back(particles.at(s1).position);
		particleVerts.push_back(particles.at(s2).position);
	}
}

//distance adding
void attachClothSprings()
{
	for (int i = 0; i < particles.size(); i++)
	{
		for (int j = i + 1; j < particles.size(); j++)
		{
			if (particles.at(i).position.distance(particles.at(j).position) <= 2)	//2 is distance two particles away
			{
				Spring newSpring;
				newSpring.Pi = i;
				newSpring.Pj = j;
				newSpring.restLength = particles.at(i).position.distance(particles.at(j).position) * 0.9;
				newSpring.springConstant = 200;
				springs.push_back(newSpring);
			}
		}
	}
}

void createHangingCloth()
{
	float startX = -6;
	float startY = -6;

	for (int i = 0; i < 13*13; i++)
	{
		Particle part;
		part.index = i;
		part.position = Vec3f(startX, startY, 0);
		part.force = Vec3f(0, 0, 0);
		if ((startX == -6 && startY == 6) || (startX == 6 && startY == 6))
		{
			part.mass = 1000000;
		}
		else
		{
			part.mass = 0.1;
		}
		part.velocity = Vec3f(0, 0, 0);
		particles.push_back(part);

		
		startX += 1;
		if (startX > 6)
		{
			startX = -6;
			startY += 1;
		}
	}

	attachClothSprings();

	particleVerts.clear();

	for (int i = 0; i < springs.size(); i++)
	{
		int s1 = springs.at(i).Pi;
		int s2 = springs.at(i).Pj;

		particleVerts.push_back(particles.at(s1).position);
		particleVerts.push_back(particles.at(s2).position);
	}

}


void attachFlagSprings()
{
	for (int i = 0; i < particles.size(); i++)
	{
		for (int j = i + 1; j < particles.size(); j++)
		{
			if (particles.at(i).position.distance(particles.at(j).position) < 1.6)	//1.6 is diagonal distance
			{
				Spring newSpring;
				newSpring.Pi = i;
				newSpring.Pj = j;
				newSpring.restLength = particles.at(i).position.distance(particles.at(j).position) * 0.85;
				newSpring.springConstant = 400;
				springs.push_back(newSpring);
			}
		}
	}
}

void createFlag()
{
	
	//25 particles

	float startX = -6;
	float startY = -6;

	for (int i = 0; i < 13*13; i++)
	{
		Particle part;
		part.index = i;
		part.position = Vec3f(startX, startY, 0);
		part.force = Vec3f(0, 0, 0);
		if ((startX == -6 && startY == 6) || (startX == -6 && startY == -6))
		{
			part.mass = 1000000;
		}
		else
		{
			part.mass = 0.1;
		}
		part.velocity = Vec3f(0, 0, 0);
		particles.push_back(part);

		
		startX += 1;
		if (startX > 6)
		{
			startX = -6;
			startY += 1;
		}
	}

	attachFlagSprings();
	
	//lazy way to create flagpole. a spring with really heavy weights
	Particle p1;
	p1.index = particles.size();
	p1.position = Vec3f(-6, -10, 0);
	p1.force = Vec3f(0, 0, 0);
	p1.mass = 1000000000;
	p1.velocity = Vec3f(0, 0, 0);
	particles.push_back(p1);
	
	Particle p2;
	p2.index = particles.size();
	p2.position = Vec3f(-6, 6, 0);
	p2.force = Vec3f(0, 0, 0);
	p2.mass = 1000000000;
	p2.velocity = Vec3f(0, 0, 0);
	particles.push_back(p2);
	
		
	Spring s1;
	s1.Pi = p1.index;
	s1.Pj = p2.index;
	s1.springConstant = 20;
	s1.restLength = 1;
	springs.push_back(s1);
	

	particleVerts.clear();

	for (int i = 0; i < springs.size(); i++)
	{
		int s1 = springs.at(i).Pi;
		int s2 = springs.at(i).Pj;

		particleVerts.push_back(particles.at(s1).position);
		particleVerts.push_back(particles.at(s2).position);
	}
}



void attachDrapeSprings()
{
	for (int i = 0; i < particles.size(); i++)
	{
		for (int j = i + 1; j < particles.size(); j++)
		{
			if (particles.at(i).position.distance(particles.at(j).position) <= 1.6)
			{
				Spring newSpring;
				newSpring.Pi = i;
				newSpring.Pj = j;
				newSpring.restLength = particles.at(i).position.distance(particles.at(j).position) * 0.95;
				newSpring.springConstant = 500;
				springs.push_back(newSpring);
			}
		}
	}
}

void createDrapes()
{
	float startX = -6;
	float startY = -6;

	for (int i = 0; i < 13*13; i++)
	{
		Particle part;
		part.index = i;
		part.position = Vec3f(startX, startY, 0);
		part.force = Vec3f(0, 0, 0);
		if (startY == 6)
		{
			part.mass = 1000000;
		}
		else
		{
			part.mass = 0.1;
		}
		part.velocity = Vec3f(0, 0, 0);
		particles.push_back(part);

		
		startX += 1;
		if (startX > 6)
		{
			startX = -6;
			startY += 1;
		}
	}

	attachDrapeSprings();
	
	//drawing the window. It is merely 4 springs with super heavy masses :D
	Particle p1;
	p1.index = particles.size();
	p1.position = Vec3f(-6, -6, 0);
	p1.force = Vec3f(0, 0, 0);
	p1.mass = 1000000000;
	p1.velocity = Vec3f(0, 0, 0);
	particles.push_back(p1);
	
	Particle p2;
	p2.index = particles.size();
	p2.position = Vec3f(-6, 6, 0);
	p2.force = Vec3f(0, 0, 0);
	p2.mass = 1000000000;
	p2.velocity = Vec3f(0, 0, 0);
	particles.push_back(p2);
	
	Particle p3;
	p3.index = particles.size();
	p3.position = Vec3f(6, 6, 0);
	p3.force = Vec3f(0, 0, 0);
	p3.mass = 1000000000;
	p3.velocity = Vec3f(0, 0, 0);
	particles.push_back(p3);
	
	Particle p4;
	p4.index = particles.size();
	p4.position = Vec3f(6, -6, 0);
	p4.force = Vec3f(0, 0, 0);
	p4.mass = 1000000000;
	p4.velocity = Vec3f(0, 0, 0);
	particles.push_back(p4);
	
	Spring s1;
	s1.Pi = p1.index;
	s1.Pj = p2.index;
	s1.springConstant = 20;
	s1.restLength = 1;
	springs.push_back(s1);

	Spring s2;
	s2.Pi = p2.index;
	s2.Pj = p3.index;
	s2.springConstant = 20;
	s2.restLength = 1;
	springs.push_back(s2);
	
	Spring s3;
	s3.Pi = p3.index;
	s3.Pj = p4.index;
	s3.springConstant = 20;
	s3.restLength = 1;
	springs.push_back(s3);
	
	Spring s4;
	s4.Pi = p4.index;
	s4.Pj = p1.index;
	s4.springConstant = 20;
	s4.restLength = 1;
	springs.push_back(s4);

	particleVerts.clear();

	for (int i = 0; i < springs.size(); i++)
	{
		int s1 = springs.at(i).Pi;
		int s2 = springs.at(i).Pj;

		particleVerts.push_back(particles.at(s1).position);
		particleVerts.push_back(particles.at(s2).position);
	}

}

void updateTimes(int times)
{
	for (int i = 0; i < times; i++)
	{
		update();
	}
}

float wind = 10.f;
bool inc = true;

float sineWaveFlag()
{
	if(inc)
	{
		wind += 0.005;
		if(wind > 25)
		{
			inc = false;
		}
	}
	else
	{
		wind -= 0.001;
		if(wind < 10)
		{
			inc = true;
		}
	}
		
	return wind;
}

float sineWaveDrapes()
{
	if(inc)
	{
		wind += 0.005;
		if(wind > 10)
		{
			inc = false;
		}
	}
	else
	{
		wind -= 0.001;
		if(wind < 0)
		{
			inc = true;
		}
	}
		
	return wind;
}

Vec3f windForce()
{
	Vec3f wF;
	if(choice == "5")
		wF = Vec3f(sineWaveFlag(), 0, 0);
	else if(choice == "6")
		wF = Vec3f(sineWaveDrapes(), 0, 0);
	else
	{
		wF = Vec3f(0, 0, 0);
	}
	return wF;
}

void update()
{
	//for each spring in S, Sij
	//Pi = P[Si]
	//Pj = P[Sj]
	//fij = -S.k(||Pj - Pi|| - S.restlength) * Pj - Pi / |Pj - Pi| - Kd(Pi.v dot n - Pj.v dot n) * n
	//n = Pj - Pi / |Pj - Pi|
	//Pi.f += fij
	//Pj.f -= fij

	for (int i = 0; i < springs.size(); i++)
	{
		int indexI = springs.at(i).Pi;
		int indexJ = springs.at(i).Pj;

		Particle & Pi = particles.at(indexI);
		Particle & Pj = particles.at(indexJ);

		float springConstant = springs.at(i).springConstant;

		Vec3f startToEnd = Pj.position - Pi.position;
		Vec3f normal = startToEnd.normalized();

		Vec3f springForce = normal * (-springConstant * (startToEnd.length() - springs.at(i).restLength));

		Vec3f dampeningForce = normal * (Pi.velocity.dotProduct(normal) - Pj.velocity.dotProduct(normal)) * spring_friction;

		Vec3f force = springForce + dampeningForce;

		Pi.force = Pi.force - force;
		Pj.force = Pj.force + force;
	}

	for (int i = 0; i < particles.size(); i++)
	{
		Particle & p = particles.at(i);
		
		p.force = p.force + globalForce + windForce();

		p.velocity = p.velocity + (p.force * 1 / p.mass * deltaTime);

		p.position = p.position + (p.velocity * deltaTime);
		
		if (choice == "3")
		{
			if (p.position.y() < -3)
			{
				p.position.y(-3);
				p.velocity = (p.velocity * -1) * 0.5;
			}
		}
		
		p.force = Vec3f(0, 0, 0);
	}

}

void loadBuffer()
{
	if (choice == "1")
	{
		cout << "Creating Mass On A Spring" << endl;
		createMassOnASpring();
	}
	else if (choice == "2")
	{
		cout << "Creating Pendulum" << endl;
		createPendulum();
	}
	else if (choice == "3")
	{
		cout << "Creating Jello Cube" << endl;
		createJelloCube();
	}

	else if (choice == "4")
	{
		cout << "Creating Hanging Cloth" << endl;
		createHangingCloth();
	}

	else if (choice == "5")
	{
		cout << "Creating Flag" << endl;
		createFlag();
	}
	else if (choice == "6")
	{
		cout << "Creating Drapes" << endl;
		createDrapes();
	}
	else
	{
		cout << "Number not in range" << endl;
		exit(0);
	}
	

	glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(Vec3f) * particleVerts.size(),	// byte size of Vec3f, 4 of them
		particleVerts.data(),		// pointer (Vec3f*) to contents of verts
		GL_STATIC_DRAW);	// Usage pattern of GPU buffer
		
	for (int i = 0; i < particleVerts.size(); i++)
	{
		colors.push_back(1.0f);
		colors.push_back(1.0f);
		colors.push_back(1.0f);
	}

	glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * colors.size(),
		colors.data(),
		GL_STATIC_DRAW);
}

void reloadVertexBuffer()
{

	particleVerts.clear();

	for (int i = 0; i < springs.size(); i++)
	{
		int s1 = springs.at(i).Pi;
		int s2 = springs.at(i).Pj;

		particleVerts.push_back(particles.at(s1).position);
		particleVerts.push_back(particles.at(s2).position);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(Vec3f) * particleVerts.size(),	// byte size of Vec3f, 4 of them
		particleVerts.data(),		// pointer (Vec3f*) to contents of verts
		GL_STATIC_DRAW);	// Usage pattern of GPU buffer
}


void init()
{
	glEnable(GL_DEPTH_TEST);

	// SETUP SHADERS, BUFFERS, VAOs
	
	generateIDs();
	setupVAO();
	loadBuffer();

	loadModelViewMatrix();
	loadProjectionMatrix();
	setupModelViewProjectionTransform();
	reloadMVPUniform();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	// Setup FB configuration
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutInitWindowPosition(0, 0);

	glutCreateWindow("Assignment 3 Gorman Law");

	glewExperimental = true; // Needed in Core Profile
							 // Comment out if you want to us glBeign etc...
	if (glewInit() != GLEW_OK)
	{
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	cout << "GL Version: :" << glGetString(GL_VERSION) << endl;

	cout << "What model would you like to display?" << endl;
	cout << "(1)Mass on a spring" << endl;
	cout << "(2)Pendulum" << endl;
	cout << "(3)Jello Cube" << endl;
	cout << "(4)Hanging cloth" << endl;
	cout << "(5)Flag in the wind (Bonus)" << endl;
	cout << "(6)Window drapes in the wind (Bonus)" << endl;

	getline(cin, choice);

	cout << "Your choice was: " << choice << endl;

	glutDisplayFunc(displayFunc);
	glutReshapeFunc(resizeFunc);
	glutIdleFunc(idleFunc);

	init(); // our own initialize stuff func

	glutMainLoop();

	// clean up after loop
	deleteIDs();

	return 0;
}
