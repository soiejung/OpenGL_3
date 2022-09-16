#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/vec2.hpp>
#pragma warning(disable:4996)

#define BUFFER_OFFSET(offset)  ((GLvoid*) (offset))

const int NumTimesToSubdivide = 5;
const int NumTriangles = 729;
const int NumVertices = 3 * NumTriangles;

glm::vec2 points[NumVertices];
glm::vec2 test[3];
GLuint initShader(const char* vShaderFile, const char* fShaderFile);

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, NumTriangles);
	glFlush();
}

void keyboard(unsigned char key, int x, int y)
{

	if (key == 27)
		exit(0);
}


void triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c)
{

	static int i = 0;
	points[i++] = a;

	points[i++] = b;

	points[i++] = c;


}

void divide_triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, int m)
{
	glm::vec2 ab, ac, bc;

	if (m > 0)
	{

		ab = (a + b) / 2.0f;
		ac = (a + c) / 2.0f;
		bc = (b + c) / 2.0f;
		divide_triangle(a, ab, ac, m - 1);
		divide_triangle(c, ac, bc, m - 1);
		divide_triangle(b, bc, ab, m - 1);

	}
	else
		triangle(a, b, c);
}

void init(void)
{

	glm::vec2 vertices[3] = {

		glm::vec2(-1.0, -1.0),
		glm::vec2(0.0, 1.0),
		glm::vec2(1.0, -1.0)
	};

	divide_triangle(vertices[0], vertices[1], vertices[2], NumTimesToSubdivide);


	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	GLuint program = initShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));


	glClearColor(1.0, 1.0, 1.0, 1.0);

}

static char* readShaderSource(const char* shaderFile)
{
	FILE* fp = fopen(shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);

	buf[size] = '\0';
	fclose(fp);

	return buf;
}

GLuint initShader(const char* vShaderFile, const char* fShaderFile)
{
	struct Shader {
		const char* filename;
		GLenum       type;
		GLchar* source;
	}  shaders[2] = {
	{ vShaderFile, GL_VERTEX_SHADER, NULL },
	{ fShaderFile, GL_FRAGMENT_SHADER, NULL }
	};

	GLuint program = glCreateProgram();

	for (int i = 0; i < 2; ++i) {
		Shader& s = shaders[i];
		s.source = readShaderSource(s.filename);
		if (shaders[i].source == NULL) {
			std::cerr << "Failed to read " << s.filename << std::endl;
			exit(EXIT_FAILURE);
		}

		GLuint shader = glCreateShader(s.type);
		glShaderSource(shader, 1, (const GLchar**)&s.source, NULL);
		glCompileShader(shader);

		GLint  compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			std::cerr << s.filename << " failed to compile:" << std::endl;
			GLint  logSize;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
			char* logMsg = new char[logSize];
			glGetShaderInfoLog(shader, logSize, NULL, logMsg);
			std::cerr << logMsg << std::endl;
			delete[] logMsg;

			exit(EXIT_FAILURE);
		}

		delete[] s.source;

		glAttachShader(program, shader);
	}

	/* link  and error check */
	glLinkProgram(program);

	GLint  linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
		std::cerr << "Shader program failed to link" << std::endl;
		GLint  logSize;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
		char* logMsg = new char[logSize];
		glGetProgramInfoLog(program, logSize, NULL, logMsg);
		std::cerr << logMsg << std::endl;
		delete[] logMsg;

		exit(EXIT_FAILURE);
	}

	/* use program object */
	glUseProgram(program);

	return program;
}



int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(512, 512);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Sierpinski Gasket");
	glewInit();

	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

	glutMainLoop();

	return EXIT_SUCCESS;
}
