#include "vertices.h"
#include "patches.h"
#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#pragma warning(disable:4996)

#define BUFFER_OFFSET(offset)  ((GLvoid*) (offset))
const int NumTriangles = (NumTeapotPatches * 2);
const int NumVertices = (NumTriangles * 3);

vec4 points[NumVertices];

GLuint Projection;
GLuint InitShader(const char* vShaderFile, const char* fShaderFile);
enum { X = 0, Y = 1, Z = 2 };



void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	glFlush();
}
void keyboard(unsigned char key, int x, int y)
{

	if (key == 27)
		exit(0);
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

GLuint InitShader(const char* vShaderFile, const char* fShaderFile)
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
void reshape(int width, int height)
{

	glViewport(0, 0, width, height);

	GLfloat left = -4.0, right = 4.0;
	GLfloat bottom = -3.0, top = 5.0;
	GLfloat zNear = -10.0, zFar = 10.0;

	GLfloat aspect = GLfloat(width) / height;

	if (aspect > 0)
	{
		left *= aspect;
		right *= aspect;
	}
	else {

		bottom /= aspect;
		top /= aspect;
	}

	mat4 projection = ortho(left, right, bottom, top, zNear, zFar);
	glUniformMatrix4fv(Projection, 1, GL_FALSE, value_ptr(projection));
}

void init(void)
{

	int Index = 0;
	for (int n = 0; n < NumTeapotPatches; n++) {
		vec4 patch[4][4];

		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				vec3& v = vertices[indices[n][i][j]];
				patch[i][j] = vec4(v[X], v[Y], v[Z], 1.0);
			}
		}


		points[Index++] = patch[0][0];
		points[Index++] = patch[3][0];
		points[Index++] = patch[3][3];
		points[Index++] = patch[0][0];
		points[Index++] = patch[3][3];
		points[Index++] = patch[0][3];
	}

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * NumVertices, points, GL_STATIC_DRAW);

	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	Projection = glGetUniformLocation(program, "Projection");

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(1.0, 1.0, 1.0, 1.0);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(512, 512);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Utah Teapot");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glewInit();
	init();

	glutMainLoop();

	return EXIT_SUCCESS;
}
