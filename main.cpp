/* Written by Jeffrey Chastine, 2012 */
/* Lazily adapted by Mark Elsinger, 2014 
	into engine for Helicopter++, a heavily centralized 
	and lowly objectified game. probaly undeserving of the
	++ suffix. */
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include "Mesh.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

GLuint shaderProgramID;
GLuint vao = 0;
GLuint vbo[2];
GLuint positionID, colorID;
GLuint * indexArray = NULL;
GLfloat projection[4][4];
GLfloat * projection_p = (GLfloat *)projection;
int ilen = 0;
float currentRoll = 0.0f;
float currentPitch = 0.0f;
float currentYaw = 0.0f;
//float dYaw = 0.0f;
//float dRoll= 0.0f;
//float dPitch = 0.0f;
float currentX = 0.0f;
float currentY = 0.0f;
float currentZ = 0.0f;
float currentS = 1.0f;
std::vector<Mesh> meshes;

#pragma region SHADER_FUNCTIONS
static char* readFile(const char* filename) {
	/*// Open the file
	FILE* fp = fopen (filename, "r");
	// Move the file pointer to the end of the file and determing the length
	fseek(fp, 0, SEEK_END);
	long file_length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* contents = new char[file_length+1];*/

	//custom
	std::ifstream in(filename);
	// Move the file pointer to the end of the file and determing the length
	in.seekg(0, std::ios::end);
	long file_length = (long)in.tellg();
	in.clear();
	in.seekg(0, std::ios::beg);
	char* contents = new char[file_length + 1];

	// zero out memory
	for (int i = 0; i < file_length + 1; i++) {
		contents[i] = 0;
	}
	// Here's the actual read
	//fread (contents, 1, file_length, fp);
	in.read(contents, file_length);
	// This is how you denote the end of a string in C
	contents[file_length + 1] = '\0';


	//if (!in.good()) { printf("bad file"); } //debug line
	//fclose(fp);
	in.close();
	return contents;
}

bool compiledStatus(GLint shaderID){
	GLint compiled = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compiled);
	if (compiled) {
		return true;
	}
	else {
		GLint logLength;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
		char* msgBuffer = new char[logLength];
		glGetShaderInfoLog(shaderID, logLength, NULL, msgBuffer);
		printf("%s\n", msgBuffer);
		delete (msgBuffer);
		return false;
	}
}

GLuint makeVertexShader(const char* shaderSource) {
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 1, (const GLchar**)&shaderSource, NULL);
	glCompileShader(vertexShaderID);
	bool compiledCorrectly = compiledStatus(vertexShaderID);
	if (compiledCorrectly) {
		return vertexShaderID;
	}
	return -1;
}

GLuint makeFragmentShader(const char* shaderSource) {
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, (const GLchar**)&shaderSource, NULL);
	glCompileShader(fragmentShaderID);
	bool compiledCorrectly = compiledStatus(fragmentShaderID);
	if (compiledCorrectly) {
		return fragmentShaderID;
	}
	return -1;
}

GLuint makeShaderProgram(GLuint vertexShaderID, GLuint fragmentShaderID) {
	GLuint shaderID = glCreateProgram();
	glAttachShader(shaderID, vertexShaderID);
	glAttachShader(shaderID, fragmentShaderID);
	glLinkProgram(shaderID);
	return shaderID;
}
#pragma endregion SHADER_FUNCTIONS

#pragma region HELPER_FUNCTIONS


//tris only for now
void parseFlatObjFile(char * filename, GLfloat ** vertices, int * vlength, GLuint ** indices, int * ilength)
{
	GLfloat * verts = *vertices; //this is necessary for some reason...
	GLuint * inds = *indices;    //just using the arguments didn't work...

	std::ifstream in(filename);
	if (!in.good())
	{

		printf("bad file: %s\n", filename);
		system("PAUSE");
		exit(-1);
	}
	const int MAX_LINE_LENGTH = 80;
	char s[MAX_LINE_LENGTH + 1];
	int vtex_count = 0;
	int indx_count = 0;
	do
	{
		in.getline(s, MAX_LINE_LENGTH);
		//printf("%s\n", s); //debug
		if (s[0] == 'v')
		{
			vtex_count = vtex_count + 3;
		}
		if (s[0] == 'f')
		{
			indx_count = indx_count + 3;
		}
	} while (!in.eof());

	in.clear();
	in.seekg(0, std::ios::beg);

	*vlength = vtex_count;
	*ilength = indx_count;
	//gonna assume 3 per index and vertex!
	//*vertices = new GLfloat[vtex_count];
	//*indices = new GLuint[indx_count];
	verts = new GLfloat[vtex_count];
	inds = new GLuint[indx_count];

	int iv = 0;
	int ii = 0;
	char * tmp = NULL;
	char * nxt_tmp = NULL;
	do
	{
		in.getline(s, MAX_LINE_LENGTH);
		//printf("%s\n", s);
		if (s[0] == 'v')
		{
			tmp = strtok_s(&s[1], " ", &nxt_tmp);
			for (int i = 0; i < 3 && iv < vtex_count; ++i)
			{
				verts[iv] = (GLfloat)atof(tmp);
				tmp = strtok_s(NULL, " ", &nxt_tmp);
				++iv;
			}
		}
		else if (s[0] == 'f')
		{
			tmp = strtok_s(&s[1], " ", &nxt_tmp);
			for (int i = 0; i < 3 && ii < indx_count; ++i)
			{
				inds[ii] = (GLuint)atoi(tmp);
				tmp = strtok_s(NULL, " ", &nxt_tmp);
				++ii;
			}
		}

	} while (iv <= vtex_count && ii <= indx_count && !in.eof());

	*vertices = verts;
	*indices = inds;

	//should be good
	printf("vcount: %d\nicount: %d\n", vtex_count, indx_count); //debug
	/*
	for (int i = 0; i < vtex_count; ++i)
	{
	printf("%f\n", verts[i]);
	}
	system("PAUSE");
	for (int i = 0; i < indx_count; ++i)
	{
	printf("%d\n", inds[i]);
	}
	*/
}

//this is for 4x4, hard coded.
void createPerspectiveMatrix4(float fov, float aspect, float _near, float _far, GLfloat out_m[4][4])
{

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			out_m[i][j] = 0.0f;
		}
	}

	float angle = (fov / 180.0f) * M_PI;
	float f = 1.0f / tan(angle * 0.5f);

	/* Note, matrices are accessed like 2D arrays in C.
	They are column major, i.e m[y][x] */

	out_m[0][0] = f / aspect;
	out_m[1][1] = f;
	out_m[2][2] = (_far + _near) / (_near - _far);
	out_m[2][3] = -1.0f;
	out_m[3][2] = (2.0f * _far * _near) / (_near - _far);


	/*
	out_m[0][0] = f / aspect;
	out_m[1][1] = f;
	out_m[2][2] = (_far + _near) / (_near - _far);
	out_m[2][3] = (2.0f * _far * _near) / (_near - _far);
	out_m[3][2] = 1.0f;
	*/

	//return m;
}

void createIdentityMatrix4(GLfloat out_m[4][4])
{

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			out_m[i][j] = 0.0f;
		}
	}

	/* Note, matrices are accessed like 2D arrays in C.
	They are column major, i.e m[y][x] */

	out_m[0][0] = 1.0f;
	out_m[1][1] = 1.0f;
	out_m[2][2] = 1.0f;
	out_m[3][3] = 1.0f;

	//return m;
}

void createScaleMatrix4(float sx, float sy, float sz, GLfloat out_m[4][4])
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			out_m[i][j] = 0.0f;
		}
	}

	/* Note, matrices are accessed like 2D arrays in C.
	They are column major, i.e m[y][x] */

	out_m[0][0] = sx;
	out_m[1][1] = sy;
	out_m[2][2] = sz;
	out_m[3][3] = 1.0f;
}

void createTranslationMatrix4(float x, float y, float z, GLfloat out_m[4][4])
{

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			out_m[i][j] = 0.0f;
		}
	}

	/* Note, matrices are accessed like 2D arrays in C.
	They are column major, i.e m[y][x] */

	out_m[0][0] = 1.0f;
	out_m[1][1] = 1.0f;
	out_m[2][2] = 1.0f;
	out_m[3][0] = -x;
	out_m[3][1] = -y;
	out_m[3][2] = -z;
	out_m[3][3] = 1.0f;

	//return m;
}

void createRotationMatrixX4(float roll, GLfloat out_m[4][4])
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			out_m[i][j] = 0.0f;
		}
	}

	/* Note, matrices are accessed like 2D arrays in C.
	They are column major, i.e m[y][x] */

	out_m[0][0] = 1.0f;
	out_m[1][1] = cos(roll);
	out_m[1][2] = -sin(roll);
	out_m[2][1] = sin(roll);
	out_m[2][2] = cos(roll);
	out_m[3][3] = 1.0f;

}

void createRotationMatrixY4(float yaw, GLfloat out_m[4][4])
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			out_m[i][j] = 0.0f;
		}
	}

	/* Note, matrices are accessed like 2D arrays in C.
	They are column major, i.e m[y][x] */

	out_m[0][0] = cos(yaw);
	out_m[0][2] = sin(yaw);
	out_m[1][1] = 1.0f;
	out_m[2][0] = -sin(yaw);
	out_m[2][2] = cos(yaw);
	out_m[3][3] = 1.0f;

}

void createRotationMatrixZ4(float pitch, GLfloat out_m[4][4])
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			out_m[i][j] = 0.0f;
		}
	}

	/* Note, matrices are accessed like 2D arrays in C.
	They are column major, i.e m[y][x] */

	out_m[0][0] = cos(pitch);
	out_m[0][1] = -sin(pitch);
	out_m[1][0] = sin(pitch);
	out_m[1][1] = cos(pitch);
	out_m[2][2] = 1.0f;
	out_m[3][3] = 1.0f;

}

//SLOW!!! but might be good for getting a uniform matrix?
void multMatrices(GLfloat first[4][4], GLfloat second[4][4], GLfloat out_m[4][4])
{
	int sum = 0;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			sum = 0;
			for (int k = 0; k < 4; ++k)
			{
				sum += first[k][i] * second[j][k];
			}
			out_m[i][j] = sum;
		}
	}
}

#pragma endregion HELPER_FUNCTIONS


// Any time the window is resized, this function gets called.  It's setup to the
// "glutReshapeFunc" in main.
void changeViewport(int w, int h){
	glViewport(0, 0, w, h);
}

// Here is the function that gets called each time the window needs to be redrawn.
// It is the "paint" method for our program, and is set up from the glutDisplayFunc in main
void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLfloat view[4][4];
	createIdentityMatrix4(view);
	createTranslationMatrix4(0.0f, 0.0f, 5.0f, view);
	GLfloat * view_p = (GLfloat *)view;
	GLuint tempLoc = glGetUniformLocation(shaderProgramID, "s_mV");
	glUniformMatrix4fv(tempLoc, 1, GL_FALSE, view_p);

	for each(Mesh m in meshes)
	{
		//GLfloat model[4][4], view[4][4], projection[4][4];
		GLfloat modelT[4][4], modelX[4][4], modelY[4][4], modelZ[4][4], modelS[4][4];
		createTranslationMatrix4(m.getX(), m.getY(), m.getZ(), modelT);
		createRotationMatrixX4(m.getRoll(), modelX);
		createRotationMatrixY4(m.getPitch(), modelY);
		createRotationMatrixZ4(m.getYaw(), modelZ);
		createScaleMatrix4(m.getScaleX(), m.getScaleY(), m.getScaleY(), modelS);


		GLfloat * modelT_p = (GLfloat *)modelT;
		GLfloat * modelX_p = (GLfloat *)modelX;
		GLfloat * modelY_p = (GLfloat *)modelY;
		GLfloat * modelZ_p = (GLfloat *)modelZ;
		GLfloat * modelS_p = (GLfloat *)modelS;
		//GLfloat * view_p = (GLfloat *)view;



		/*for (int i = 0; i < 4; ++i)
		{
		for (int j = 0; j < 4; ++j)
		{
		printf("%f ", view[i][j]);
		}
		printf("\n");
		}*/

		tempLoc = glGetUniformLocation(shaderProgramID, "s_mMT");//Matrix that handle the transformations
		glUniformMatrix4fv(tempLoc, 1, GL_FALSE, modelT_p);

		tempLoc = glGetUniformLocation(shaderProgramID, "s_mMX");
		glUniformMatrix4fv(tempLoc, 1, GL_FALSE, modelX_p);

		tempLoc = glGetUniformLocation(shaderProgramID, "s_mMY");
		glUniformMatrix4fv(tempLoc, 1, GL_FALSE, modelY_p);

		tempLoc = glGetUniformLocation(shaderProgramID, "s_mMZ");
		glUniformMatrix4fv(tempLoc, 1, GL_FALSE, modelZ_p);

		tempLoc = glGetUniformLocation(shaderProgramID, "s_mMS");
		glUniformMatrix4fv(tempLoc, 1, GL_FALSE, modelS_p);

		//tempLoc = glGetUniformLocation(shaderProgramID, "s_mV");
		//glUniformMatrix4fv(tempLoc, 1, GL_FALSE, view_p);

		tempLoc = glGetUniformLocation(shaderProgramID, "s_mP");
		glUniformMatrix4fv(tempLoc, 1, GL_FALSE, projection_p);

		//glDrawArrays(GL_TRIANGLES, 0, 3);
		glDrawElements(GL_TRIANGLES, ilen, GL_UNSIGNED_INT, 0);
	}
	glutSwapBuffers();

	//currentYaw += 0.01f; //debug
}

//realy the update func
void timer(int val)
{
	for each (Mesh m in meshes)
	{
		m.update();
	}
	glutPostRedisplay();
	glutTimerFunc(10, timer, 1);
}

/*void mouse(int button, int state, int x, int y)
{
if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
{
if (dYaw > 0.0f)
{
dYaw = 0.0f;
dPitch = 0.0f;
dRoll = 0.01f;
}
else if (dRoll > 0.0f)
{
dYaw = 0.0f;
dPitch = 0.01f;
dRoll = 0.0f;
}
else
{
dYaw = 0.01f;
dPitch = 0.0f;
dRoll = 0.0f;
}
}
}*/

void keyboard(unsigned char key, int x, int y)
{
	if (key == 'w')
	{
		currentY -= 0.1f;
	}
	if (key == 's')
	{
		currentY += 0.1f;
	}
	if (key == 'a')
	{
		currentX += 0.1f;
	}
	if (key == 'd')
	{
		currentX -= 0.1f;
	}
	if (key == 'q')
	{
		currentS += 0.1f;
		//printf("%f", currentS);
	}
	if (key == 'e')
	{
		if (currentS > 0.0f)
		{
			currentS -= 0.1f;
		}
		//printf("%f", currentS);
	}
	if (key == 'i')
	{
		currentRoll += 0.1f;
	}
	if (key == 'k')
	{
		currentRoll -= 0.1f;
	}
	if (key == 'j')
	{
		currentYaw += 0.1f;
	}
	if (key == 'l')
	{
		currentYaw -= 0.1f;
	}
}

int main(int argc, char** argv) {
	// Standard stuff...
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 500);
	glutCreateWindow("Helicopter++");
	glutReshapeFunc(changeViewport);
	glutDisplayFunc(render);
	glutTimerFunc(10, timer, 1);
	//glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glewInit();

	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	// Vertices and colors of a triangle
	// notice, position values are between -1.0f and +1.0f
	/*
	GLfloat vertices[] = { -0.5f, -0.5f, 0.0f,	// Lower-left
	0.5f, -0.5f, 0.0f,				// Lower-right
	0.0f, 0.5f, 0.0f };				// Top
	GLfloat colors[] = { 1.0f, 0.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f };
	GLint indices[] = { 1, 2, 3 };
	*/
	GLfloat * vertices = NULL, *colors = NULL;
	//GLuint * indices = NULL;
	int vlen = 0, clen = 0;
	//int ilen = 0;

	parseFlatObjFile("plane.obj", &vertices, &vlen, &indexArray, &ilen);
	//no color support yet, use position.
	clen = vlen;
	colors = new GLfloat[clen];
	//printf("vlen = %d\n", vlen);
	for (int i = 0; i < clen; ++i) //clamp them vals
	{
		//printf("at %d!\n", i);
		if (vertices[i] < 0.0f)
		{
			colors[i] = 0.0f;
		}
		else
		{
			colors[i] = vertices[i];
		}
		//printf("%f\n", colors[i]);
	}


	printf("ilen: %d\n", ilen);

	//set up matrices
	createPerspectiveMatrix4(45.0f, 1.6f, 0.1f, 10.0f, projection);
	//CreateIdentityMatrix4(projection);

	// Make a shader
	char* vertexShaderSourceCode = readFile("vertexShader.glsl");
	char* fragmentShaderSourceCode = readFile("fragmentShader.glsl");

	//printf("vtex shader: %s\n", vertexShaderSourceCode); // debug stuff
	//printf("fment shader: %s\n", fragmentShaderSourceCode);

	GLuint vertShaderID = makeVertexShader(vertexShaderSourceCode);
	GLuint fragShaderID = makeFragmentShader(fragmentShaderSourceCode);
	shaderProgramID = makeShaderProgram(vertShaderID, fragShaderID);

	printf("vertShaderID is %d\n", vertShaderID);
	printf("fragShaderID is %d\n", fragShaderID);
	printf("shaderProgramID is %d\n", shaderProgramID);


	// Create the "remember all"
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	// Create the buffer, but don't load anything yet
	glBufferData(GL_ARRAY_BUFFER, (vlen + clen)*(sizeof(GLfloat)), NULL, GL_STATIC_DRAW);
	// Load the vertex points
	glBufferSubData(GL_ARRAY_BUFFER, 0, vlen*sizeof(GLfloat), vertices);
	// Load the colors right after that
	glBufferSubData(GL_ARRAY_BUFFER, vlen*sizeof(GLfloat), clen*sizeof(GLfloat), colors);

	//now for the index array!
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ilen * sizeof(GLuint), indexArray, GL_STATIC_DRAW);


	// Find the position of the variables in the shader
	positionID = glGetAttribLocation(shaderProgramID, "s_vPosition");
	colorID = glGetAttribLocation(shaderProgramID, "s_vColor");
	printf("s_vPosition's ID is %d\n", positionID);
	printf("s_vColor's ID is %d\n", colorID);

	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(colorID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glUseProgram(shaderProgramID);
	glEnableVertexAttribArray(positionID);
	glEnableVertexAttribArray(colorID);

	//set up some stuff, maybe
	//gluPerspective(45.0f, 1.6f, 0.1f, 10.0f);

	glutMainLoop();

	return 0;
}
