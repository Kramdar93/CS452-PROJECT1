/////////////////////////
// Mesh abstracts a generic object to be drawn
//  should be inheritted for interesting objects
//  (also really just stores location and orientation stuff)
// Mark Elsinger 2/12/2014
/////////////////////////

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>

#include "Mesh.h"

Mesh::Mesh(std::string n, float r, float b, float g)
{
	name = n;
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	roll = 0.0f;
	pitch = 0.0f;
	yaw = 0.0f;
	scaleX = 1.0f;
	scaleY = 1.0f;
	scaleZ = 1.0f;
	red = r;
	blue = b;
	green = g;
	//verts = vertexp;
	//inds = indexp;
	//tex = texturep;
}

/*void Mesh::update()
{
	//overwrite for actual entities...
}*/

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

bool Mesh::collidesWith(Mesh other) //1x1 collision box
{
	if (x < other.x + 2.0f
		&& x > other.x - 2.0f
		&& y < other.y + 2.0f
		&& y > other.y - 2.0f)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Mesh::changeColor(float r, float g, float b)
{
	red = r;
	green = g;
	blue = b;
}

#pragma region GETTERS
/*
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
*/

/*
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

std::string Mesh::getName()
{
	return name;
}
*/

#pragma endregion GETTERS
