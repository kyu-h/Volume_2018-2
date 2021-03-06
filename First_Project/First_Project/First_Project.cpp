#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include <iostream>
#include <fstream>

#define WIDTH   512
#define HEIGHT  512

int color = 0;

unsigned char ImageBuf[WIDTH][HEIGHT] = { 0,255,0,255, 255,0,255,0, 0,255,0,255, 255,0,255,0 };
unsigned char MyTexture[WIDTH][HEIGHT][3];

unsigned char vol[225][256][256];

float alphaTable[256];
float colorTable[256];

void MakeAlphaTable(int a, int b) {
	for (int i = 0; i < a; i++) {
		alphaTable[i] = 0;
	}
	for (int i = b; i < 256; i++) {
		alphaTable[i] = 1;
	}
	for (int i = a; i < b; i++) {
		alphaTable[i] = (float)(i - a) / (b - a);
	}
}

void MakeColorTable(int a, int b) {
	for (int i = 0; i < a; i++) {
		colorTable[i] = 0;
	}
	for (int i = b; i < 256; i++) {
		colorTable[i] = 1;
	}
	for (int i = a; i < b; i++) {
		colorTable[i] = (float)(i - a) / (b - a);
	}
}

class Vec3 { //벡터, 좌표를 계산하기 위한 
public:
	float x, y, z; //좌표
public:
	Vec3() {
		x = y = z = 0;
	}

	Vec3(float _x, float _y, float _z) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vec3 operator+ (Vec3 t) {
		Vec3 res;

		res.x = x + t.x;
		res.y = y + t.y;
		res.z = z + t.z;

		return res;
	}

	Vec3 operator- (Vec3 t) {
		Vec3 res(x - t.x, y - t.y, z - t.z);

		return res;
	}

	Vec3 operator* (float t) { //상수배
		Vec3 res(x*t, y*t, z*t);

		return res;
	}

	Vec3 Cross(Vec3 t) {
		Vec3 res(y * t.z - z * t.y, z * t.x - x * t.y, x * t.y - y * t.x);

		return res;
	}

	float Length() {
		return sqrt(x * x + y * y + z * z);
	}

	void Normalize() {
		float len = Length();
		if (len == 0) {
			//printf("Devide by zero ERROR");
			return;
		}
		x = x / len;
		y = y / len;
		z = z / len;
	}

	float Dot(Vec3 t) { //내적
		return x * t.x + y * t.y + z * t.z;
	}

	void Print() {
		printf("%f, %f, %f\n", x, y, z);
	}
};

Vec3 vEye;
Vec3 vDir, vU, vCross;

void FileRead() {
	std::ifstream myfile;
	myfile.open("bighead.den", std::ios::in | std::ios::binary);
	if (!myfile.is_open()) {
		std::cout << "file error";
	}
	myfile.read((char*)vol, 256 * 256 * 225);
	myfile.close();
}

unsigned char Sampling(float x, float y, float z) {
	int ix = (int)x;
	int iy = (int)y;
	int iz = (int)z;

	unsigned char v000 = vol[iz][iy][ix];
	unsigned char v001 = vol[iz][iy][ix + 1];
	unsigned char v010 = vol[iz][iy + 1][ix];
	unsigned char v011 = vol[iz][iy + 1][ix + 1];

	unsigned char v100 = vol[iz + 1][iy][ix];
	unsigned char v101 = vol[iz + 1][iy][ix + 1];
	unsigned char v110 = vol[iz + 1][iy + 1][ix];
	unsigned char v111 = vol[iz + 1][iy + 1][ix + 1];

	float wx = x - ix;
	float wy = y - iy;
	float wz = z - iz;

	unsigned char val =
		v000 * (1 - wz) * (1 - wy) * (1 - wx) +
		v001 * (1 - wz) * (1 - wy) * wx +
		v010 * (1 - wz)  * wy * (1 - wx) +
		v011 * (1 - wz) * wy * wx +
		v100 * wz * (1 - wy) * (1 - wx) +
		v101 * wz * (1 - wy) * wx +
		v110 * wz * wy * (1 - wx) +
		v111 * wz * wy * wx;

	return val;
}

Vec3 GetNormal(Vec3 vPos) {
	float dx = Sampling(vPos.x + 1, vPos.y, vPos.z) - Sampling(vPos.x - 1, vPos.y, vPos.z);
	float dy = Sampling(vPos.x, vPos.y + 1, vPos.z) - Sampling(vPos.x, vPos.y - 1, vPos.z);
	float dz = Sampling(vPos.x, vPos.y, vPos.z + 1) - Sampling(vPos.x, vPos.y, vPos.z - 1);

	return Vec3(dx, dy, dz);
}

#include <omp.h>

void FillMyTexture() {
	vCross = vCross * 0.5;
	vU = vU * 0.5;

#pragma omp parallel for
	for (int v = 0; v < WIDTH; v++) { //v가 width로 가로, u가 세로
		for (int u = 0; u < HEIGHT; u++) {
			//*****1 week****//
			//unsigned char Intensity = ImageBuf[s][t];
			//rand() % 255;
			//*****1 week****//

			/*
			unsigned char Intensity = vol[110][s][t]; //110번째 slice에서 직접 값을 추출

			for (int x = 0; x < 225; x++) {
			if (Intensity < vol[x][s][t]) {
			Intensity = vol[x][s][t];
			}
			}
			*/

			/*Vec3 vCross = UP * Dir;
			Vec3 vU = Dir * vCross;*/

			Vec3 vRayStart = vEye + vU * (HEIGHT/2 - u) + vCross * (WIDTH/2 - v); //pseudo code, 

			float t1, t2;

			//x축
			t1 = -vRayStart.x / vDir.x;
			t2 = (255 - vRayStart.x) / vDir.x;

			float xm = __min(t1, t2);
			float xM = __max(t1, t2);

			//y축
			t1 = -vRayStart.y / vDir.y;
			t2 = (255 - vRayStart.y) / vDir.y;

			float ym = __min(t1, t2);
			float yM = __max(t1, t2);

			//z축
			t1 = -vRayStart.z / vDir.z;
			t2 = (224 - vRayStart.z) / vDir.z;

			float zm = __min(t1, t2);
			float zM = __max(t1, t2);

			float tm = __max(__max(xm, ym), zm);
			float tM = __min(__min(xM, yM), zM);

			unsigned char max_value = 0;
			float alpha = 0, color = 0; // 누적 0~1 범위
			for (float t = tm; t < tM; t+=1.0) {
				Vec3 vPos = vRayStart + vDir * t;

				int iz = (int)(vPos.z);
				int iy = (int)(vPos.y);
				int ix = (int)(vPos.x);

				if (1 <= iz && iz + 1 < 224 &&
					1 <= iy && iy + 1 < 255 &&
					1 <= ix && ix + 1 < 255) {

					//******************************해당 아래 코드는 알았으면 좋겠다*************************//
					unsigned char val = Sampling(vPos.x, vPos.y, vPos.z);
					//******************************해당 위에 코드는 알았으면 좋겠다*************************//
					float c_alpha = alphaTable[val];
					if (c_alpha == 0)
						continue;

					Vec3 normal = GetNormal(vPos);
					normal.Normalize();
					//unsigned char val = vol[iz][iy][ix];

					//alpha blending
					
					float c_color = colorTable[val];

					float NL = fabs(normal.Dot(vDir));
					float NH = fabs(normal.Dot(vDir));
					c_color = 0.3 * c_color + 0.6 * c_color * NL + 0.1 * c_color + pow(NH, 20);

					if (c_color > 1)
						c_color = 1;

					//계산 순서 주의 color, alpha의 순서가 바뀌면 alpha 값이 바뀌기 때문에 영향을 줌
					color = color + (1 - alpha) * c_alpha * c_color;
					alpha = alpha + (1 - alpha) * c_alpha;
					if (alpha == 1.0)
						break;

					//max_value = __max(max_value, val); //핵심 코드!!
				}

				//vol[vPos.z][vPos.y][vPos.x] 값을 이용해서 비교
			}

			//unsigned char Intensity_Y = vol[v][110][u];

			/*for (int x = 0; x < 256; x++) {
			if (Intensity_Y < vol[v][x][u]) {
			Intensity_Y = vol[v][x][u];
			}
			}*/

			MyTexture[v][u][0] = MyTexture[v][u][1] = MyTexture[v][u][2] = (unsigned char)(255 * color); //max_value;

																										 //MyTexture[s][t][1] = MyTexture[s][t][2] = 0; // 순서R, G, B
		}
	}
}
#include <time.h>
void MyInit() {
	glClearColor(0.0, 0.0, 0.0, 0.0);

	time_t start = clock();
	FillMyTexture();
	time_t end = clock();

	float ms = (float)(end - start) / (CLOCKS_PER_SEC) * 1000;
	printf("working time : %f\n", ms);

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
	/*
	//밑에 Make~~ 함수들을 scanf 하기 위해..//
	int a, b;
	scanf_s("%d %d", &a, &b);
	*/

	MakeAlphaTable(20, 150);
	MakeColorTable(20, 90);

	Vec3 Eye, At(128, 128, 112), UP(0, 1, 0);

	scanf_s("%f %f %f", &vEye.x, &vEye.y, &vEye.z);

	/*
	//vEye.x,y,z에 값을 할당하지 않고 수동으로 값을 넣을 때
	vEye.x = 128;
	vEye.y = 1;
	vEye.z = 111;
	*/

	vDir = At - vEye;
	vDir.Normalize();
	vCross = UP.Cross(vDir);
	vCross.Normalize();
	vU = vDir.Cross(vCross);
	vU.Normalize();

	vDir.Print();
	vCross.Print();
	vU.Print();

	/*
	Dir.Normalize();
	Vec3 Cross = UP.Cross(Dir);
	Cross.Normalize();
	Vec3 U = Dir.Cross(Cross);
	U.Normalize();

	Dir.Print();
	Cross.Print();
	U.Print();

	Vec3 a(1, 2, 3);
	Vec3 b(4, 5, 6);
	Vec3 c = a + b;
	c.Print();

	Vec3 d(1, 2, 3);
	Vec3 e(4, 5, 6);
	Vec3 f = d + e;
	f.Print();
	*/

	FileRead();
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 800);
	glutCreateWindow("openGL Sample Program");
	MyInit();
	glutDisplayFunc(MyDisplay);
	glutKeyboardFunc(MyKeyboard);
	glutMainLoop();

	return 0;
}