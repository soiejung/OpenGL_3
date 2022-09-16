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

int NumTimesToSubdivide = 3;
int PatchesPerSubdivision = 4;
int NumQuadsPerPatch = (int)pow(PatchesPerSubdivision, NumTimesToSubdivide);
int NumTriangles = (NumQuadsPerPatch * 2);
const int NumVertices = 32 * 64 * 12;

int Index = 0;
int selectedPoint = 0;
vec4 points[NumVertices];


GLuint Projection;
GLuint InitShader(const char* vShaderFile, const char* fShaderFile);
enum { X = 0, Y = 1, Z = 2 };



void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	//glutWireTeapot(0.5);
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


void divide_curve(vec4 c[4], vec4 r[4], vec4 l[4])
{
	int i;
	vec4 t;

	for (i = 0; i < 3; i++) {
		l[0][i] = c[0][i];
		r[3][i] = c[3][i];
		l[1][i] = (c[1][i] + c[0][i]) / 2;
		r[2][i] = (c[2][i] + c[3][i]) / 2;
		t[i] = (l[1][i] + r[2][i]) / 2;
		l[2][i] = (t[i] + l[1][i]) / 2;
		r[1][i] = (t[i] + r[2][i]) / 2;
		l[3][i] = r[0][i] = (l[2][i] + r[1][i]) / 2;
	}
	for (i = 0;i < 4;i++)
		l[i][3] = r[i][3] = 1.0;
}

void draw_patch(vec4 p[4][4]) {


	points[Index++] = p[0][0];
	points[Index++] = p[3][0];
	points[Index++] = p[3][3];
	points[Index++] = p[0][0];
	points[Index++] = p[3][3];
	points[Index++] = p[0][3];
}

void transpose(vec4 v[4][4]) {

	int i, j, k;
	vec4 tmp;
	for (i = 0;i < 4;i++) {
		for (j = i;j < 4;j++) {
			for (k = 0;k < 3;k++) {

				tmp = v[i][j];
				v[i][j] = v[j][i];
				v[j][i] = tmp;
			}
		}
	}
}

void divide_patch(vec4 p[4][4], int n) {

	vec4 q[4][4], r[4][4], s[4][4], t[4][4];
	vec4 a[4][4], b[4][4];
	int k;
	if (n == 0)
		draw_patch(p);

	else {

		for (k = 0;k < 4;k++)
			divide_curve(p[k], a[k], b[k]);

		transpose(a);
		transpose(b);

		for (k = 0;k < 4;k++) {
			divide_curve(a[k], q[k], r[k]);
			divide_curve(b[k], s[k], t[k]);
		}

		divide_patch(q, n - 1);
		divide_patch(r, n - 1);
		divide_patch(s, n - 1);
		divide_patch(t, n - 1);
	}
}

void init(void)
{


	int Index = 0;
	vec4 patch[4][4];
	for (int n = 0; n < NumTeapotPatches; n++) {

		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				vec3& v = vertices[indices[n][i][j]];
				patch[i][j] = vec4(v[X], v[Y], v[Z], 1.0);
			}
		}

		divide_patch(patch, NumTimesToSubdivide);
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
	glutCreateWindow("Utah Teapot (interaction)");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glewInit();
	init();

	glutMainLoop();

	return EXIT_SUCCESS;
}
