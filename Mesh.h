/////////////////////////
// Mesh abstracts a generic object to be drawn
// Mark Elsinger 2/12/2014
/////////////////////////

#ifndef __MESH_H__
#define __MESH_H__

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>

class Mesh{
public:
	Mesh(std::string n, float r, float b, float g);
	//void update();
	//void draw();
	void translate(float dx, float dy, float dz);
	void rotate(float dpitch, float dyaw, float droll);
	void scale(float dscalex, float dscaley, float dscalez);
	void getMatrix(GLfloat out_m[4][4]);
	bool collidesWith(Mesh other);
	void changeColor(float r, float g, float b);
	//GLuint getVertP();
	//GLuint getIndP();
	//GLuint getTexP();
	/*float getX();
	float getY();
	float getZ();
	float getRoll();
	float getPitch();
	float getYaw();
	float getScaleX();
	float getScaleY();
	float getScaleZ();
	float getRed();
	float getBlue();
	float getGreen();
	std::string getName();*/

//private:
	float x, y, z, pitch, yaw, roll, scaleX, scaleY, scaleZ, red, blue, green;
	std::string name;
	//GLuint verts, inds, tex;
	//bool ready;
	//GLfloat matrix[4][4];
};

#endif
