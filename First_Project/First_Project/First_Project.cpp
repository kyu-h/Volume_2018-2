#include "stdafx.h"
#include <stdlib.h>
#include <GL/glut.h>

#define WIDTH   256
#define HEIGHT  256

int color = 0;

unsigned char ImageBuf[WIDTH][HEIGHT] = { 0,255,0,255, 255,0,255,0, 0,255,0,255, 255,0,255,0 };
unsigned char MyTexture[WIDTH][HEIGHT][3];

void FillMyTexture() {
	for (int s = 0; s < WIDTH; s++) {
		for (int t = 0; t < HEIGHT; t++) {
			//unsigned char Intensity = ImageBuf[s][t];
			//rand() % 255;
			
			MyTexture[s][t][0] = rand() % 255;
			MyTexture[s][t][1] = MyTexture[s][t][2] = 0; // 순서R, G, B
		}
	}
}

void MyInit() {

	glClearColor(0.0, 0.0, 0.0, 0.0);
	FillMyTexture();

	glTexImage2D(GL_TEXTURE_2D, 0, 3, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, &MyTexture[0][0][0]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	glEnable(GL_TEXTURE_2D);
}

void MyDisplay() {
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_QUADS);
	if (color == 1) {
		glColor3f(1, 1, 0);
	}
	else {
		glColor3f(1, 1, 1);
	}
	float fSize = 0.6f;
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-fSize, -fSize, 0.0);

	glTexCoord2f(0.0, 1.0);
	glVertex3f(-fSize, fSize, 0.0);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(fSize, fSize, 0.0);

	glTexCoord2f(1.0, 0.0);
	glVertex3f(fSize, -fSize, 0.0);

	glEnd();
	glutSwapBuffers();
}

void MyKeyboard(unsigned char k, int x, int y) {
	if (k == 'x') {
		exit(0);
	}
	else if (k == 'c') {
		color = 1;

		glutPostRedisplay();
	}
}

int main() {
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 800);
	glutCreateWindow("openGL Sample Program");
	MyInit();
	glutDisplayFunc(MyDisplay);
	glutKeyboardFunc(MyKeyboard);
	glutMainLoop();
	return 0;
}
