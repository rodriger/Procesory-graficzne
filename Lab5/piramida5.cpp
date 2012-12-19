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
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <vector>

GLuint shader;
GLuint MVPMatrixLocation, shaderPositionLocation, shaderColorLocation, shaderAngleLocation, 
		shaderAttenuation0Location, shaderAttenuation1Location, shaderAttenuation2Location,
		MVMatrixLocation, normalMatrixLocation;
GLuint materialAmbientLocation, materialDiffuseLocation, materialSpecularLocation, materialExponentLocation;
M3DMatrix44f mVP, mMVP, cameraMatrix, identity;
CStopWatch timer;
GLFrustum viewFrustum;
GLGeometryTransform geometryPipeline;

std::vector<float> vertices;
std::vector<GLuint> faces;

int n_vertices, n_faces;

GLuint vertex_buffer, faces_buffer;

void loadObjectData() {
	n_vertices = 0;
	n_faces = 0;

	FILE *fvertices=fopen("geode_vertices.dat","r");
	if(fvertices==NULL) {
		fprintf(stderr,"cannot open vertices file for reading\n");
		exit(-1);
	}
	char line[120];

	while(fgets(line,120,fvertices)!=NULL) {
		float x,y,z;
		double norm;
		sscanf(line,"%f %f %f",&x,&y,&z);

		norm=x*x+y*y+z*z;
		norm=sqrt(norm);
		n_vertices++;
		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);
		vertices.push_back(1.0f);
		vertices.push_back(x/norm);
		vertices.push_back(y/norm);
		vertices.push_back(z/norm);
	}

	fprintf(stderr,"nv = %u %u\n",n_vertices,vertices.size());

	// Load faces
	FILE *ffaces=fopen("geode_faces.dat","r");
	if(ffaces==NULL) {
		fprintf(stderr,"cannot open faces file for reading\n");
		exit(-1);
	}

	while(fgets(line,120,ffaces)!=NULL) {
		GLuint  i,j,k;

		if(3!=sscanf(line,"%u %u %u",&i,&j,&k)){
			fprintf(stderr,"error reading faces\n"); 
			exit(-1);
		}
		//fprintf(stderr,"%u %u %u\n",i-1,j-1,k-1);
		n_faces++;
		faces.push_back(i-1);
		faces.push_back(j-1);
		faces.push_back(k-1);
	}
	fprintf(stderr,"nf = %u\n",n_faces);
}

void prepareBuffers() {
	glGenBuffers(1,&vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	glBufferData(GL_ARRAY_BUFFER,n_vertices*sizeof(float)*7,&vertices[0],GL_STATIC_DRAW);
	if(glGetError()!=GL_NO_ERROR) {
		fprintf(stderr,"error copying vertices\n");
	}

	glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX,4,GL_FLOAT,GL_FALSE,sizeof(float)*7,(const GLvoid *)0);

	glVertexAttribPointer(GLT_ATTRIBUTE_NORMAL,3,GL_FLOAT,GL_FALSE, 
		sizeof(float)*7,(const GLvoid *)(4*sizeof(float)) );

	glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
	glEnableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);

	glGenBuffers(1,&faces_buffer);
		if(glGetError()!=GL_NO_ERROR) {
		fprintf(stderr,"faces_buffer invalid\n");
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,faces_buffer);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER,n_faces*sizeof(GLuint)*3,&faces[0],GL_STATIC_DRAW);
	if(glGetError()!=GL_NO_ERROR) {
		fprintf(stderr,"error copying faces\n");
	}
}

void drawObject() {
	glDrawElements(GL_TRIANGLES,3*n_faces,GL_UNSIGNED_INT,0);
}


float ico_vertices[3 * 12] = {
      0., 0., -0.9510565162951536,
      0., 0., 0.9510565162951536,
      -0.85065080835204, 0., -0.42532540417601994,
      0.85065080835204, 0., 0.42532540417601994,
      0.6881909602355868, -0.5, -0.42532540417601994,
      0.6881909602355868, 0.5, -0.42532540417601994,
      -0.6881909602355868, -0.5, 0.42532540417601994,
      -0.6881909602355868, 0.5, 0.42532540417601994,
      -0.2628655560595668, -0.8090169943749475, -0.42532540417601994,
      -0.2628655560595668, 0.8090169943749475, -0.42532540417601994,
      0.2628655560595668, -0.8090169943749475, 0.42532540417601994,
      0.2628655560595668, 0.8090169943749475, 0.42532540417601994
      };
int ico_faces[3*20]={
      1 ,			 11 ,			 7 ,
      1 ,			 7 ,			 6 ,
      1 ,			 6 ,			 10 ,
      1 ,			 10 ,			 3 ,
      1 ,			 3 ,			 11 ,
      4 ,			 8 ,			 0 ,
      5 ,			 4 ,			 0 ,
      9 ,			 5 ,			 0 ,
      2 ,			 9 ,			 0 ,
      8 ,			 2 ,			 0 ,
      11 ,			 9 ,			 7 ,
      7 ,			 2 ,			 6 ,
      6 ,			 8 ,			 10 ,
      10 ,			 4 ,			 3 ,
      3 ,			 5 ,			 11 ,
      4 ,			 10 ,			 8 ,
      5 ,			 3 ,			 4 ,
      9 ,			 11 ,			 5 ,
      2 ,			 7 ,			 9 ,
      8 ,			 6 ,			 2 };

void TriangleFace(M3DVector3f a, M3DVector3f b, M3DVector3f c) {
   M3DVector3f normal, bMa, cMa;
   m3dSubtractVectors3(bMa, b, a);
   m3dSubtractVectors3(cMa, c, a);
   m3dCrossProduct3(normal, bMa, cMa);
   m3dNormalizeVector3(normal);
   glVertexAttrib3fv(GLT_ATTRIBUTE_NORMAL, normal);
   glVertex3fv(a);
   glVertex3fv(b);
   glVertex3fv(c);
}

void SquareFace(M3DVector3f a, M3DVector3f b, M3DVector3f c, M3DVector3f d) {
   M3DVector3f normal, bMa, cMa;
   m3dSubtractVectors3(bMa, b, a);
   m3dSubtractVectors3(cMa, c, a);
   m3dCrossProduct3(normal, bMa, cMa);
   m3dNormalizeVector3(normal);
   glVertexAttrib3fv(GLT_ATTRIBUTE_NORMAL, normal);
   glVertex3fv(a);
   glVertex3fv(b);
   glVertex3fv(c);
   glVertex3fv(d);
}

void drawTriangles(int n_faces, float *vertices, int *faces) {
      for (int i = 0; i < n_faces; i++) {
		  glBegin(GL_TRIANGLES);
		  TriangleFace(vertices + 3 * faces[3 * i], vertices + 3 * faces[3 * i + 1], vertices + 3 * faces[3 * i + 2]);
		  glEnd();
      }
}


void ChangeSize(int w, int h) {
	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(40, (float)w/(float)h, 1.0, 20.0);
}

void setLightColor(int r, int g, int b) {
	M3DVector3f lightColor = {(float)r/255., (float)g/255, (float)b/255};
	glUniform3fv(shaderColorLocation, 1, lightColor);
}

void SetupRC() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	shader = gltLoadShaderPairWithAttributes("gouraud_phong.vp", "gouraud.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal");
	shaderPositionLocation = glGetUniformLocation(shader, "light1.position");
	shaderColorLocation = glGetUniformLocation(shader, "light1.color");
	shaderAngleLocation = glGetUniformLocation(shader, "light1.angle");
	shaderAttenuation0Location = glGetUniformLocation(shader, "light1.attenuation0");
	shaderAttenuation1Location = glGetUniformLocation(shader, "light1.attenuation1");
	shaderAttenuation2Location = glGetUniformLocation(shader, "light1.attenuation2");
	materialAmbientLocation = glGetUniformLocation(shader, "material.ambientColor");
	materialDiffuseLocation = glGetUniformLocation(shader, "material.diffuseColor");
	materialSpecularLocation = glGetUniformLocation(shader, "material.specularColor");
	materialExponentLocation = glGetUniformLocation(shader, "material.specularExponent");
	MVPMatrixLocation=glGetUniformLocation(shader,"MVPMatrix");
	MVMatrixLocation=glGetUniformLocation(shader,"MVMatrix");
	normalMatrixLocation=glGetUniformLocation(shader,"normalMatrix");
	if(MVPMatrixLocation==-1)
	{
		fprintf(stderr,"uniform MVPMatrix could not be found\n");
	}

	identity[0] = 1; identity[1] = 0; identity[2] = 0; identity[3] = 0;
	identity[4] = 0; identity[5] = 1; identity[6] = 0; identity[7] = 0;
	identity[8] = 0; identity[9] = 0; identity[10] = 1; identity[11] = 0;
	identity[12] = 0; identity[13] = 0; identity[14] = 0; identity[15] = 1;

	glUseProgram(shader);

	M3DVector3f ambientColor = {0.f, 0.f, 0.f};
	glUniform3fv(materialAmbientLocation, 1, ambientColor);
	M3DVector3f diffuseColor = {0.8f, 0.8f, 1.f};
	glUniform3fv(materialDiffuseLocation, 1, diffuseColor);
	M3DVector3f specularColor = {0.8f, 0.8f, 1.f};
	glUniform3fv(materialSpecularLocation, 1, specularColor);
	float specularExponent = 100.f;
	glUniform1f(materialExponentLocation, specularExponent);


	M3DVector3f lightPosition = {5.f, 0.f, 5.f};
	glUniform3fv(shaderPositionLocation, 1, lightPosition);
	setLightColor(96, 179, 240);
//	M3DVector3f lightColor = {0.25f, 0.7f, .95f};
//	glUniform3fv(shaderColorLocation, 1, lightColor);

	float angle = 0;
	glUniform1f(shaderAngleLocation, angle); // ?????????????

	float attenuation0 = 0.2f;
	float attenuation1 = 0.02f;
	float attenuation2 = 0.001f;
	glUniform1f(shaderAttenuation0Location, attenuation0);
	glUniform1f(shaderAttenuation1Location, attenuation1);
	glUniform1f(shaderAttenuation2Location, attenuation2);

	loadObjectData();
	prepareBuffers();

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

void drawGrid(float dist=1, bool vertical=false) {
	glBegin(GL_LINES);
	for(float i=-10; i<=10; i+=dist) {
		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1, 1, 1);
		glVertex3f(-10, i, 0);
		glVertex3f(10, i, 0);

		glVertex3f(i, -10, 0);
		glVertex3f(i, 10, 0);
	}

	if(vertical) {
		glVertex3f(0, 0, -10);
		glVertex3f(0, 0, 10);
	}

	glEnd();
}

void drawPlatform() {
	float vertices[3 * 4] = {
		10., 10., 0,
		-10., 10., 0,
		-10., -10., 0,
		10., -10., 0
	};

	float faces[3 * 1] = {
		1, 1, 1
	};

	glEnable(GL_POLYGON_OFFSET_FILL);
	glBegin(GL_QUADS);
		SquareFace(vertices, vertices+3, vertices+6, vertices+9);
	glEnd();
	glDisable(GL_POLYGON_OFFSET_FILL);
}

void draw20() {
	drawTriangles(20, ico_vertices, ico_faces);
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

void updateShaderData() {
	glUniformMatrix4fv(MVPMatrixLocation,1,GL_FALSE,geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(MVMatrixLocation,1,GL_FALSE,geometryPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(normalMatrixLocation,1,GL_FALSE,geometryPipeline.GetNormalMatrix());
}

void RenderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable( GL_DEPTH_TEST ) ;
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	float angle = timer.GetElapsedSeconds();

	M3DVector3f eye = {cos(angle)*4.0f,sin(angle)*4.0,sin(angle)*3.0f+3.5}; 
	M3DVector3f at = {0,0,1.0f}; 
	M3DVector3f up = {0.0f,0.0f,1.0f};

	GLFrame cameraFrame;
	LookAt(cameraFrame, eye, at, up);
	cameraFrame.GetCameraMatrix(cameraMatrix);
	
	GLMatrixStack modelView;
	GLMatrixStack projection;
	geometryPipeline.SetMatrixStacks(modelView, projection);

	projection.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelView.LoadMatrix(cameraMatrix);
	updateShaderData();
	drawGrid(1);
	drawPlatform();
	modelView.Rotate(angle*100, 0, 0, 1.0f);
	modelView.Translate(0, 0, 1.f);
	updateShaderData();
//	draw20();
	drawObject();
	modelView.Translate(-1.5f, -1.0f, 0);
	modelView.Rotate(angle*100, 0, 0, 1.0f);
	modelView.Scale(0.5f, 0.5f, 0.5f);
	updateShaderData();
//	draw20();
	drawObject();
	modelView.PopMatrix();

	glutSwapBuffers();
	glutPostRedisplay();
}

int main(int argc, char* argv[])
{
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

