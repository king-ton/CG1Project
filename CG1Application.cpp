#include <stdlib.h>         // for rand()
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>           // for time() in srand()

#include "CG1Helper.h"
#include "CGContext.h"
#include "CGImageFile.h"
#include "CGMath.h"
#include "CG1Application_renderSphere.h"
#include "CGQuadric.h"
#include "TestDataSet.h"
#include "CGLightSource.h"


//---------------------------------------------------------------------------
// FRAMEBUFFER-SIZE
//---------------------------------------------------------------------------
#define FRAME_WIDTH  500	// Framebuffer width.
#define FRAME_HEIGHT 300 	// Framebuffer height.
#define FRAME_SCALE  1		// Integer scaling factors (zoom).

//---------------------------------------------------------------------------
// STRUCTS
//---------------------------------------------------------------------------
typedef struct
{
	int paddel_size;
	int paddel_pos;
	int points;
} Player;

typedef struct
{
	// -1 nach links, 1 nach rechts, 0 keine Rotation
	int rotate;

	CGMatrix4x4 projectMatrix;
	CGMatrix4x4 viewMatrix;
} Camera;

typedef struct
{
	int dx;
	int dz;
	int x;
	int z;

	CGQuadric disk;
	CGQuadric cylinder;
} Puk;

typedef struct {
	float width;
	float height;

	float color_ground[4];
	float color_border[4];

	CGQuadric ground;
	CGQuadric border;
} PlayGround;

//---------------------------------------------------------------------------
// GLOBALE VARIABLEN
//---------------------------------------------------------------------------
CGContext *ourContext;

Player player1 = { 6, 23, 0 };
Player player2 = { 6, 23, 0 };
Puk puk = { 0, 0, 0, 0 };
Camera camera = { 0 };
PlayGround playGround;

//---------------------------------------------------------------------------
// Farb-Wert
//---------------------------------------------------------------------------
float shininess = 32.0f;
float	rgbaWhite10[4] = { 1, 1, 1, 1 },
		rgbaWhite01[4] = { 0.1, 0.1, 0.1, 1.0 },
		rgbaWhite05[4] = { 0.5, 0.5, 0.5, 1.0 },
		rgbaWhite005[4] = { 0.05f, 0.05f, 0.05f, 1.0 },
		rgbaGreen[4] = { 0, 1, 0, 1.0 },
		rgbaRed[4] = { 1, 0, 0, 1 },
		rgbaYellow[4] = { 1, 1, 0, 1 },
		rgbaOrange[4] = { 1, 0.647F, 0, 1.0 },
		rgbaBlack[4] = { 0, 0, 0, 1 };


//---------------------------------------------------------------------------
// Erstellt die View-Matrix
//---------------------------------------------------------------------------
CGMatrix4x4 cguLookAt(	float eyeX,		float eyeY,		float eyeZ,
	float centerX,	float centerY,	float centerZ,
	float upX,		float upY,		float upZ)
{
	CGMatrix4x4 V;
	CGVec4 f, s, u, up;

	f.set(centerX - eyeX, centerY - eyeY, centerZ - eyeZ, 0.0F);
	f = CGMath::normalize(f);

	up.set(upX, upY, upZ, 0);
	s = CGMath::normalize(CGMath::cross(f, up));

	u = CGMath::cross(s, f);

	f = CGMath::scale(f, -1);

	float R[16] = { s[X],	s[Y],	s[Z],	0,
					u[X],	u[Y],	u[Z],	0,
					f[X],	f[Y],	f[Z],	0,
					0,		0,		0,		1 };
	V.setFloatsFromRowMajor(R);
	V = V * CGMatrix4x4::getTranslationMatrix((-1)*eyeX, (-1)*eyeY, (-1)*eyeZ);
	return V;
}

//---------------------------------------------------------------------------
// Zeichnet das 3D-Modell
//---------------------------------------------------------------------------
void renderQuadric(CGQuadric &quadric)
{
	ourContext->cgVertexAttribPointer(CG_POSITION_ATTRIBUTE, quadric.getPositionArray());
	ourContext->cgVertexAttribPointer(CG_NORMAL_ATTRIBUTE, quadric.getNormalArray());
	ourContext->cgVertexAttribPointer(CG_COLOR_ATTRIBUTE, quadric.getColorArray());
	ourContext->cgVertexAttribPointer(CG_TEXCOORD_ATTRIBUTE, quadric.getTexCoordArray());
	ourContext->cgDrawArrays(GL_TRIANGLES, 0, quadric.getVertexCount());
	ourContext->cgVertexAttribPointer(CG_POSITION_ATTRIBUTE, NULL);
	ourContext->cgVertexAttribPointer(CG_NORMAL_ATTRIBUTE, NULL);
	ourContext->cgVertexAttribPointer(CG_COLOR_ATTRIBUTE, NULL);
	ourContext->cgVertexAttribPointer(CG_TEXCOORD_ATTRIBUTE, NULL);
}

//---------------------------------------------------------------------------
// KAMERA
//---------------------------------------------------------------------------
#pragma region
//---------------------------------------------------------------------------
// Setzt die Kamera beim Start
//---------------------------------------------------------------------------
void camera_init()
{
	camera.projectMatrix = CGMatrix4x4::getFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 50.0f);
	camera.viewMatrix = cguLookAt(0, 10, 10, 0, 0, 0, 0, 1, 0);
}

void drawCamera()
{
	camera.viewMatrix = camera.viewMatrix * CGMatrix4x4::getRotationMatrixY(1);
}

#pragma endregion

//---------------------------------------------------------------------------
// LICHTQUELLE
//---------------------------------------------------------------------------
#pragma region
//---------------------------------------------------------------------------
// Zeichnet Lichtquelle
//---------------------------------------------------------------------------
void drawLight()
{
	ourContext->cgUniform4fv(CG_ULOC_LIGHT0_AMBIENT, 1, rgbaWhite01);
	ourContext->cgUniform4fv(CG_ULOC_LIGHT0_DIFFUSE, 1, rgbaWhite05);
	ourContext->cgUniform4fv(CG_ULOC_LIGHT0_SPECULAR, 1, rgbaWhite05);
}
#pragma endregion

//---------------------------------------------------------------------------
// PLAYGROUND
//---------------------------------------------------------------------------
#pragma region
//---------------------------------------------------------------------------
// Setzt den Spiel-Grund beim Start
//---------------------------------------------------------------------------
void ground_init(float width, float height, float* color_ground, float* color_border)
{
	playGround.width = width; playGround.height = height;

	playGround.ground.setStandardColor(color_ground[0], color_ground[1], color_ground[2]);
	playGround.ground.createBox(width, 1, height);

	playGround.border.setStandardColor(color_border[0], color_border[1], color_border[2]);
	playGround.border.createBox();
}

//---------------------------------------------------------------------------
// Zeichnet Spiel-Grund
//---------------------------------------------------------------------------
void drawGround()
{
	// GROUND
	CGMatrix4x4 modelviewT = camera.viewMatrix;
	modelviewT = modelviewT * CGMatrix4x4::getTranslationMatrix(0, -0.1F, 0);
	modelviewT = modelviewT * CGMatrix4x4::getScaleMatrix(playGround.width, 0.1F, playGround.height);
	float mv[16]; modelviewT.getFloatsToColMajor(mv);
	ourContext->cgUniformMatrix4fv(CG_ULOC_MODELVIEW_MATRIX, 1, false, mv);
	ourContext->cgUniformMatrix4fv(CG_ULOC_NORMAL_MATRIX, 1, false, mv);
	renderQuadric(playGround.ground);

	// BORDER TOP
	modelviewT = camera.viewMatrix;
	modelviewT = modelviewT * CGMatrix4x4::getTranslationMatrix(0, 0, (-1) * playGround.height);
	modelviewT = modelviewT * CGMatrix4x4::getScaleMatrix(playGround.width, 0.4F, 0.05F);
	mv[16]; modelviewT.getFloatsToColMajor(mv);
	ourContext->cgUniformMatrix4fv(CG_ULOC_MODELVIEW_MATRIX, 1, false, mv);
	ourContext->cgUniformMatrix4fv(CG_ULOC_NORMAL_MATRIX, 1, false, mv);
	renderQuadric(playGround.border);

	// BORDER BOTTOM
	modelviewT = camera.viewMatrix;
	modelviewT = modelviewT * CGMatrix4x4::getTranslationMatrix(0, 0, playGround.height);
	modelviewT = modelviewT * CGMatrix4x4::getScaleMatrix(playGround.width, 0.4F, 0.05F);
	mv[16]; modelviewT.getFloatsToColMajor(mv);
	ourContext->cgUniformMatrix4fv(CG_ULOC_MODELVIEW_MATRIX, 1, false, mv);
	ourContext->cgUniformMatrix4fv(CG_ULOC_NORMAL_MATRIX, 1, false, mv);
	renderQuadric(playGround.border);
}
#pragma endregion

//---------------------------------------------------------------------------
// PUK
//---------------------------------------------------------------------------
#pragma region
//---------------------------------------------------------------------------
// Setzt den Puk beim Start
//---------------------------------------------------------------------------
void puk_init(float* color)
{
	puk.cylinder.setStandardColor(color[0], color[1], color[2]);
	puk.cylinder.createCylinder(15,1);

	puk.disk.setStandardColor(color[0], color[1], color[2]);
	puk.disk.createDisk(15, 2);
}

void drawPuk()
{
	//if (CG1Helper::isKeyPressed(CG_KEY_DOWN)) puk.z++;
	//if (CG1Helper::isKeyPressed(CG_KEY_UP)) puk.z--;
	//if (CG1Helper::isKeyPressed(CG_KEY_RIGHT)) puk.x++;
	//if (CG1Helper::isKeyPressed(CG_KEY_LEFT)) puk.x--;

	CGMatrix4x4 modelviewT = camera.viewMatrix;
	modelviewT = modelviewT * CGMatrix4x4::getTranslationMatrix(puk.x, 0, puk.z);
	modelviewT = modelviewT * CGMatrix4x4::getScaleMatrix(0.5F, 0.5F, 0.5F);
	modelviewT = modelviewT * CGMatrix4x4::getRotationMatrixX(-90);
	float mv[16]; modelviewT.getFloatsToColMajor(mv);
	ourContext->cgUniformMatrix4fv(CG_ULOC_MODELVIEW_MATRIX, 1, false, mv);
	ourContext->cgUniformMatrix4fv(CG_ULOC_NORMAL_MATRIX, 1, false, mv);
	renderQuadric(puk.cylinder);


	modelviewT = modelviewT * CGMatrix4x4::getTranslationMatrix(0, 0, 0.5F);
	modelviewT.getFloatsToColMajor(mv);
	ourContext->cgUniformMatrix4fv(CG_ULOC_MODELVIEW_MATRIX, 1, false, mv);
	ourContext->cgUniformMatrix4fv(CG_ULOC_NORMAL_MATRIX, 1, false, mv);
	renderQuadric(puk.disk);
}
#pragma endregion

void drawFrame()
{
	ourContext->cgClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	ourContext->cgClear(CG_COLOR_BUFFER_BIT | CG_DEPTH_BUFFER_BIT);
	ourContext->cgEnable(CG_DEPTH_TEST);
	ourContext->cgEnable(CG_CULL_FACE);
	ourContext->cgEnable(CG_BLEND);
	ourContext->cgEnable(CG_USE_MATERIAL_COLOR);

	float proj[16]; camera.projectMatrix.getFloatsToColMajor(proj);
	ourContext->cgUniformMatrix4fv(CG_ULOC_PROJECTION_MATRIX, 1, false, proj);

	//static float anim = 0.0; anim+=0.1;
	//float eyeX = cos(anim)*10.0f, eyeY = 10.0f, eyeZ = sin(anim)*10.0f;
	//float eyeX = 0, eyeY = 10, eyeZ = 10;





	ourContext->cgUniform4fv(CG_ULOC_MATERIAL_EMISSION, 1, rgbaBlack);
	ourContext->cgUniform4fv(CG_ULOC_MATERIAL_SPECULAR, 1, rgbaWhite10);
	ourContext->cgUniform1fv(CG_ULOC_MATERIAL_SHININESS, 1, &shininess);
	ourContext->cgUniform4fv(CG_ULOC_MATERIAL_AMBIENT, 1, rgbaWhite005);
	ourContext->cgUniform4fv(CG_ULOC_SCENE_AMBIENT, 1, rgbaWhite005);

	drawCamera();
	drawLight();
	drawGround();
	drawPuk();

	//// set modelview matrix and normal matrix
	float mv[16], nm[16];
	////CGMatrix4x4 viewT = cguLookAt(0, 2, 2, 0, 0, 0, 0, 1, 0);

	////static float anim = 0.0f; anim += 0.01f;
	////float eyeX = cos(anim)*11.0f, eyeY = 11.0f, eyeZ = sin(anim)*11.0f;
	////eyeX = 0; eyeY = 11; eyeZ = 0;
	////CGMatrix4x4 viewT = cguLookAt(eyeX, eyeY, eyeZ, 0, 0, 0, 0, 1, 0);


	//CGMatrix4x4 modelT = CGMatrix4x4::getRotationMatrixY(0);
	//CGMatrix4x4 modelviewT = viewT;
	//CGMatrix4x4 modelviewT = viewT * modelT;
	//modelviewT.getFloatsToColMajor(mv);
	//ourContext->cgUniformMatrix4fv(CG_ULOC_MODELVIEW_MATRIX, 1, false, mv);
	//modelviewT.getFloatsToColMajor(nm); // Get normal matrix (column major!).
	//nm[12] = nm[13] = nm[14] = 0.0f; // Reduce to 3x3 matrix (column major!).
	//nm[3] = nm[7] = nm[11] = 0.0f;
	//nm[15] = 1.0f;
	//CGMatrix4x4 normalMatrix; normalMatrix.setFloatsFromColMajor(nm);
	//normalMatrix.invert(); normalMatrix.transpose();
	//normalMatrix.getFloatsToColMajor(nm); // Get the correct values of (MV.3x3)^-1^T
	//ourContext->cgUniformMatrix4fv(CG_ULOC_NORMAL_MATRIX, 1, false, nm);

	//float vertices4quad[6 * 3] = { -2, 0, -1, -2, 0, 1, 2, 0, -1, 2, 0, -1, -2, 0, 1, 2, 0, 1 };
	//float colors4quad[6 * 4] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	//ourContext->cgVertexAttribPointer(CG_POSITION_ATTRIBUTE, vertices4quad);
	//ourContext->cgVertexAttribPointer(CG_COLOR_ATTRIBUTE, colors4quad);

	// ourContext->cgDrawArrays(GL_TRIANGLES, 0, 2 * 3);

	//ourContext->cgUniform4fv(CG_ULOC_MATERIAL_AMBIENT, 1, rgbaRed);
	//ourContext->cgUniform4fv(CG_ULOC_MATERIAL_DIFFUSE, 1, rgbaRed);


	//CGQuadric box; box.createBox();

	//CGMatrix4x4 test = modelviewT;

	//test = test * CGMatrix4x4::getTranslationMatrix(0,0,-1);
	//test = test * CGMatrix4x4::getScaleMatrix(2.0F, 0.1F, 0.05F);
	//test.getFloatsToColMajor(mv);
	//ourContext->cgUniformMatrix4fv(CG_ULOC_MODELVIEW_MATRIX, 1, false, mv);
	//ourContext->cgUniformMatrix4fv(CG_ULOC_NORMAL_MATRIX, 1, false, mv);

	//renderQuadric(box);

	//test = modelviewT;
	//test = test * CGMatrix4x4::getTranslationMatrix(0, 0, 1);
	//test = test * CGMatrix4x4::getScaleMatrix(2.0F, 0.1F, 0.05F);
	//test.getFloatsToColMajor(mv);
	//ourContext->cgUniformMatrix4fv(CG_ULOC_MODELVIEW_MATRIX, 1, false, mv);
	//ourContext->cgUniformMatrix4fv(CG_ULOC_NORMAL_MATRIX, 1, false, mv);

	// renderQuadric(box);

	// test = modelviewT;
	// test = test * CGMatrix4x4::getTranslationMatrix(2.0F, 0.1F, 0);
	// test = test * CGMatrix4x4::getScaleMatrix(0.05F, 0.05F, 0.2F);
	// test.getFloatsToColMajor(mv);
	// ourContext->cgUniformMatrix4fv(CG_ULOC_MODELVIEW_MATRIX, 1, false, mv);
	// ourContext->cgUniformMatrix4fv(CG_ULOC_NORMAL_MATRIX, 1, false, mv);

	// renderQuadric(box);

	// test = modelviewT;
	// test = test * CGMatrix4x4::getTranslationMatrix(-2.0F, 0.1F, 0);
	// test = test * CGMatrix4x4::getScaleMatrix(0.05F, 0.05F, 0.2F);
	// test.getFloatsToColMajor(mv);
	// ourContext->cgUniformMatrix4fv(CG_ULOC_MODELVIEW_MATRIX, 1, false, mv);
	// ourContext->cgUniformMatrix4fv(CG_ULOC_NORMAL_MATRIX, 1, false, mv);

	// renderQuadric(box);

	// CGQuadric ball; ball.createIcoSphere(3);

	// test = modelviewT;
	// test = test * CGMatrix4x4::getTranslationMatrix(0, 0.2F, 0);
	// test.getFloatsToColMajor(mv);
	// ourContext->cgUniformMatrix4fv(CG_ULOC_MODELVIEW_MATRIX, 1, false, mv);
	// ourContext->cgUniformMatrix4fv(CG_ULOC_NORMAL_MATRIX, 1, false, mv);

	// renderQuadric(ball);
}

void programStep()
{
	drawFrame();
}

int main(int argc, char** argv)
{
	camera_init();
	ground_init(5, 4, rgbaGreen, rgbaOrange);
	puk_init(rgbaRed);

	CG1Helper::initApplication(ourContext, FRAME_WIDTH, FRAME_HEIGHT, FRAME_SCALE);
	CG1Helper::setProgramStep(programStep);
	CG1Helper::runApplication();

	return 0;
}