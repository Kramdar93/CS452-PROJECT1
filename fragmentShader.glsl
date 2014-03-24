#version 130

//in vec4 color;
//out vec4 fColor;
in vec2 texcoord;

uniform sampler2D texture;

void main () {
	// pass through the given color
	//fColor = color;

	//solid color
	//fColor = vec4(0.5f, 0.5, 0.5f, 1.0f);

	//textures!
	gl_FragColor = texture2D(texture, texcoord);
}
