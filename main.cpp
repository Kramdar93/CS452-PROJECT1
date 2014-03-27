/* Written by Jeffrey Chastine, 2012 */
/* Lazily adapted by Mark Elsinger, 2014 */
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>
#include <random>
#include <ctime>

#define _USE_MATH_DEFINES
#include <math.h>

#include "Mesh.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
typedef unsigned char Uint8; //close enough
GLuint shaderProgramID;
GLuint vao = 0;
GLuint vbo[2];
GLuint positionID, texcoordID, textures[1];
GLuint * indexArray = NULL;
GLfloat projection[4][4];
GLfloat * projection_p = (GLfloat *)projection;
int ilen = 0;
//float currentRoll = 0.0f;
//float currentPitch = 0.0f;
//float currentYaw = 0.0f;
//float dYaw = 0.0f;
//float dRoll= 0.0f;
//float dPitch = 0.0f;
//float currentX = 0.0f;
//float currentY = 0.0f;
//float currentZ = 0.0f;
//float currentS = 1.0f;
float speed = 0.1f;
Mesh player("player", 0.0f, 0.0f, 1.0f);
std::vector<Mesh *> static_objects, moving_objects, wall_objects; //quite ironic really...
//game variables
float velY = 0.0f;
float thrust = 0.0f;
const float MIN_THRUST = -0.01f;
const float MAX_THRUST = 0.02f;
const float TOP = 9.0f;
const float START = 20.0f;
const float INCREMENT = 0.01f;
unsigned int time_next_powerup;
float clearance, ceiling_height, offset;
int score = 0;
const int GOODBADRATIO = 2;
const int OBJSPACES = 10;


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


//tris only for now, restricted practical use. also not really for obj files...
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

//pretty much same as above but provides texture mapping!
void parseUVObjFile(char * filename, GLfloat ** vertices, int * vlength, GLuint ** indices, int * ilength, GLfloat ** texcoords, int * tlength)
{
	GLfloat * verts = *vertices; //this is necessary for some reason...
	GLuint * inds = *indices;    //just using the arguments didn't work...
	GLfloat * texs = *texcoords;

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
	int texs_count = 0;
	do
	{
		in.getline(s, MAX_LINE_LENGTH);
		//printf("%s\n", s); //debug
		if (s[0] == 'v' && s[1] != 't')
		{
			vtex_count = vtex_count + 3;
		}
		if (s[0] == 'f')
		{
			indx_count = indx_count + 3;
		}
		if (s[0] == 'v' && s[1] == 't')
		{
			texs_count = texs_count + 2;
		}
	} while (!in.eof());

	in.clear();
	in.seekg(0, std::ios::beg);

	*vlength = vtex_count;
	*ilength = indx_count;
	*tlength = texs_count;
	//gonna assume 3 per index and vertex!
	//*vertices = new GLfloat[vtex_count];
	//*indices = new GLuint[indx_count];
	verts = new GLfloat[vtex_count];
	inds = new GLuint[indx_count];
	texs = new GLfloat[texs_count];

	int iv = 0;
	int ii = 0;
	int it = 0;
	char * tmp = NULL;
	char * nxt_tmp = NULL;
	do
	{
		in.getline(s, MAX_LINE_LENGTH);
		//printf("%s\n", s);
		if (s[0] == 'v' && s[1] != 't')
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
		else if (s[0] == 'v' && s[1] == 't')
		{
			tmp = strtok_s(&s[2], " ", &nxt_tmp);
			for (int i = 0; i < 2 && it < texs_count; ++i) //assume only u&v coords
			{
				texs[it] = (GLfloat)atof(tmp);
				tmp = strtok_s(NULL, " ", &nxt_tmp);
				++it;
			}
		} 

	} while (iv <= vtex_count && ii <= indx_count && it <= texs_count && !in.eof());

	*vertices = verts;
	*indices = inds;
	*texcoords = texs;

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
	system("PAUSE");
	for (int i = 0; i < texs_count; ++i)
	{
		printf("%f\n", texs[i]);
	}
	system("PAUSE"); */
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

	float angle = (fov / 180.0f) * (float)M_PI;
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
	out_m[3][0] = x;
	out_m[3][1] = y;
	out_m[3][2] = z;
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

//kinda need windows for this bit. easily ported to linux though, just need libraries...
//also nabbed this from http://www.cplusplus.com/articles/GwvU7k9E/, thanks Fredbill =]
int loadBMP(const char* location, GLuint *texture)
{
	Uint8* datBuff[2] = { nullptr, nullptr }; // Header buffers

	Uint8* pixels = nullptr; // Pixels

	BITMAPFILEHEADER* bmpHeader = nullptr; // Header
	BITMAPINFOHEADER* bmpInfo = nullptr; // Info 

	// The file... We open it with it's constructor
	std::ifstream file(location, std::ios::binary);
	if (!file)
	{
		printf("Failure to open bitmap file.\n");

		return 1;
	}

	// Allocate byte memory that will hold the two headers
	datBuff[0] = new Uint8[sizeof(BITMAPFILEHEADER)];
	datBuff[1] = new Uint8[sizeof(BITMAPINFOHEADER)];

	file.read((char*)datBuff[0], sizeof(BITMAPFILEHEADER));
	file.read((char*)datBuff[1], sizeof(BITMAPINFOHEADER));

	// Construct the values from the buffers
	bmpHeader = (BITMAPFILEHEADER*)datBuff[0];
	bmpInfo = (BITMAPINFOHEADER*)datBuff[1];

	// Check if the file is an actual BMP file
	if (bmpHeader->bfType != 0x4D42)
	{
		printf("File \"%s\" isn't a bitmap file\n", location);
		return 2;
	}

	// First allocate pixel memory
	pixels = new Uint8[bmpInfo->biSizeImage];

	// Go to where image data starts, then read in image data
	file.seekg(bmpHeader->bfOffBits);
	file.read((char*)pixels, bmpInfo->biSizeImage);

	// We're almost done. We have our image loaded, however it's not in the right format.
	// .bmp files store image data in the BGR format, and we have to convert it to RGB.
	// Since we have the value in bytes, this shouldn't be to hard to accomplish
	Uint8 tmpRGB = 0; // Swap buffer
	for (unsigned long i = 0; i < bmpInfo->biSizeImage; i += 3)
	{
		tmpRGB = pixels[i];
		pixels[i] = pixels[i + 2];
		pixels[i + 2] = tmpRGB;
	}

	// Set width and height to the values loaded from the file
	GLuint w = bmpInfo->biWidth;
	GLuint h = bmpInfo->biHeight;

	/*******************GENERATING TEXTURES*******************/

	//enable that texturing?
	glEnable(GL_TEXTURE_2D);
	//glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, texture);             // Generate a texture
	glBindTexture(GL_TEXTURE_2D, *texture); // Bind that texture temporarily

	GLint mode = GL_RGB;                   // Set the mode

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Create the texture. We get the offsets from the image, then we use it with the image's
	// pixel data to create it.
	glTexImage2D(GL_TEXTURE_2D, 0, mode, w, h, 0, mode, GL_UNSIGNED_BYTE, pixels);

	// Unbind the texture
	glBindTexture(GL_TEXTURE_2D, NULL);

	// Output a successful message
	printf("Texture \"%s\" successfully loaded.\n", location);

	// Delete the two buffers.
	delete[] datBuff[0];
	delete[] datBuff[1];
	delete[] pixels;

	return 0; // Return success code 
}

#pragma endregion HELPER_FUNCTIONS


// Any time the window is resized, this function gets called.  It's setup to the
// "glutReshapeFunc" in main.
void changeViewport(int w, int h){
	glViewport(0, 0, w, h);
}

void renderMesh(Mesh m)
{

	//GLfloat model[4][4], view[4][4], projection[4][4];
	GLfloat modelT[4][4], modelX[4][4], modelY[4][4], modelZ[4][4], modelS[4][4], view[4][4];
	/*
	createTranslationMatrix4(m.getX(), m.getY(), m.getZ(), modelT);
	createRotationMatrixX4(m.getRoll(), modelX);
	createRotationMatrixY4(m.getPitch(), modelY);
	createRotationMatrixZ4(m.getYaw(), modelZ);
	createScaleMatrix4(m.getScaleX(), m.getScaleY(), m.getScaleY(), modelS);
	*/
	createTranslationMatrix4(m.x, m.y, m.z, modelT);
	createRotationMatrixX4(m.roll, modelX);
	createRotationMatrixY4(m.pitch, modelY);
	createRotationMatrixZ4(m.yaw, modelZ);
	createScaleMatrix4(m.scaleX, m.scaleY, m.scaleZ, modelS);

	createTranslationMatrix4(0.0f, 0.0f, -20.0f, view);

	GLfloat * modelT_p = (GLfloat *)modelT;
	GLfloat * modelX_p = (GLfloat *)modelX;
	GLfloat * modelY_p = (GLfloat *)modelY;
	GLfloat * modelZ_p = (GLfloat *)modelZ;
	GLfloat * modelS_p = (GLfloat *)modelS;
	GLfloat * view_p = (GLfloat *)view;



	/*for (int i = 0; i < 4; ++i)
	{
	for (int j = 0; j < 4; ++j)
	{
	printf("%f ", view[i][j]);
	}
	printf("\n");
	}*/

	GLint tempLoc = glGetUniformLocation(shaderProgramID, "s_mMT");//Matrix that handle the transformations
	glUniformMatrix4fv(tempLoc, 1, GL_FALSE, modelT_p);

	tempLoc = glGetUniformLocation(shaderProgramID, "s_mMX");
	glUniformMatrix4fv(tempLoc, 1, GL_FALSE, modelX_p);

	tempLoc = glGetUniformLocation(shaderProgramID, "s_mMY");
	glUniformMatrix4fv(tempLoc, 1, GL_FALSE, modelY_p);

	tempLoc = glGetUniformLocation(shaderProgramID, "s_mMZ");
	glUniformMatrix4fv(tempLoc, 1, GL_FALSE, modelZ_p);

	tempLoc = glGetUniformLocation(shaderProgramID, "s_mMS");
	glUniformMatrix4fv(tempLoc, 1, GL_FALSE, modelS_p);

	tempLoc = glGetUniformLocation(shaderProgramID, "s_mV");
	glUniformMatrix4fv(tempLoc, 1, GL_FALSE, view_p);

	tempLoc = glGetUniformLocation(shaderProgramID, "s_mP");
	glUniformMatrix4fv(tempLoc, 1, GL_FALSE, projection_p);

	tempLoc = glGetUniformLocation(shaderProgramID, "color");
	glUniform4f(tempLoc, m.red, m.blue, m.green, 1.0f);

	//glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glDrawElements(GL_TRIANGLES, ilen, GL_UNSIGNED_INT, NULL);
	glBindTexture(GL_TEXTURE_2D, NULL);
}


void gameInit()
{
	srand(time(NULL));
	player.x = -10.0f;

	//static_objects.push_back(new Mesh("test", 0.5f, 0.5f, 0.5f));
	Mesh * temp = new Mesh("wall", 1.0f, 0.0f, 0.0f);
	temp->translate(START, TOP, 0.0f);
	wall_objects.push_back(temp);

	temp = new Mesh("wall", 1.0f, 0.0f, 0.0f);
	temp->translate(START, -TOP, 0.0f);
	wall_objects.push_back(temp);

	clearance = 18.0f;
	ceiling_height = 0.0f; //as in 4 from top of allowed area
	offset = 0.0f;

	printf("Game init'd!\n");
}

void endGame()
{
	printf("Game Over\n");
	printf("Score: %d\n", score);
	system("pause");
	exit(1);
}



//updates the world-> throws more objects at the player and checks collisions, game logic.
//be sure to call initGame.
void updateWorld()
{
	if (wall_objects.empty()) //make sure you're getting some obstacles.
	{
		gameInit();
	}
	
	if (wall_objects.front()->x < -START) // wall (0 & 1) went well off-screen
	{
		delete(wall_objects.front());			//free the memory
		wall_objects.erase(wall_objects.begin()); //remove the pointer from the vector
		delete(wall_objects.front());			
		wall_objects.erase(wall_objects.begin()); 
	}

	if (wall_objects.back()->x <= START - 2.0f + speed) //trailing wall has moved out of space, can make new walls!
														//note planes have a with of 2!!!
	{
		int ceilingMod = 25;
		int r = rand() % 3 - 1; //-1, 0, or 1
		clearance -= INCREMENT;          // always gets smaller... it's a trap!
		ceiling_height += r * INCREMENT * ceilingMod; // makes tunnel twist a bit
		offset = 0.5f * (2 * TOP - clearance);

		if (clearance < 2.0f) //cant have negative space, remember width
		{
			clearance = 2.0f; //although game will be over before this...
		}
		if (ceiling_height + offset < 0) //ceiling cant be negatively high
		{
			ceiling_height = INCREMENT * ceilingMod;
		}
		if (ceiling_height + offset + clearance > 2 * TOP) //that's lower than -top! do not want!
		{
			ceiling_height -= INCREMENT * 2 * ceilingMod;
		}

		Mesh * temp;
		temp = new Mesh("wall", 1.0f, 0.0f, 0.0f);
		temp->translate(START, TOP - (ceiling_height + offset), 0.0f);
		wall_objects.push_back(temp);

		temp = new Mesh("wall", 1.0f, 0.0f, 0.0f);
		temp->translate(START, TOP - (ceiling_height + offset) - clearance, 0.0f);
		wall_objects.push_back(temp);
		//printf("added walls!\n");
		++score; //reward for new segment added.
	}

	if (static_objects.empty() || static_objects.back()->x < 0) //no objects or last one passed center
	{
		Mesh * temp;
		int rint = (rand() % OBJSPACES) + 1;
		float position = clearance - ((clearance) / (OBJSPACES+1)) * (rint); //position from wall, selected randomly
		//(clearance/OBJSPACES) to clearance
		//printf("%d: %f\n", rint, position);

		if (rand() % (GOODBADRATIO + 1) == 0)
		{
			temp = new Mesh("good", 0.0f, 1.0f, 0.0f);
		}
		else
		{
			temp = new Mesh("bad", 1.0f, 0.0f, 0.0f);
		}

		if (clearance < 8.0f) //not enough room for purely random, could block player
			//planes are 2x2
		{
			int r = rand() % 2;
			position -= 4.0f; //give enough room
			if (position < 2.0f)
			{
				position = 2.0f;
			}

			if (r == 0) //ensure room on top
			{
				position = clearance - position; //flip position
			}
		}

		if (clearance > 6.0f || temp->name == "good")
		{
			temp->translate(START, TOP - (ceiling_height + offset) - position, 0.0f);
			static_objects.push_back(temp);
		}

	}

	for each (Mesh * m in wall_objects)
	{
		if (m->collidesWith(player))
		{
			endGame();
		}
	}
	for each (Mesh * m in static_objects)
	{
		if (m->collidesWith(player))
		{
			if (m->name == "good")
			{
				score += 100;
				m->name = "used";
				m->changeColor(0.5f, 0.5f, 0.5f);
			}
			else if (m->name == "bad")
			{
				endGame();
			}
		}
	}
}

// Here is the function that gets called each time the window needs to be redrawn.
// It is the "paint" method for our program, and is set up from the glutDisplayFunc in main
void render() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for each(Mesh* m in wall_objects)
	{
		renderMesh(*m);
	}

	for each(Mesh* m in static_objects)
	{
		renderMesh(*m);
	}

	renderMesh(player);

	glutSwapBuffers();

	//currentYaw += 0.01f; //debug
}

//realy the update func
void update(int val)
{
	//currentYaw += dYaw; //debug
	//currentRoll += dRoll;
	//currentPitch += dPitch;

	updateWorld();

	for each(Mesh* m in static_objects)
	{
		m->translate(-speed, 0.0f, 0.0f);
		//printf("%f\n", m->x);
	}

	for each(Mesh* m in wall_objects)
	{
		m->translate(-speed, 0.0f, 0.0f);
		//printf("%f\n", m->x);
	}

	velY += thrust * speed;
	player.translate(0.0f, velY, 0.0f);
	speed += INCREMENT * 0.01f;
	//++score;
	//printf("%f\n", player.y);
	glutPostRedisplay();
	glutTimerFunc(17, update, 1);
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			thrust = MAX_THRUST;
		}
		else
		{
			thrust = MIN_THRUST;
		}
	}
}

/*
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
} */


int main(int argc, char** argv) {
	// Standard stuff...
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 500);
	glutCreateWindow("Helicopter++");
	glutReshapeFunc(changeViewport);
	glutDisplayFunc(render);
	glutTimerFunc(17, update, 1);
	glutMouseFunc(mouse);
	//glutKeyboardFunc(keyboard);
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
	GLfloat * vertices = NULL, *texcoord = NULL;
	//GLuint * indices = NULL;
	int vlen = 0, tlen = 0;
	//int ilen = 0;

	//parseFlatObjFile("plane.obj", &vertices, &vlen, &indexArray, &ilen);
	parseUVObjFile("plane.mesh", &vertices, &vlen, &indexArray, &ilen, &texcoord, &tlen);

	printf("vlen: %d\n", vlen);
	printf("tlen: %d\n", tlen);
	printf("ilen: %d\n", ilen);

	//set up matrices
	createPerspectiveMatrix4(45.0f, 1.6f, 10.0f, 30.0f, projection);
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
	glBufferData(GL_ARRAY_BUFFER, (vlen + tlen)*(sizeof(GLfloat)), NULL, GL_STATIC_DRAW);
	// Load the vertex points
	glBufferSubData(GL_ARRAY_BUFFER, 0, vlen*sizeof(GLfloat), vertices);
	// Load the colors right after that
	glBufferSubData(GL_ARRAY_BUFFER, vlen*sizeof(GLfloat), tlen*sizeof(GLfloat), texcoord);

	//now for the index array!
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ilen * sizeof(GLuint), indexArray, GL_STATIC_DRAW);


	// Find the position of the variables in the shader
	positionID = glGetAttribLocation(shaderProgramID, "s_vPosition");
	texcoordID = glGetAttribLocation(shaderProgramID, "s_vTexcoord");
	printf("s_vPosition's ID is %d\n", positionID);
	printf("s_vTexcoord's ID is %d\n", texcoordID);

	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(texcoordID, 2, GL_FLOAT, GL_FALSE, 0, 0);
	//glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glUseProgram(shaderProgramID);
	glEnableVertexAttribArray(positionID);
	glEnableVertexAttribArray(texcoordID);

	//set up some stuff, maybe
	//gluPerspective(45.0f, 1.6f, 0.1f, 10.0f);
	loadBMP("Heli1.bmp", &textures[0]); //remember, heli is texture #0!

	GLuint texID = glGetUniformLocation(shaderProgramID, "s_fTexture");
	printf("s_fTexture location: %d\n", texID);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(texID, 0); //point dat uniform to the texture.

	//load up game stuff
	gameInit();

	glutMainLoop();

	return 0;
}
