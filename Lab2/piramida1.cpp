// piramida1.cpp : main project file.

#include "stdafx.h"
#include <GLTools.h>
#define FREEGLUT_STATIC
#include <GL/glut.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <StopWatch.h>

GLuint shader;
GLuint MVPMatrixLocation;
M3DMatrix44f matrix, cameraMatrix, identity;
CStopWatch timer;
GLFrustum viewFrustum;


void ChangeSize(int w, int h) {
	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(40, (float)w/(float)h, 1.0, 10.0);
}

void SetupRC() {
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	shader = gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_COLOR, "vColor");

	identity[0] = 1; identity[1] = 0; identity[2] = 0; identity[3] = 0;
	identity[4] = 0; identity[5] = 1; identity[6] = 0; identity[7] = 0;
	identity[8] = 0; identity[9] = 0; identity[10] = 1; identity[11] = 0;
	identity[12] = 0; identity[13] = 0; identity[14] = 0; identity[15] = 1;


	MVPMatrixLocation=glGetUniformLocation(shader,"MVPMatrix");
	if(MVPMatrixLocation==-1)
	{
		fprintf(stderr,"uniform MVPMatrix could not be found\n");
	}
}

void SetUpFrame(GLFrame &frame, const M3DVector3f origin, const M3DVector3f forward, const M3DVector3f up) {
	frame.SetOrigin(origin);
	M3DVector3f side, oUp;
	m3dCrossProduct3(side, forward, up);
	m3dCrossProduct3(oUp, side, forward);
	frame.SetForwardVector(forward);
	frame.SetUpVector(oUp);
	frame.Normalize();
}

void LookAt(GLFrame &frame, const M3DVector3f eye, const M3DVector3f at, const M3DVector3f up) {
	M3DVector3f forward;
	m3dSubtractVectors3(forward, at, eye);
	SetUpFrame(frame, eye, forward, up);
}

void debugMatrix44(M3DMatrix44f matrix) {
	for(int i=0; i<16; ++i) {
		fprintf(stdout, "%.2f ", matrix[i]);
		if(i%4 == 3) { printf("\n"); }
	}
	printf("\n");
}

void drawGrid(float dist=1) {
	glBegin(GL_LINES);
	for(float i=-10; i<=10; i+=dist) {
		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1, 1, 1);
		glVertex3f(-10, i, 0);
		glVertex3f(10, i, 0);

		glVertex3f(i, -10, 0);
		glVertex3f(i, 10, 0);
	}

	glVertex3f(0, 0, -10);
	glVertex3f(0, 0, 10);
	
	glEnd();
}

void drawPyramid() {
	glBegin(GL_QUADS);
	glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.5, 0.5, 0.5);
	glVertex3f(0.5f, 0.5f, 0.0);
	glVertex3f(0.5f, -0.5f, 0.0);
	glVertex3f(-0.5f, -0.5f, 0.0);
	glVertex3f(-0.5f, 0.5f, 0.0);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0, 1.0, 1.0);
	glVertex3f(-0.5f, -0.5f, 0.0);
	glVertex3f(0.5f, -0.5f, 0.0);
	glVertex3f(0.0, 0.0, 0.7f);

	glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 0.0, 0.0);
	glVertex3f(-0.5f, 0.5f, 0.0);
	glVertex3f(-0.5f, -0.5f, 0.0);
	glVertex3f(0.0, 0.0, 0.7f);

	glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 1.0, 0.0);
	glVertex3f(0.5f, -0.5f, 0.0);
	glVertex3f(0.5f, 0.5f, 0.0);
	glVertex3f(0.0, 0.0, 0.7f);

	glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 0.0, 1.0);
	glVertex3f(0.5f, 0.5f, 0.0);
	glVertex3f(-0.5f, 0.5f, 0.0);
	glVertex3f(0.0, 0.0, 0.7f);

	glEnd();
}

void RenderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable( GL_DEPTH_TEST ) ;
	glUseProgram(shader);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	float angle = timer.GetElapsedSeconds();

	M3DVector3f eye = {cos(angle)*4.0f,sin(angle)*4.0,sin(angle)*4.0f}; 
	M3DVector3f at = {0,0,0}; 
	M3DVector3f up = {0.0f,0.0f,1.0f};

	GLFrame cameraFrame;
	LookAt(cameraFrame, eye, at, up);
	cameraFrame.GetCameraMatrix(cameraMatrix);
	
	m3dMatrixMultiply44(matrix, viewFrustum.GetProjectionMatrix(), cameraMatrix);

	M3DMatrix44f tMatrix;
	m3dTranslationMatrix44(tMatrix, 0, 0, 0);
	m3dMatrixMultiply44(matrix, tMatrix, matrix);

	if((sin(angle) < 0 && sin(angle-0.003) > 0) || (sin(angle) > 0 && sin(angle-0.003) < 0)) {
		debugMatrix44(matrix);
	}

	glUniformMatrix4fv(MVPMatrixLocation,1,GL_FALSE,matrix);

//	drawGrid(0.1);
	drawPyramid();

	glutSwapBuffers();
	glutPostRedisplay();
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Triangle");
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

	GLenum err = glewInit();
	if(GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		return 1;
	}

	SetupRC();

	glutMainLoop();
	return 0;
}