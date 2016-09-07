//Gorman Law
// 10053193
//December 7 2015
// CPSC 587 Assignment 4 Boids
// TA: Andrew Owens


// NOTE: this dependencies (include/library files) will need to be tweaked if
// you wish to run this on a non lab computer

#include<iostream>
#include<cmath>

#include<GL/glew.h> // include BEFORE glut
#include<GL/glut.h>

//for file reading
#include<iostream>
#include<cmath>
#include<sstream>
#include <iostream>
#include <string>
#include <cstddef>
#include <vector>
#include <sstream>
#include <iterator>
#include <fstream>
#include <stdexcept>

#include "ShaderTools.h"
#include "Vec3f.h"
#include "Mat4f.h"
#include "OpenGLMatrixTools.h"

using std::cout;
using std::endl;
using std::cerr;
using namespace std;


//boid stuff
GLuint vaoID;
GLuint basicProgramID;
GLuint vertBufferID;
GLuint colorBufferID;

//sphere stuff
GLuint sphereID;
GLuint sphereProgramID;
GLuint sphereVertBufferID;
GLuint sphereColorBufferID;

// Could store these two in an array GLuint[]


Mat4f MVP;
Mat4f sphereMVP;
Mat4f M;
Mat4f V;
Mat4f P;

int WIN_WIDTH = 800, WIN_HEIGHT = 600;

// function declarations... just to keep things kinda organized.
void displayFunc();
void resizeFunc();
void idleFunc();
void init();
void generateIDs();
void deleteIDs();
void setupVAO();
void loadBuffer();
void reloadVertexBuffer();
void loadProjectionMatrix();
void loadModelViewMatrix();
void setupModelViewProjectionTransform();
void reloadMVPUniform();
void reloadSphereMVPUniform();
int main(int, char**);
// function declarations

float RADIUS_AVOIDANCE = 2;
float RADIUS_COHERANCE = 4;
float RADIUS_MERGING = 6;
float MAX_ACCEL = 12;
float velocity_scale = 1;
float BOUNDING_BOX = 20;

float deltaTime = 0.01;

float sphereRadius;
Vec3f spherePosition;


vector <Vec3f> verts;
vector <Vec3f> velocities;
double PI = 3.14159;
vector<Vec3f> sphereVerts;

int num_of_boids;

struct Boid
{
	Vec3f position;		//position
	Vec3f velocity;		//velocity
	Vec3f head;			//where the nose points
	Vec3f up;			//up direction
	Vec3f right;		//right direction
	Vec3f left;
	Vec3f accumulator;	//accumulate velocities
};

vector <Boid> boidVector;

//code source: https://sites.google.com/site/drunkdevsltd/tutorials/draw-a-sphere-using-opengl-3-3
void create(float Radius, int Resolution, float x, float y, float z) {
	double PI = 3.14159;


	// vectors to hold our data
	// vertice positions


	// iniatiate the variable we are going to use
	float X1, Y1, X2, Y2, Z1, Z2;
	float inc1, inc2, inc3, inc4, inc5, Radius1, Radius2;

	for (int w = 0; w < Resolution; w++) {
		for (int h = (-Resolution / 2); h < (Resolution / 2); h++) {


			inc1 = (w / (float)Resolution) * 2 * PI;
			inc2 = ((w + 1) / (float)Resolution) * 2 * PI;

			inc3 = (h / (float)Resolution)*PI;
			inc4 = ((h + 1) / (float)Resolution)*PI;


			X1 = sin(inc1);
			Y1 = cos(inc1);
			X2 = sin(inc2);
			Y2 = cos(inc2);

			// store the upper and lower radius, remember everything is going to be drawn as triangles
			Radius1 = Radius*cos(inc3);
			Radius2 = Radius*cos(inc4);




			Z1 = Radius*sin(inc3);
			Z2 = Radius*sin(inc4);

			// insert the triangle coordinates
			sphereVerts.push_back(Vec3f(Radius1*X1+x, Z1+y, Radius1*Y1+z));
			sphereVerts.push_back(Vec3f(Radius1*X2+x, Z1+y, Radius1*Y2+z));
			sphereVerts.push_back(Vec3f(Radius2*X2+x, Z2+y, Radius2*Y2+z));



			sphereVerts.push_back(Vec3f(Radius1*X1+x, Z1+y, Radius1*Y1+z));
			sphereVerts.push_back(Vec3f(Radius2*X2+x, Z2+y, Radius2*Y2+z));
			sphereVerts.push_back(Vec3f(Radius2*X1+x, Z2+y, Radius2*Y1+z));

		}
	}
}

//from Andrew Owens tutorial. Load positions
void loadV3fFromFile(vector <Vec3f> vecs, std::string const & fileName, int fileType)
{
	using std::string;
	using std::stringstream;
	using std::istream_iterator;

	std::ifstream file(fileName);

	if (!file)
	{
		throw std::runtime_error("Unable to open file.");
	}

	string line;
	size_t index;
	stringstream ss(std::ios_base::in);

	size_t lineNum = 0;
	vecs.clear();

	while (getline(file, line))
	{
		++lineNum;

		// remove comments	
		index = line.find_first_of("#");
		if (index != string::npos)
		{
			line.erase(index, string::npos);
		}

		// removes leading/tailing junk
		line.erase(0, line.find_first_not_of(" \t\r\n\v\f"));
		index = line.find_last_not_of(" \t\r\n\v\f") + 1;
		if (index != string::npos)
		{
			line.erase(index, string::npos);
		}

		if (line.empty())
		{
			continue; // empty or commented out line
		}

		ss.str(line);
		ss.clear();

		Vec3f v;

		float x, y, z, r;
		if ((ss >> x >> y >> z >> r) && (!ss || !ss.eof() || ss.good()))
		{
			throw std::runtime_error("Error read file: "
				+ line
				+ " (line: "
				+ std::to_string(lineNum)
				+ ")");
		}
		else
		{
			cout << x << y << z << r << endl;
			if (fileType == 0)
			{
				num_of_boids = x;
			}
			else if (fileType == 2)
			{
				spherePosition = Vec3f(x, y, z);
				sphereRadius = r;
			}
		}
	}
	file.close();
}

void accumulateVelocity()
{
	for (int i = 0; i < boidVector.size(); i++)
	{
		for (int j = 0; j < boidVector.size(); j++)
		{
			if (i != j)
			{
				Boid & boid_i = boidVector.at(i);
				Boid & boid_j = boidVector.at(j);

				float length = boid_i.position.distance(boid_j.position);
				Vec3f direction = (boid_j.position - boid_i.position).normalized();

				if (length < RADIUS_AVOIDANCE)
				{
					boid_i.accumulator = boid_i.accumulator - direction * 0.01;
				}
				else if (length < RADIUS_COHERANCE)
				{
					boid_i.accumulator = boid_i.accumulator + boid_j.velocity * 0.01;
				}
				else if (length < RADIUS_MERGING)
				{
					boid_i.accumulator = boid_i.accumulator + direction * 0.1;
				}
				else
				{
					boid_i.accumulator = boid_i.accumulator;
				}

			}
		}
	}

} 

void updateVelocity()
{
	for (int i = 0; i < boidVector.size(); i++)
	{
		
		boidVector.at(i).velocity = boidVector.at(i).velocity + (boidVector.at(i).accumulator * velocity_scale);
		if (boidVector.at(i).velocity.length() > MAX_ACCEL)
		{
			boidVector.at(i).velocity = boidVector.at(i).velocity / boidVector.at(i).velocity.length() * MAX_ACCEL;
		}
	}
}

void updatePosition()
{
	for (int i = 0; i < boidVector.size(); i++)
	{
		boidVector.at(i).position = boidVector.at(i).position + (boidVector.at(i).velocity * deltaTime);
	}
}

void checkCollision()
{
	for (int i = 0; i < boidVector.size(); i++)
	{
		float distanceFromSphere = (boidVector.at(i).position - spherePosition).length();
		if (distanceFromSphere < sphereRadius + 2)
		{
			boidVector.at(i).accumulator = boidVector.at(i).accumulator + (boidVector.at(i).position - spherePosition).normalized() * 0.75;
		}
	}
}

void checkBoundingBox()
{
	for (int i = 0; i < boidVector.size(); i++)
	{
		Vec3f pos = boidVector.at(i).position;
		//check the x
		if (pos.x() > BOUNDING_BOX)
			boidVector.at(i).position.x(pos.x() * -1);
		if(pos.x() < -BOUNDING_BOX)
			boidVector.at(i).position.x(pos.x() * -1);
		//cehck y
		if (pos.y() > BOUNDING_BOX)
			boidVector.at(i).position.y(pos.y() * -1);
		if (pos.y() < -BOUNDING_BOX)
			boidVector.at(i).position.y(pos.y() * -1);
		//check z
		if (pos.z() > BOUNDING_BOX)
			boidVector.at(i).position.z(pos.z() * -1);
		if (pos.z() < -BOUNDING_BOX)
			boidVector.at(i).position.z(pos.z() * -1);
			
	}
}

void updateCoordinateFrame()
{
	for (int i = 0; i < boidVector.size(); i++)
	{
		Vec3f tangent = boidVector.at(i).velocity.normalized();
		Vec3f normal(0, 1, 0);
		Vec3f binormal = tangent.crossProduct(normal) * 0.5;

		Vec3f gravity(0, 1, 0);
		Vec3f aPrime = boidVector.at(i).accumulator + gravity;

		Vec3f aVel = boidVector.at(i).velocity * ((aPrime.dotProduct(boidVector.at(i).velocity)) / (boidVector.at(i).velocity.dotProduct(boidVector.at(i).velocity)));

		Vec3f aEl = aPrime - aVel;

		boidVector.at(i).head = boidVector.at(i).position + tangent;

		boidVector.at(i).up = aEl.normalized();
		boidVector.at(i).up = boidVector.at(i).position + boidVector.at(i).up;

		boidVector.at(i).right = boidVector.at(i).position + (tangent.normalized().crossProduct(boidVector.at(i).up.normalized()));

		boidVector.at(i).left = boidVector.at(i).position - (tangent.normalized().crossProduct(boidVector.at(i).up.normalized()));
	}
}

void clearAccumulator()
{
	for (int i = 0; i < boidVector.size(); i++)
		boidVector.at(i).accumulator = Vec3f(0, 0, 0);
}


void displayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use our shader
	glUseProgram(basicProgramID);

	// Use VAO that holds buffer bindings
	// and attribute config of buffers
	glBindVertexArray(vaoID);
	// Draw Quads, start at vertex 0, draw 4 of them (for a quad)
	glPointSize(6);
	glDrawArrays(GL_TRIANGLES, 0, verts.size());

	glUseProgram(sphereProgramID);

	// Use VAO that holds buffer bindings
	// and attribute config of buffers
	glBindVertexArray(sphereID);
	// Draw Quads, start at vertex 0, draw 4 of them (for a quad)
	glPointSize(6);
	glDrawArrays(GL_LINE_STRIP, 0, sphereVerts.size());

	glutSwapBuffers();
}

void idleFunc()
{
	// every frame refresh, rotate quad around y axis by 1 degree
	//	MVP = MVP * RotateAboutYMatrix( 1.0 );
	//M = M * RotateAboutYMatrix(1.0);


	//reload vertex

	accumulateVelocity();
	checkCollision();
	updateVelocity();
	updatePosition();
	checkBoundingBox();
	updateCoordinateFrame();
	clearAccumulator();

	reloadVertexBuffer();
	

	setupModelViewProjectionTransform();

	
	// send changes to GPU
	reloadMVPUniform();
	reloadSphereMVPUniform();

	glutPostRedisplay();
}

void resizeFunc(int width, int height)
{
	WIN_WIDTH = width;
	WIN_HEIGHT = height;

	glViewport(0, 0, width, height);

	loadProjectionMatrix();
	reloadMVPUniform();
	reloadSphereMVPUniform();

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

void generateSphereIDs()
{
	std::string vsSource = loadShaderStringfromFile("./basic_vs.glsl");
	std::string fsSource = loadShaderStringfromFile("./basic_fs.glsl");
	sphereProgramID = CreateShaderProgram(vsSource, fsSource);

	// load IDs given from OpenGL
	glGenVertexArrays(1, &sphereID);
	glGenBuffers(1, &sphereVertBufferID);
	glGenBuffers(1, &sphereColorBufferID);
}

void deleteIDs()
{
	glDeleteProgram(basicProgramID);
	glDeleteVertexArrays(1, &vaoID);
	glDeleteBuffers(1, &vertBufferID);
	glDeleteBuffers(1, &colorBufferID);

	glDeleteProgram(sphereProgramID);
	glDeleteVertexArrays(1, &sphereID);
	glDeleteBuffers(1, &sphereVertBufferID);
	glDeleteBuffers(1, &sphereColorBufferID);
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
	M = TranslateMatrix(0, 0, -10.0) * M;	// translate away from (0,0,0)

											// view doesn't change, but if it did you would use this
	V = IdentityMatrix();
}

void setupModelViewProjectionTransform()
{
	MVP = P * V * M; // transforms vertices from right to left (odd huh?)
	sphereMVP = P * V * M;
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

void reloadSphereMVPUniform()
{
	GLint mvpID = glGetUniformLocation(sphereProgramID, "MVP");

	glUseProgram(sphereProgramID);
	glUniformMatrix4fv(mvpID,
		1,
		GL_TRUE,
		sphereMVP.data()
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

void setupSphereVAO()
{
	glBindVertexArray(sphereID);

	glEnableVertexAttribArray(0); // match layout # in shader
	glBindBuffer(GL_ARRAY_BUFFER, sphereVertBufferID);
	glVertexAttribPointer(
		0,		// attribute layout # above
		3,		// # of components (ie XYZ )
		GL_FLOAT,	// type of components
		GL_FALSE,	// need to be normalized?
		0,		// stride
		(void*)0	// array buffer offset
		);

	glEnableVertexAttribArray(1); // match layout # in shader
	glBindBuffer(GL_ARRAY_BUFFER, sphereColorBufferID);
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

float randomize(float low, float high)
{
	return (low + (rand()) / ((RAND_MAX / (high - low))));
}


//create boids
void loadBuffer()
{

	string numBoids("./numboids.txt");

	loadV3fFromFile(verts, numBoids, 0);

	//create boids
	for (int i = 0; i < num_of_boids; i++)
	{
		float x = randomize(-20, 20);
		float y = randomize(-20, 20);
		float z = randomize(0, -20);


		Boid boid;
		boid.position = Vec3f(x, y, z);
		boidVector.push_back(boid);
	}

	//give boids random velocities
	for (int i = 0; i < num_of_boids; i++)
	{
		float x = randomize(-5, 5);
		float y = randomize(-5, 5);
		float z = randomize(-5, 5);

		velocities.push_back(Vec3f(x, y, z));
	}

	//take vertices from boids and push them into own vector
	for (int i = 0; i < boidVector.size(); i++)
	{

		boidVector.at(i).velocity = velocities.at(i);
		boidVector.at(i).accumulator = Vec3f(0, 0, 0);
	}

	updateCoordinateFrame();

	for (int i = 0; i < boidVector.size(); i++)
	{
		verts.push_back(boidVector.at(i).head);
		verts.push_back(boidVector.at(i).right);
		verts.push_back(boidVector.at(i).left);


		verts.push_back(boidVector.at(i).head);
		verts.push_back(boidVector.at(i).left);
		verts.push_back(boidVector.at(i).up);
		

		verts.push_back(boidVector.at(i).head);
		verts.push_back(boidVector.at(i).right);
		verts.push_back(boidVector.at(i).up);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(Vec3f) * verts.size(),	// byte size of Vec3f, 4 of them
		verts.data(),		// pointer (Vec3f*) to contents of verts
		GL_STATIC_DRAW);	// Usage pattern of GPU buffer

							// RGB values for the 4 vertices of the quad
	vector <float> colors;

	for (int i = 0; i < verts.size(); i+=9)
	{
		//head
		colors.push_back(1.0f);
		colors.push_back(1.0f);
		colors.push_back(1.0f);
		
		//right
		colors.push_back(0.0f);
		colors.push_back(1.0f);
		colors.push_back(0.0f);

		//left
		colors.push_back(0.0f);
		colors.push_back(0.0f);
		colors.push_back(1.0f);


		///////////////////
		//head
		colors.push_back(1.0f);
		colors.push_back(1.0f);
		colors.push_back(1.0f);
		//left
		colors.push_back(0.0f);
		colors.push_back(0.0f);
		colors.push_back(1.0f);
		//up
		colors.push_back(1.0f);
		colors.push_back(0.0f);
		colors.push_back(0.0f);


		/////////////////////////////
		//head
		colors.push_back(1.0f);
		colors.push_back(1.0f);
		colors.push_back(1.0f);
		//right
		colors.push_back(0.0f);
		colors.push_back(1.0f);
		colors.push_back(0.0f);
		//up
		colors.push_back(1.0f);
		colors.push_back(0.0f);
		colors.push_back(0.0f);
	}

	glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(float)*colors.size(),
		colors.data(),
		GL_STATIC_DRAW);
}

void loadSphereBuffer()
{

	string velocityFile("./sphere_position.txt");

	loadV3fFromFile(verts, velocityFile, 2);

	create(sphereRadius, 20, spherePosition.x(), spherePosition.y(), spherePosition.z());


	glBindBuffer(GL_ARRAY_BUFFER, sphereVertBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(Vec3f) * sphereVerts.size(),	// byte size of Vec3f, 4 of them
		sphereVerts.data(),		// pointer (Vec3f*) to contents of verts
		GL_STATIC_DRAW);	// Usage pattern of GPU buffer

							// RGB values for the 4 vertices of the quad
	vector <float> colors;

	for (int i = 0; i < sphereVerts.size(); i++)
	{
		colors.push_back(0.0f);
		colors.push_back(1.0f);
		colors.push_back(1.0f);
	}

	glBindBuffer(GL_ARRAY_BUFFER, sphereColorBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(float)*colors.size(),
		colors.data(),
		GL_STATIC_DRAW);
}


void reloadVertexBuffer()
{
	verts.clear();
	for (int i = 0; i < boidVector.size(); i++)
	{
		verts.push_back(boidVector.at(i).head);
		verts.push_back(boidVector.at(i).right);
		verts.push_back(boidVector.at(i).left);


		verts.push_back(boidVector.at(i).head);
		verts.push_back(boidVector.at(i).up);
		verts.push_back(boidVector.at(i).left);

		verts.push_back(boidVector.at(i).head);
		verts.push_back(boidVector.at(i).up);
		verts.push_back(boidVector.at(i).right);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(Vec3f) * verts.size(),	// byte size of Vec3f, 4 of them
		verts.data(),		// pointer (Vec3f*) to contents of verts
		GL_STATIC_DRAW);	// Usage pattern of GPU buffer
}

void init()
{
	glEnable(GL_DEPTH_TEST);

	// SETUP SHADERS, BUFFERS, VAOs

	generateIDs();
	generateSphereIDs();
	setupVAO();
	setupSphereVAO();
	loadBuffer();
	loadSphereBuffer();

	loadModelViewMatrix();
	loadProjectionMatrix();
	setupModelViewProjectionTransform();
	reloadMVPUniform();
	reloadSphereMVPUniform();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	// Setup FB configuration
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutInitWindowPosition(0, 0);

	glutCreateWindow("CPSC 587 Assignment 4 Boids");

	glewExperimental = true; // Needed in Core Profile
							 // Comment out if you want to us glBeign etc...
	if (glewInit() != GLEW_OK)
	{
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	cout << "GL Version: :" << glGetString(GL_VERSION) << endl;

	glutDisplayFunc(displayFunc);
	glutReshapeFunc(resizeFunc);
	glutIdleFunc(idleFunc);

	init(); // our own initialize stuff func

	glutMainLoop();

	// clean up after loop
	deleteIDs();

	return 0;
}
