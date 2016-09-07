/*
Gorman Law
10053193
CPSC 587 Assignment 1 RollerCoaster
October 8, 2015
TA: Andrew Owens
Done in Visual Studio Community 2015 with GLUT 3.7.6 and GLEW 3.1
Tested in labs.
*/
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

//track stuff
GLuint vaoID;
GLuint basicProgramID;
GLuint vertBufferID;
GLuint colorBufferID;

//car stuff
GLuint vaoCarID;
GLuint carProgramID;
GLuint carVertBufferID;
GLuint carColorBufferID;

//rail stuff
GLuint vaoRailID;
GLuint railProgramID;
GLuint railVertBufferID;
GLuint railColorBufferID;

//floor stuff
GLuint vaoFloorID;
GLuint floorProgramID;
GLuint floorVertBufferID;
GLuint floorColorBufferID;

vector< Vec3f > verts;	//vertices for track
vector< Vec3f > sub_verts;	//subdivided vertices XD
vector< Vec3f > car_verts; //vertices for cart
vector< Vec3f > rail_verts;
vector< Vec3f > floor_verts;


Vec3f startVert;	//track start vertex
Vec3f endVert;		//track end vertex
Vec3f currentPos;
int trackCounter = 0;
int railCounter = 0;

//values for calculating speed
float speed = 0.5f;		//speed of cart
float t = 0.01f;	//time between animations
float GRAVITY = 9.81f;
float maximum_height = -1000.0f;
float minimum_height = 1000.0f;

// Could store these two in an array GLuint[]
//GLuint vertBufferID;
//GLuint colorBufferID;

Mat4f MVP;
Mat4f M;
Mat4f V;
Mat4f P;

Mat4f carM;
Mat4f carMVP;

Mat4f railMVP;
Mat4f floorMVP;
Mat4f floorM;

Mat4f rail_matrix = IdentityMatrix();

int WIN_WIDTH = 800, WIN_HEIGHT = 600;

// function declarations... just to keep things kinda organized.
void subdivide();
Vec3f findMidPoint(Vec3f firstPoint, Vec3f secondPoint);
void callSubdivide(int depth);

void displayFunc();
void resizeFunc();
void idleFunc();
void init();

void generateIDs();
void generateCarID();
void generateRailID();
void deleteIDs();

void setupVAO();
void setupCarVAO();
void setupRailVAO();

void loadBuffer();
void loadCarBuffer();
void loadRailBuffer();

void loadProjectionMatrix();
void loadModelViewMatrix();
void setupModelViewProjectionTransform();

void reloadMVPUniform();
void reloadCarMVPUniform();
void reloadRailMVPUniform();
void reloadAll();

void setCarPos(Vec3f new_pos);
void replaceMatrixColumn();

Vec3f getNextCarPosition();

float getSpeed();
int speed_state = 0;
void speedToggle();

int main( int, char** );
// function declarations

//subdivide points
void subdivide()
{
	sub_verts.clear();
	sub_verts.push_back(verts.front());	//saving first point

	for (int i = 0; i < verts.size()-1; i++)
	{
		//find first middles
		Vec3f firstVert = verts.at(i);
		Vec3f secondVert = verts.at(i + 1);
		Vec3f middleVert = findMidPoint(firstVert, secondVert);

		//find middle again and save points
		Vec3f firstQuarter = findMidPoint(firstVert, middleVert);
		Vec3f secondQuarter = findMidPoint(middleVert, secondVert);

		//push into temp directory
		sub_verts.push_back(firstQuarter);
		sub_verts.push_back(secondQuarter);
	}

	sub_verts.push_back(verts.back());	//saving last point

	verts = sub_verts;	//setting verts equal to subdivided verts for re-subdividing if necessary
}

//function that finds the midpoint between two points
Vec3f findMidPoint(Vec3f firstPoint, Vec3f secondPoint)
{
	Vec3f middlePoint = (firstPoint + secondPoint) * 0.5;	//pi + pi+1 /2

	return middlePoint; //return
}

//call subdivide function multiple times
void callSubdivide(int depth)
{
	for (int i = 0; i < depth; i++)
	{
		subdivide();
	}
}

Vec3f getNextCarPosition()
{
	float delta_d = speed * t;	//d = s t
	float dA = 0.0f;

	Vec3f pos, next_pos;


	//from psuedocode given in tutorial
	pos = currentPos;
	next_pos = verts.at(trackCounter % verts.size());

	while ((dA + pos.distance(next_pos)) < delta_d)
	{
		dA += pos.distance(next_pos);
		pos = verts.at(trackCounter % verts.size());
		next_pos = verts.at((trackCounter + 1) % verts.size());
		trackCounter++;
	}

	Vec3f car_pos = (next_pos - pos) * (1 / (next_pos - pos).length() * (delta_d - dA));
	
	//float test = (next_pos - pos).length();
	//float dis = (delta_d - dA);
	//dis = dis * test;
	
	//Vec3f car_pos = (next_pos - pos);
	//car_pos = car_pos * (1/dis);

	car_pos = currentPos + car_pos;

	return car_pos;
}

//function that changes speed states between lift (0), physics(1), and slowing down(2)
void speedToggle()
{
	if (currentPos.x() >= -2.0f && speed_state == 0)
	{
		speed_state = 1;
	}

	if (currentPos.x() <= -1.2f && currentPos.z() == 3.0f && speed_state == 1)
	{
		speed_state = 2;
	}
}

float speed_decay = 1;
//function to find speed based on what state it is
float getSpeed()
{
	speedToggle();
	float minimum_speed = 0.01f;

	if (speed_state == 0)
	{
		cout << "Lifting phase: ";
		return 1;	//lifting speed
	}

	else if (speed_state == 1)
	{
		//sqrt(2 * g * (H-h))
		// physics speed
		cout << "Physics phase: ";
		float next_speed = sqrt(2 * GRAVITY * (maximum_height - currentPos.y())) + minimum_speed;

		return next_speed;
	}

	else if (speed_state == 2)	//slowing speed
	{
		cout << "Braking phase: ";
		if (speed_decay > 40)	//car stopped
		{
			return 0.0f;
		}

		float next_speed = (sqrt(2 * GRAVITY * (maximum_height - currentPos.y())) + minimum_speed);
		float brakes = next_speed / speed_decay;

		speed_decay += 0.1f;

		return brakes;
	}

	else
	{
		cout << "Something is broken!!!" << endl;	//uhoh!
		exit(0);
	}

}

//function just to replace the columns in matrices
void replaceMatrixColumn(Mat4f & matrix, int column, Vec3f vector)
{
	matrix(0, column) = vector.x();
	matrix(1, column) = vector.y();
	matrix(2, column) = vector.z();
}

//calculate the TNB matrix for carts
void calculateTNBMatrixCart()
{
		Vec3f point(verts.at((trackCounter) % verts.size()));
		Vec3f point_next(verts.at((trackCounter + 1) % verts.size()));
		Vec3f point_prev(verts.at((trackCounter - 1) % verts.size()));

		//determine the smallest distance between points, then set all distances to the same 
		float distance_between_point_and_prev = (point_prev - point).length();
		float distance_between_next_and_point = (point_next - point).length();

		float min_length = min(distance_between_next_and_point, distance_between_point_and_prev);

		Vec3f fixed_point_next = point + ((point_next - point).normalized() * min_length);
		Vec3f fixed_point_prev = point + ((point - point_prev).normalized() * min_length);

		//find a perpendicular
		//a = v^2 / r
		//v = v^2
		//r = (c^2+4h^2)/8h
		//h = length ( 1/2 (next point - 2* point + prev point) )
		//c = length (next point - prev point

		float c = (fixed_point_next - fixed_point_prev).length();

		float h = ((fixed_point_next - (point * 2.0f) + fixed_point_prev) * 0.5).length();

		float r = ((c * c) + (4 * h * h)) / (8 * h);

		float a = (speed*speed) / r;

		//gravity always points down in Y
		Vec3f gravity(0, 1, 0);

		//N = (point_prev - point + point_next - point)/Length(point_prev - 2 * point + point_next)
		//N' = aN + gravity / aN + gravity .length
		Vec3f normalVec = (fixed_point_next - (point * 2.0f) + fixed_point_prev).normalized();
		Vec3f normalVecPrime = ((normalVec * a) + gravity).normalized();

		//T = fixed_point_next - point / fixedPointNext - point
		Vec3f tangentVec = (fixed_point_next - point).normalized();

		//B = T x N'
		Vec3f binormalVec = tangentVec.crossProduct(normalVecPrime);
		binormalVec = binormalVec.normalized();

		//T' = N' x B
		Vec3f tangentVecPrime = normalVecPrime.crossProduct(binormalVec);
		tangentVecPrime = tangentVecPrime.normalized();

		//replace with T' N' B translation
		replaceMatrixColumn(carM, 0, tangentVecPrime);
		replaceMatrixColumn(carM, 1, normalVecPrime);
		replaceMatrixColumn(carM, 2, binormalVec);

}

//replace translation vectors in carM. set new current position vector
void setCarPos(Vec3f new_pos)
{
	Vec3f scaled_pos = new_pos;
	scaled_pos.x(scaled_pos.x()*0.25);
	scaled_pos.y(scaled_pos.y()*0.25);
	scaled_pos.z(scaled_pos.z()*0.25);

	replaceMatrixColumn(carM, 3, scaled_pos);
	currentPos = new_pos;
}

//calculate TNB matrix for rails and such
void calculateTNBMatrixRail()
{
	Vec3f point(verts.at((railCounter) % verts.size()));
	Vec3f point_next(verts.at((railCounter + 1) % verts.size()));
	Vec3f point_prev(verts.at((railCounter - 1) % verts.size()));

	float distance_between_point_and_prev = (point_prev - point).length();
	float distance_between_next_and_point = (point_next - point).length();

	float min_length = min(distance_between_next_and_point, distance_between_point_and_prev);

	Vec3f fixed_point_next = point + (point_next - point).normalized() * min_length;
	Vec3f fixed_point_prev = point - (point_prev - point).normalized() * min_length;

	//find a perpendicular
	//a = v^2 / r
	//v = v^2
	//r = (c^2+4h^2)/8h
	//h = length ( 1/2 (next point - 2* point + prev point) )
	//c = length (next point - prev point

	float c = (fixed_point_next - fixed_point_prev).length();

	float h = ((fixed_point_next - (point * 2.0f) + fixed_point_prev) * 0.5).length();

	float r = (pow(c, 2) + 4 * (pow(h, 2))) / (8 * h);

	float a = pow(speed, 2) / r;

	//gravity always points down in Y
	Vec3f gravity(0, -1, 0);

	//N = (point_prev - point + point_next - point)/Length(point_prev - 2 * point + point_next)
	//N' = aN + gravity / aN + gravity .length
	Vec3f normalVec = (fixed_point_next - (point * 2.0f) + fixed_point_prev).normalized();
	Vec3f normalVecPrime = ((normalVec * a) + gravity).normalized();

	Vec3f tangentVec = (fixed_point_next - point).normalized();

	//B = T x N'
	Vec3f binormalVec = tangentVec.crossProduct(normalVecPrime);
	binormalVec = binormalVec.normalized();

	//T' = N' x B
	Vec3f tangentVecPrime = normalVecPrime.crossProduct(binormalVec);
	tangentVecPrime = tangentVecPrime.normalized();

	//replace with T' N' B translation

	replaceMatrixColumn(rail_matrix, 0, tangentVecPrime);
	replaceMatrixColumn(rail_matrix, 1, normalVecPrime);
	replaceMatrixColumn(rail_matrix, 2, binormalVec);
}

void setRailsAndSupports()
{
	vector <Vec3f> inside_rail;
	vector <Vec3f> outside_rail;

	for (int i = 0; i < verts.size(); i++)
	{
		//calculate_TNB_matrix(rail_matrix);

		calculateTNBMatrixRail();
		railCounter++;
		//rail_matrix(0, 2) = 0;
		//rail_matrix(1, 2) = 0.5;
		//rail_matrix(2, 2) = 0.5;

		Vec3f bin(rail_matrix(0, 2), rail_matrix(1, 2), rail_matrix(2, 2));	//grab binormal from rail tnb matrix
		
		//cout << "rails " << rail_matrix.at(2,2) << endl;

		Vec3f in_vert = verts.at(i) - (bin * 0.1);	//multiply by 0.2 otherwise the rails will be too big!
		Vec3f out_vert = verts.at(i) + (bin * 0.1);
		//cout << "binormal " << binormal << endl;

		rail_verts.push_back(in_vert);	//push back rails
		rail_verts.push_back(out_vert);
		
		rail_verts.push_back(verts.at(i));	//push back supports
		rail_verts.push_back(Vec3f(verts.at(i).x(), 0, verts.at(i).z()));
	}

}


//Taken from Andrew Owens tutorials
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

		float x, y, z;
		if ((ss >> x >> y >> z) && (!ss || !ss.eof() || ss.good()))
		{
			throw std::runtime_error("Error read file: "
				+ line
				+ " (line: "
				+ std::to_string(lineNum)
				+ ")");
		}
		else
		{
			if (fileType == 0)	//if file is for track, push onto verts
			{
				verts.push_back(Vec3f(x, y, z));
			}
			else if (fileType == 1)	//if file is for car, push to car
			{
				car_verts.push_back(Vec3f(x, y, z));
			}
		}
	}
	file.close();
}

void displayFunc()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Use our shader
	glUseProgram( basicProgramID );
	// Use VAO that holds buffer bindings
	// and attribute config of buffers
	glBindVertexArray( vaoID );
	GLfloat width = 1;
	glLineWidth(width);
	// Draw Quads, start at vertex 0, draw 4 of them (for a quad)
	glDrawArrays( GL_LINE_STRIP, 0, verts.size() );

	//display car
	glUseProgram(carProgramID);
	glBindVertexArray(vaoCarID);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, car_verts.size());

	//display rails
	glUseProgram(railProgramID);
	glBindVertexArray(vaoRailID);
	glDrawArrays(GL_LINES, 0, rail_verts.size());

	//display sweet green grass
	glUseProgram(floorProgramID);
	glBindVertexArray(vaoFloorID);
	glDrawArrays(GL_TRIANGLES, 0, floor_verts.size());

	glutSwapBuffers();
}

//function that moves the cart
void idleFunc()
{
	speed = getSpeed();
	cout << "Speed: " << speed << endl;
	setCarPos(getNextCarPosition());
	calculateTNBMatrixCart();

	carM = TranslateMatrix(0, 0, -1) * carM; //Gotta have this
	carM = TranslateMatrix(0, 0.02, 0)*carM;	//move car above track

	setupModelViewProjectionTransform();
	reloadAll();
	glutPostRedisplay();
}

void resizeFunc( int width, int height )
{
    WIN_WIDTH = width;
    WIN_HEIGHT = height;

    glViewport( 0, 0, width, height );

    loadProjectionMatrix();
    reloadMVPUniform();

    glutPostRedisplay();
}

void generateIDs()
{
	std::string vsSource = loadShaderStringfromFile( "./basic_vs.glsl" );
	std::string fsSource = loadShaderStringfromFile( "./basic_fs.glsl" );
	basicProgramID = CreateShaderProgram( vsSource, fsSource );

	// load IDs given from OpenGL
	glGenVertexArrays( 1, &vaoID );
	glGenBuffers( 1, &vertBufferID );
	glGenBuffers( 1, &colorBufferID );

}

void generateCarID()
{
	std::string vsSource = loadShaderStringfromFile("./basic_vs.glsl");
	std::string fsSource = loadShaderStringfromFile("./basic_fs.glsl");
	carProgramID = CreateShaderProgram(vsSource, fsSource);

	glGenVertexArrays(1, &vaoCarID);
	glGenBuffers(1, &carVertBufferID);
	glGenBuffers(1, &carColorBufferID);

}

void generateRailID()
{
	std::string vsSource = loadShaderStringfromFile("./basic_vs.glsl");
	std::string fsSource = loadShaderStringfromFile("./basic_fs.glsl");
	railProgramID = CreateShaderProgram(vsSource, fsSource);

	glGenVertexArrays(1, &vaoRailID);
	glGenBuffers(1, &railVertBufferID);
	glGenBuffers(1, &railColorBufferID);
}

void generateFloorID()
{
	std::string vsSource = loadShaderStringfromFile("./basic_vs.glsl");
	std::string fsSource = loadShaderStringfromFile("./basic_fs.glsl");
	floorProgramID = CreateShaderProgram(vsSource, fsSource);

	glGenVertexArrays(1, &vaoFloorID);
	glGenBuffers(1, &floorVertBufferID);
	glGenBuffers(1, &floorColorBufferID);
}

void deleteIDs()
{
	//clear track stuff
	glDeleteProgram( basicProgramID );
	glDeleteVertexArrays( 1, &vaoID );
	glDeleteBuffers( 1, &vertBufferID );
	glDeleteBuffers( 1, &colorBufferID );

	//clear cart stuff
	glDeleteProgram(carProgramID);
	glDeleteVertexArrays(1, &vaoCarID);
	glDeleteBuffers(1, &carVertBufferID);
	glDeleteBuffers(1, &carColorBufferID);

	//clear rail stuff
	glDeleteProgram(railProgramID);
	glDeleteVertexArrays(1, &vaoRailID);
	glDeleteBuffers(1, &railVertBufferID);
	glDeleteBuffers(1, &railColorBufferID);

	//clear floor stuff
	glDeleteProgram(floorProgramID);
	glDeleteVertexArrays(1, &vaoFloorID);
	glDeleteBuffers(1, &floorVertBufferID);
	glDeleteBuffers(1, &floorColorBufferID);

}

void loadProjectionMatrix()
{
    // Perspective Only
    
	// field of view angle 60 degrees
	// window aspect ratio
	// near Z plane > 0
	// far Z plane

    P = PerspectiveProjection(  60, // FOV
                                static_cast<float>(WIN_WIDTH)/WIN_HEIGHT, // Aspect
                                0.01,   // near plane
                                500 ); // far plane depth
}

void loadModelViewMatrix()
{
    M = UniformScaleMatrix( 0.25 );	// scale Quad First
    M = TranslateMatrix( 0, 0, -1.0 ) * M;	// translate away from (0,0,0)

	carM = IdentityMatrix();

	setCarPos(startVert);

	//view model
    // view doesn't change, but if it did you would use this
    V = IdentityMatrix();
	V = TranslateMatrix(0, -0.4, -0.7);
}

void setupModelViewProjectionTransform()
{
	MVP = P * V * M; // transforms vertices from right to left (odd huh?)
	carMVP = P * V * carM;
	railMVP = P * V * M;
	floorMVP = P * V * M;
}
 
void reloadMVPUniform()
{
	GLint mvpID = glGetUniformLocation( basicProgramID, "MVP" );
	
	glUseProgram( basicProgramID );
	glUniformMatrix4fv( 	mvpID,		// ID
				1,		// only 1 matrix
				GL_TRUE,	// transpose matrix, Mat4f is row major
				MVP.data()	// pointer to data in Mat4f
			);
}

void reloadCarMVPUniform()
{
	GLint mvpID = glGetUniformLocation(carProgramID, "MVP");

	glUseProgram(carProgramID);
	glUniformMatrix4fv(mvpID,
		1,
		GL_TRUE,
		carMVP.data()
		);
}

void reloadRailMVPUniform()
{
	GLint mvpID = glGetUniformLocation(railProgramID, "MVP");

	glUseProgram(railProgramID);
	glUniformMatrix4fv(mvpID,
		1,
		GL_TRUE,
		railMVP.data()
		);
}

void reloadFloorMVPUniform()
{
	GLint mvpID = glGetUniformLocation(floorProgramID, "MVP");

	glUseProgram(floorProgramID);
	glUniformMatrix4fv(mvpID,
		1,
		GL_TRUE,
		floorMVP.data()
		);
}


void reloadAll()
{
	reloadCarMVPUniform();
	reloadMVPUniform();
	reloadRailMVPUniform();
	reloadFloorMVPUniform();
}

void setupVAO()
{
	glBindVertexArray( vaoID );

	glEnableVertexAttribArray( 0 ); // match layout # in shader
	glBindBuffer( GL_ARRAY_BUFFER, vertBufferID );
	glVertexAttribPointer(
		0,		// attribute layout # above
		3,		// # of components (ie XYZ )
		GL_FLOAT,	// type of components
		GL_FALSE,	// need to be normalized?
		0,		// stride
		(void*)0	// array buffer offset
	);

	glEnableVertexAttribArray( 1 ); // match layout # in shader
	glBindBuffer( GL_ARRAY_BUFFER, colorBufferID );
	glVertexAttribPointer(
		1,		// attribute layout # above
		3,		// # of components (ie XYZ )
		GL_FLOAT,	// type of components
		GL_FALSE,	// need to be normalized?
		0,		// stride
		(void*)0	// array buffer offset
	);

	glBindVertexArray( 0 ); // reset to default		
}

void setupCarVAO()
{
	glBindVertexArray(vaoCarID);

	glEnableVertexAttribArray(0); // match layout # in shader
	glBindBuffer(GL_ARRAY_BUFFER, carVertBufferID);
	glVertexAttribPointer(
		0,		// attribute layout # above
		3,		// # of components (ie XYZ )
		GL_FLOAT,	// type of components
		GL_FALSE,	// need to be normalized?
		0,		// stride
		(void*)0	// array buffer offset
		);

	glEnableVertexAttribArray(1); // match layout # in shader
	glBindBuffer(GL_ARRAY_BUFFER, carColorBufferID);
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

void setupRailVAO()
{
	glBindVertexArray(vaoRailID);

	glEnableVertexAttribArray(0); // match layout # in shader
	glBindBuffer(GL_ARRAY_BUFFER, railVertBufferID);
	glVertexAttribPointer(
		0,		// attribute layout # above
		3,		// # of components (ie XYZ )
		GL_FLOAT,	// type of components
		GL_FALSE,	// need to be normalized?
		0,		// stride
		(void*)0	// array buffer offset
		);

	glEnableVertexAttribArray(1); // match layout # in shader
	glBindBuffer(GL_ARRAY_BUFFER, railColorBufferID);
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

void setupFloorVAO()
{
	glBindVertexArray(vaoFloorID);

	glEnableVertexAttribArray(0); // match layout # in shader
	glBindBuffer(GL_ARRAY_BUFFER, floorVertBufferID);
	glVertexAttribPointer(
		0,		// attribute layout # above
		3,		// # of components (ie XYZ )
		GL_FLOAT,	// type of components
		GL_FALSE,	// need to be normalized?
		0,		// stride
		(void*)0	// array buffer offset
		);

	glEnableVertexAttribArray(1); // match layout # in shader
	glBindBuffer(GL_ARRAY_BUFFER, floorColorBufferID);
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

void loadBuffer()
{
	string trackFile("./Testtrack.txt");	//load from file

	loadV3fFromFile(verts, trackFile, 0);

	callSubdivide(3);	//subdivide track 3 times!!

	startVert = verts.at(0);
	currentPos = verts.at(0);

	for (int i = 0; i < verts.size(); i++)
	{
		if (verts.at(i).y() > maximum_height)
		{
			maximum_height = verts.at(i).y();
		}
	}

	
	glBindBuffer( GL_ARRAY_BUFFER, vertBufferID );
	glBufferData(	GL_ARRAY_BUFFER,	
			sizeof(Vec3f)*verts.size(),	// byte size of Vec3f, 4 of them
			verts.data(),		// pointer (Vec3f*) to contents of verts
			GL_STATIC_DRAW );	// Usage pattern of GPU buffer


	vector<float> colors;
	//track is white
	for (int i = 0; i < verts.size(); i++)
	{
		//white
		colors.push_back(1.0f);
		colors.push_back(1.0f);
		colors.push_back(1.0f);
	
	}

	glBindBuffer( GL_ARRAY_BUFFER, colorBufferID );
	glBufferData(	GL_ARRAY_BUFFER,
			sizeof(float)*colors.size(),
			colors.data(),
			GL_STATIC_DRAW );
}

void loadCarBuffer()
{
		string carFile("./Car.txt");	//read from file

	loadV3fFromFile(car_verts, carFile, 1);

	glBindBuffer(GL_ARRAY_BUFFER, carVertBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(Vec3f)*car_verts.size(),	// byte size of Vec3f, 4 of them
		car_verts.data(),		// pointer (Vec3f*) to contents of verts
		GL_STATIC_DRAW);	// Usage pattern of GPU buffer


	vector<float> carColors;

	for (int i = 0; i < car_verts.size(); i++)
	{
		//yellow
		carColors.push_back(1.0f);
		carColors.push_back(1.0f);
		carColors.push_back(0.0f);
	}

	glBindBuffer(GL_ARRAY_BUFFER, carColorBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(float)*carColors.size(),
		carColors.data(),
		GL_STATIC_DRAW);

}

void loadRailBuffer()
{
	setRailsAndSupports();

	glBindBuffer(GL_ARRAY_BUFFER, railVertBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(Vec3f)*rail_verts.size(),	// byte size of Vec3f, 4 of them
		rail_verts.data(),		// pointer (Vec3f*) to contents of verts
		GL_STATIC_DRAW);	// Usage pattern of GPU buffer


	vector<float> railColors;

	for (int i = 0; i < rail_verts.size(); i++)
	{
		//red
		railColors.push_back(1.0f);
		railColors.push_back(0.0f);
		railColors.push_back(0.0f);
	}

	glBindBuffer(GL_ARRAY_BUFFER, railColorBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(float)*railColors.size(),
		railColors.data(),
		GL_STATIC_DRAW);
}

void loadFloorBuffer()
{
	floor_verts.push_back(Vec3f(3, 0, -3));
	floor_verts.push_back(Vec3f(3, 0, 3));
	floor_verts.push_back(Vec3f(-3, 0, 3));

	floor_verts.push_back(Vec3f(-3, 0, 3));
	floor_verts.push_back(Vec3f(-3, 0, -3));
	floor_verts.push_back(Vec3f(3, 0, -3));

	floor_verts.push_back(Vec3f(250, 250, -6));
	floor_verts.push_back(Vec3f(250, -250, -6));
	floor_verts.push_back(Vec3f(-250, -250, -6));

	floor_verts.push_back(Vec3f(-250, -250, -6));
	floor_verts.push_back(Vec3f(-250, 250, -6));
	floor_verts.push_back(Vec3f(250, 250, -6));

	glBindBuffer(GL_ARRAY_BUFFER, floorVertBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(Vec3f)*floor_verts.size(),	// byte size of Vec3f, 4 of them
		floor_verts.data(),		// pointer (Vec3f*) to contents of verts
		GL_STATIC_DRAW);	// Usage pattern of GPU buffer


	vector<float> floorColors;

	for (int i = 0; i < floor_verts.size()/2; i++)
	{
		//green
		floorColors.push_back(0.0039f);
		floorColors.push_back((0.65098f));
		floorColors.push_back(0.0667f);
	}

	for (int i = 0; i < floor_verts.size() / 2; i++)
	{
		//green
		floorColors.push_back(0.529f);
		floorColors.push_back(0.8078f);
		floorColors.push_back(0.98f);
	}

	glBindBuffer(GL_ARRAY_BUFFER, floorColorBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(float)*floorColors.size(),
		floorColors.data(),
		GL_STATIC_DRAW);
}

void init()
{


	glEnable( GL_DEPTH_TEST );

	// SETUP SHADERS, BUFFERS, VAOs

	generateIDs();
	setupVAO();
	loadBuffer();

	generateCarID();
	setupCarVAO();
	loadCarBuffer();

	generateRailID();
	setupRailVAO();
	loadRailBuffer();

	generateFloorID();
	setupFloorVAO();
	loadFloorBuffer();

    loadModelViewMatrix();
    loadProjectionMatrix();
	setupModelViewProjectionTransform();

	reloadMVPUniform();
	reloadCarMVPUniform();
	reloadRailMVPUniform();
	reloadFloorMVPUniform();
	reloadAll();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	// Setup FB configuration
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutInitWindowPosition(0, 0);

	glutCreateWindow("Gorman's Rollercoaster");

	glewExperimental = true; // Needed in Core Profile
	// Comment out if you want to us glBeign etc...
	if (glewInit() != GLEW_OK)
	{
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	cout << "GL Version: :" << glGetString(GL_VERSION) << endl;
	cout <<"What" << endl;

	glutDisplayFunc(displayFunc);
	glutReshapeFunc(resizeFunc);
	glutIdleFunc(idleFunc);

	init(); // our own initialize stuff func

	glutMainLoop();
	
	// clean up after loop
	deleteIDs();

	return 0;
}
