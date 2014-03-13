#version 130

in vec4 s_vPosition;
in vec4 s_vColor;
out vec4 color;
uniform mat4 s_mMT;	// The matrix for the translation of the model
uniform mat4 s_mMX; // The matrix for the roll of the model
uniform mat4 s_mMY; // The matrix for the pitch of the model
uniform mat4 s_mMZ; // The matrix for the yaw of the model
uniform mat4 s_mMS; // The matrix for the scale of the model
uniform mat4 s_mV;	// The matrix for the pose of the camera
uniform mat4 s_mP;	// The perspective matrix

void main () {
	// Look, Ma!  I avoided any matrix multiplication!
	// The value of s_vPosition should be between -1.0 and +1.0 (to be visible on the screen)
	gl_Position = s_mP * s_mV * s_mMT * s_mMS * s_mMZ * s_mMX * s_mMY * s_vPosition;
	color = s_vColor;
	//gl_Position = s_vPosition, 0.0f;
}
