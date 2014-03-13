/////////////////////////
// Mesh abstracts a generic object to be drawn
//  should be inheritted for interesting objects
// Mark Elsinger 2/12/2014
/////////////////////////

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Mesh.h"

Mesh::Mesh(GLuint vertexp, GLuint indexp, GLuint texturep)
{
	x = 0;
	y = 0;
	z = 0;
	roll = 0;
	pitch = 0;
	yaw = 0;
	scaleX = 0;
	scaleY = 0;
	scaleZ = 0;
}

void Mesh::update()
{
	//overwrite for actual entities...
}

void Mesh::translate(float dx, float dy, float dz)
{
	x += dx;
	y += dy;
	z += dz;
}

void Mesh::rotate(float dpitch, float dyaw, float droll)
{
	pitch += dpitch;
	yaw += dyaw;
	roll += droll;
}

void Mesh::scale(float dscalex, float dscaley, float dscalez)
{
	scaleX += dscalex;
	scaleY += dscaley;
	scaleZ += dscalez;
	if (scaleX < 0.0f)
	{
		scaleX = 0.0f;
	}
	if (scaleY < 0.0f)
	{
		scaleY = 0.0f;
	}
	if (scaleZ < 0.0f)
	{
		scaleZ = 0.0f;
	}
}

#pragma region GETTERS

GLuint Mesh::getVertP()
{
	return verts;
}

GLuint Mesh::getIndP()
{
	return inds;
}

GLuint Mesh::getTexP()
{
	return tex;
}

float Mesh::getX()
{
	return x;
}

float Mesh::getY()
{
	return y;
}

float Mesh::getZ()
{
	return z;
}

float Mesh::getRoll()
{
	return roll;
}

float Mesh::getPitch()
{
	return pitch;
}

float Mesh::getYaw()
{
	return yaw;
}

float Mesh::getScaleX()
{
	return scaleX;
}

float Mesh::getScaleY()
{
	return scaleY;
}

float Mesh::getScaleZ()
{
	return scaleZ;
}

#pragma endregion GETTERS
