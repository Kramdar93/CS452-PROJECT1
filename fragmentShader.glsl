#version 130

in vec4 color;
out vec4 fColor;

void main () {
	// pass through the given color
	fColor = color;

	//solid color
	//fColor = vec4(0.5f, 0.5, 0.5f, 1.0f);
}
