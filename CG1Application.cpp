#include <stdlib.h>         // for rand()
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>           // for time() in srand()

#include "CG1Helper.h"

#include "CGCamera.h"
#include "CGContext.h"
#include "CGImageFile.h"
#include "CGLightSource.h"
#include "CGMath.h"
#include "CGPlayer.h"

#include "CGPuk.h"
#include "CGQuadric.h"

//---------------------------------------------------------------------------
// FRAMEBUFFER-SIZE
//---------------------------------------------------------------------------
#define FRAME_WIDTH  500	// Framebuffer width.
#define FRAME_HEIGHT 300 	// Framebuffer height.
#define FRAME_SCALE  1		// Integer scaling factors (zoom).

//---------------------------------------------------------------------------
// GLOBALE VARIABLEN
//---------------------------------------------------------------------------
CGContext *ourContext;

CGPlayer *player1, *player2;
CGPuk *puk;
CGCamera *camera;
CGPlayGround *pg;
CGQuadric box;

//---------------------------------------------------------------------------
// Farb-Werte
//---------------------------------------------------------------------------
float shininess = 32.0f;
float rgbaWhite10[4] = { 1, 1, 1, 1 };
float rgbaWhite01[4] = { 0.1F, 0.1F, 0.1F, 1.0F };
float rgbaWhite05[4] = { 0.5F, 0.5F, 0.5F, 1.0F };
float rgbaWhite005[4] = { 0.05f, 0.05f, 0.05f, 1.0 };
float rgbaBlack[4] = { 0, 0, 0, 1 };

float rgbaBlue[4] = { 0, 0, 1, 1 };
float rgbaGreen[4] = { 0, 1.0F, 0, 1.0F };
float rgbaOrange[4] = { 1.0F, 0.647F, 0, 1.0F };
float rgbaRed[4] = { 1, 0, 0, 1.0F };
float rgbaYellow[4] = { 1, 1, 0, 1 };

//---------------------------------------------------------------------------
// Gibt die Tastenbelegungen aus
//---------------------------------------------------------------------------
void help_showKeys()
{
	printf("\n---------------\n     Hilfe     \n---------------\n");
	printf("\nZeige Hilfe:\t\tF1 ( so schlau warst du schonmal ;-) )\nKamera-Modus\nwechseln (2D/3D):\tc\n");
	printf("\nSpieler 1:\n----------\nNach oben:\t\tPFEILTASTE OBEN\nNach unten:\t\tPFEILTASTE UNTEN\nKI aktivieren:\t\tPFEILTASTE RECHTS\nSpieler hat Puk:\t1 ( Nur vor Spielbeginn m\224glich! )\nPaddel-Gr\224\341e \204ndern:\tBILD AUF/BILD AB\n");
	printf("\nSpieler 2:\n----------\nNach oben:\t\tW\nNach unten:\t\tS\nKI aktivieren:\t\tA\nSpieler hat Puk:\t2 ( Nur vor Spielbeginn m\224glich! )\nPaddel-Gr\224\341e \204ndern:\tq/e\n");
	printf("\nKamera-Steuerung: ( Nur 3D-Modus )\n-----------------\nNach oben:\t\ti\nNach unten:\t\tk\nNach links:\t\tj\nNach rechts:\t\tl\n");
}

//---------------------------------------------------------------------------
// Zeichnet eine Zahl an die angegebene Stelle
//---------------------------------------------------------------------------
void help_draw_number(float startX, int number)
{
	float legth = 1.0F;
	float vertex[6];
	float color[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };

	CGMatrix4x4 modelviewT;
	modelviewT = camera->getViewMatrix();
	float mv[16]; modelviewT.getFloatsToColMajor(mv);
	ourContext->cgUniformMatrix4fv(CG_ULOC_MODELVIEW_MATRIX, 1, false, mv);

	//		  0
	//		 ---
	//	  3 | 1 | 5
	//		 ---
	//	  4 |   | 6
	//		 ---
	//		  2

	// Segment 0, 1, 2
	for (int i = 0; i < 3; i++) {
		if ((i == 0 && number != 1 && number != 4) ||
			(i == 1 && number != 0 && number != 1 && number != 7) ||
			(i == 2 && number != 1 && number != 4 && number != 7)) {
			vertex[0] = startX;		vertex[1] = 0; vertex[2] = 7 + i;
			vertex[3] = startX + 1; vertex[4] = 0; vertex[5] = 7 + i;
			ourContext->cgVertexAttribPointer(CG_POSITION_ATTRIBUTE, vertex);
			ourContext->cgDrawArrays(CG_LINES, 0, 2);
			ourContext->cgVertexAttribPointer(CG_POSITION_ATTRIBUTE, NULL);
		}
	}

	// Segment 3, 4, 5, 6
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++) {
			if ((i == 0 && j == 0 && (number != 1 && number != 2 && number != 3 && number != 7)) ||
				(i == 0 && j == 1 && (number == 0 || number == 2 || number == 6 || number == 8)) ||
				(i == 1 && j == 0 && (number != 5 && number != 6)) ||
				(i == 1 && j == 1 && (number != 2))) {
				vertex[0] = startX + i;	vertex[1] = 0; vertex[2] = 7 + j;
				vertex[3] = startX + i; vertex[4] = 0; vertex[5] = 7 + j + 1;
				ourContext->cgVertexAttribPointer(CG_POSITION_ATTRIBUTE, vertex);
				ourContext->cgDrawArrays(CG_LINES, 0, 2);
				ourContext->cgVertexAttribPointer(CG_POSITION_ATTRIBUTE, NULL);
			}
		}
}

//---------------------------------------------------------------------------
// KAMERA
//---------------------------------------------------------------------------
#pragma region
//---------------------------------------------------------------------------
// Setzt die Kamera beim Start
//---------------------------------------------------------------------------
void drawCamera()
{
	float proj[16]; camera->getProjectMatrix().getFloatsToColMajor(proj);
	ourContext->cgUniformMatrix4fv(CG_ULOC_PROJECTION_MATRIX, 1, false, proj);

	camera->draw();
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
	ourContext->cgUniform4fv(CG_ULOC_LIGHT0_AMBIENT, 1, rgbaWhite005);
	ourContext->cgUniform4fv(CG_ULOC_LIGHT0_DIFFUSE, 1, rgbaWhite005);
	ourContext->cgUniform4fv(CG_ULOC_LIGHT0_SPECULAR, 1, rgbaBlack);

	CGLightSource lightSource;

	CGVec4 lightPos; lightPos.set(puk->getX(), 3, puk->getZ(), 1);
	lightSource.setColorAmbient(0.1, 0.1, 0.1, 1.0);
	lightSource.setColorDiffuse(0.5, 0.5, 0.5, 1.0);
	lightSource.setColorSpecular(0.5, 0.5, 0.5, 1.0);
	lightSource.setPosition((camera->getViewMatrix() * lightPos).elements);
	lightSource.setConstantAttenuation(1.0F);
	lightSource.setLinearAttenuation(0.0F);
	lightSource.setQuadraticAttenuation(0.0F);

	// Set only the current light source.
	lightSource.setupUniforms(ourContext);

	// set materials for objects
	ourContext->cgUniform4fv(CG_ULOC_MATERIAL_AMBIENT, 1, rgbaWhite01);
	ourContext->cgUniform4fv(CG_ULOC_MATERIAL_SPECULAR, 1, rgbaWhite10);
	ourContext->cgUniform1fv(CG_ULOC_MATERIAL_SHININESS, 1, &shininess);
	ourContext->cgUniform4fv(CG_ULOC_MATERIAL_EMISSION, 1, rgbaBlack);

	// Setyp scene ambient
	ourContext->cgUniform4fv(CG_ULOC_SCENE_AMBIENT, 1, rgbaWhite10);
}
#pragma endregion

//---------------------------------------------------------------------------
// PLAYGROUND
//---------------------------------------------------------------------------
#pragma region
//---------------------------------------------------------------------------
// Zeichnet Spielfläche
//---------------------------------------------------------------------------
void drawGround()
{
	pg->draw(ourContext, &camera->getViewMatrix());
}
#pragma endregion

//---------------------------------------------------------------------------
// PUK
//---------------------------------------------------------------------------
#pragma region
void drawPuk()
{
	// Wenn sich Puk nicht bewegt, setze ihn auf eine Seite des Spielers
	if (!puk->isMove()) {
		if (player1->hasPuk()) {
			puk->setX(pg->getWidth() - puk->getSize());
			puk->setZ(player1->getPaddelPos());
		}
		else if (player2->hasPuk()) {
			puk->setX(-(pg->getWidth() - puk->getSize()));
			puk->setZ(player2->getPaddelPos());
		}
	}

	puk->draw(ourContext, &camera->getViewMatrix());
}
#pragma endregion

//---------------------------------------------------------------------------
// SCHLÄGER
//---------------------------------------------------------------------------
#pragma region
void drawPaddle()
{
	player1->draw(ourContext, &camera->getViewMatrix(), pg->getWidth() + 0.1F);
	player2->draw(ourContext, &camera->getViewMatrix(), -(pg->getWidth() + 0.1F));
}
#pragma endregion

//---------------------------------------------------------------------------
// SPIELSTAND
//---------------------------------------------------------------------------
#pragma region
void drawScore()
{
	ourContext->cgUniform4fv(CG_ULOC_MATERIAL_AMBIENT, 1, rgbaBlack);
	ourContext->cgUniform4fv(CG_ULOC_MATERIAL_DIFFUSE, 1, rgbaBlack);
	ourContext->cgUniform4fv(CG_ULOC_MATERIAL_SPECULAR, 1, rgbaBlack);
	ourContext->cgUniform4fv(CG_ULOC_MATERIAL_EMISSION, 1, rgbaWhite05);


	float vertex[6];
	float color[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };

	int number = player2->getPoints();

	for (int i = 0; number >= 0; i++) {
		help_draw_number(-(i*1.5F + 2), number % 10);
		number = (int)(number * 0.1F);
		if (number == 0)
			number = -1;
	}

	number = player1->getPoints();
	int count;
	for (count = 0; number >= 0; count++) {
		number = (int)(number * 0.1F);
		if (number == 0)
			number = -1;
	}

	number = player1->getPoints();
	for (count--; count >= 0; count--) {
		help_draw_number(count*1.5F + 1, number % 10);
		number = (int)(number * 0.1F);
	}

	CGMatrix4x4 modelviewT;
	modelviewT = camera->getViewMatrix();
	float mv[16]; modelviewT.getFloatsToColMajor(mv);
	ourContext->cgUniformMatrix4fv(CG_ULOC_MODELVIEW_MATRIX, 1, false, mv);
	ourContext->cgUniformMatrix4fv(CG_ULOC_NORMAL_MATRIX, 1, false, mv);

	vertex[0] = 0;	vertex[1] = 0; vertex[2] = 7.25F;
	vertex[3] = 0; vertex[4] = 0; vertex[5] = 7.75F;
	ourContext->cgVertexAttribPointer(CG_POSITION_ATTRIBUTE, vertex);
	ourContext->cgDrawArrays(CG_LINES, 0, 2);

	vertex[0] = 0;	vertex[1] = 0; vertex[2] = 8.25F;
	vertex[3] = 0; vertex[4] = 0; vertex[5] = 8.75F;
	ourContext->cgVertexAttribPointer(CG_POSITION_ATTRIBUTE, vertex);
	ourContext->cgDrawArrays(CG_LINES, 0, 2);

}
#pragma endregion

void drawFrame()
{
	ourContext->cgClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	ourContext->cgClear(CG_COLOR_BUFFER_BIT | CG_DEPTH_BUFFER_BIT);
	ourContext->cgEnable(CG_DEPTH_TEST);
	ourContext->cgDisable(CG_CULL_FACE);
	ourContext->cgDisable(CG_BLEND);

	drawCamera();
	drawLight();

	drawGround();
	drawPuk();
	drawPaddle();

	CGMatrix4x4 modelviewT = camera->getViewMatrix() * CGMatrix4x4::getRotationMatrixY(45);
	CGQuadric::renderQuadric(box, ourContext, modelviewT, rgbaOrange);

	drawScore();


}

void processUserInput()
{
	// Zeige alle Tastenbelegungen
	if (CG1Helper::isKeyReleased(GLUT_KEY_F1)) help_showKeys();

	// Tasten Spieler 1
	if (!player1->isKi()) {
		if (CG1Helper::isKeyPressed(CG_KEY_UP))
			player1->move(pg, -0.2F);
		if (CG1Helper::isKeyPressed(CG_KEY_DOWN))
			player1->move(pg, 0.2F);
		if (CG1Helper::isKeyReleased(CG_KEY_LEFT) && !puk->isMove()) {
			puk->setDX(-(rand() % 10 + 1.0F));
			if (rand() % 2 == 1)
				puk->setDZ((rand() % 10 + 1.0F) * 0.3F);
			else
				puk->setDZ(-(rand() % 10 + 1.0F) * 0.3F);

			float length = 20 * sqrt(pow(puk->getDX(), 2) + pow(puk->getDZ(), 2));
			puk->setDX(puk->getDX() / length); puk->setDZ(puk->getDZ() / length);
		}
		if (CG1Helper::isKeyReleased('1') && player1->getPoints() == 0 && player2->getPoints() == 0) {
			player2->hasPuk(false); player1->hasPuk(true);
		}
	}
	if (CG1Helper::isKeyReleased(CG_KEY_RIGHT)) {
		if (!player2->isKi()) {
			player1->isKi(!player1->isKi());
			if (player1->isKi()) {
				player1->hasPuk(false); player2->hasPuk(true);
				printf("Spieler1: KI\n\n");
			}
			else
				printf("Spieler1: Mensch\n\n");
		}
		else
			printf("Maximal 1 KI m\224glich!\n\n");
	}
	if (CG1Helper::isKeyReleased(CG_KEY_PAGE_UP) && player1->getPaddelSize() + 0.5F < pg->getHeight()) player1->setPaddelSize(player1->getPaddelSize() + 0.5F);
	if (CG1Helper::isKeyReleased(CG_KEY_PAGE_DOWN) && player1->getPaddelSize() - 0.5F > 0) player1->setPaddelSize(player1->getPaddelSize() - 0.5F);


	// Tasten Spieler 2
	if (!player2->isKi()) {
		if (CG1Helper::isKeyPressed('w'))
			player2->move(pg, -0.2F);
		if (CG1Helper::isKeyPressed('s'))
			player2->move(pg, 0.2F);
		if (CG1Helper::isKeyReleased('d') && !puk->isMove()) {
			puk->setDX(rand() % 10 + 1.0F);
			if (rand() % 2 == 1)
				puk->setDZ((rand() % 10 + 1.0F) * 0.3F);
			else
				puk->setDZ(-(rand() % 10 + 1.0F) * 0.3F);

			float length = 20 * sqrt(pow(puk->getDX(), 2) + pow(puk->getDZ(), 2));
			puk->setDX(puk->getDX() / length); puk->setDZ(puk->getDZ() / length);
		}
		if (CG1Helper::isKeyReleased('2') && player1->getPoints() == 0 && player2->getPoints() == 0) {
			player1->hasPuk(false); player2->hasPuk(true);
		}
	}
	if (CG1Helper::isKeyReleased('a')) {
		if (!player1->isKi()) {
			player2->isKi(!player2->isKi());
			if (player2->isKi()) {
				player2->hasPuk(false); player1->hasPuk(true);
				printf("Spieler2: KI\n\n");
			}
			else
				printf("Spieler2: Mensch\n\n");
		}
		else
			printf("Maximal 1 KI m\224glich!\n\n");
	}
	if (CG1Helper::isKeyReleased('q') && player2->getPaddelSize() + 0.5F < pg->getHeight()) player2->setPaddelSize(player2->getPaddelSize() + 0.5F);
	if (CG1Helper::isKeyReleased('e') && player2->getPaddelSize() - 0.5F > 0) player2->setPaddelSize(player2->getPaddelSize() - 0.5F);

	// Kamera-Steuerung
	if (!camera->is2D()) {
		if (CG1Helper::isKeyPressed('j')) camera->setViewMatrix(camera->getViewMatrix() * CGMatrix4x4::getRotationMatrixY(1));
		if (CG1Helper::isKeyPressed('l')) camera->setViewMatrix(camera->getViewMatrix() * CGMatrix4x4::getRotationMatrixY(-1));
		if (CG1Helper::isKeyPressed('i')) camera->setViewMatrix(camera->getViewMatrix() *  CGMatrix4x4::getRotationMatrixX(1));
		if (CG1Helper::isKeyPressed('k')) camera->setViewMatrix(camera->getViewMatrix() * CGMatrix4x4::getRotationMatrixX(-1));
	}
	if (CG1Helper::isKeyReleased('c')) camera->is2D(!camera->is2D());
}

//---------------------------------------------------------------------------
// Simuliert die künstliche Intelligenz
//---------------------------------------------------------------------------
void processAI()
{
	if (player1->isKi()) {
		player2->hasPuk(true); player1->hasPuk(false);
		if (puk->getZ() > player1->getPaddelPos()){
			player1->move(pg, 0.12F);
		}
		else if (puk->getZ() < player1->getPaddelPos())
			player1->move(pg, -0.12F);
	}
	if (player2->isKi()) {
		player1->hasPuk(true); player2->hasPuk(false);
		if (puk->getZ() > player2->getPaddelPos()) {
			player2->move(pg, 0.12F);
		}
		else if (puk->getZ() < player2->getPaddelPos())
			player2->move(pg, -0.12F);
	}
}

//---------------------------------------------------------------------------
// Sorgt dafür, dass der Puk im Spielfeld bleibt
//---------------------------------------------------------------------------
void processPhysics()
{
	// Puk trifft Box
	if (abs(puk->getX() + puk->getDX()) - puk->getSize() + abs(puk->getZ() + puk->getDZ()) - puk->getSize() < sqrt(2)) {
		float temp = puk->getDX();
		if (puk->getZ() > 0) {
			if (puk->getX() > 0) {
				puk->setDX(-puk->getDZ());
				puk->setDZ(-temp);
			}
			else {
				puk->setDX(puk->getDZ());
				puk->setDZ(temp);
			}
		}
		else {
			if (puk->getX() > 0) {
				puk->setDX(puk->getDZ());
				puk->setDZ(temp);
			}
			else {
				puk->setDX(-puk->getDZ());
				puk->setDZ(-temp);
			}
		}
		puk->setDX(puk->getDZ());
		puk->setDZ(temp);
	}
	else {

		// Puk trifft Bande
		if ((puk->getZ() + puk->getDZ() > pg->getHeight() - puk->getSize()) || (puk->getZ() + puk->getDZ() < -(pg->getHeight() - puk->getSize())))
			puk->setDZ(-(puk->getDZ()));

		// Puk ist am Ende des Spielfeldes
		if ((puk->getX() + puk->getDX() > pg->getWidth() - puk->getSize()) || (puk->getX() + puk->getDX() < -(pg->getWidth() - puk->getSize()))) {
			// Auf Seite von Player1
			if (puk->getX() > 0) {
				// Puk wurde nicht gerettet
				if (puk->getZ() > player1->getPaddelPos() + player1->getPaddelSize() || puk->getZ() < player1->getPaddelPos() - player1->getPaddelSize()) {
					puk->stop();
					player2->win();
					player2->hasPuk(true); player1->hasPuk(false);
				}
				// Puk prallt ab
				else {
					puk->setDZ(puk->getDZ() - (player1->getPaddelPos() - puk->getZ()) / (player1->getPaddelSize() * 10));
				}
			}
			// Auf Seite von Player2
			else {
				// Puk wurde nicht gerettet
				if (puk->getZ() > player2->getPaddelPos() + player2->getPaddelSize() || puk->getZ() < player2->getPaddelPos() - player2->getPaddelSize()) {
					puk->stop();
					player1->win();
					player1->hasPuk(true); player2->hasPuk(false);
				}
				// Puk prallt ab
				else {
					puk->setDZ(puk->getDZ() - (player2->getPaddelPos() - puk->getZ()) / (player2->getPaddelSize() * 10));
				}
			}
			puk->setDX(-(puk->getDX()));

			puk->setDX(puk->getDX() * 1.5F);
			puk->setDZ(puk->getDZ() * 1.5F);
		}
	}
	puk->move();
}


//---------------------------------------------------------------------------
// Wird bei jedem Programmschritt aufgerufen
//---------------------------------------------------------------------------
void programStep()
{
	processUserInput();
	processAI();
	processPhysics();

	drawFrame();
}

//---------------------------------------------------------------------------
// Wird beim Start der Applikation aufgerufen
//---------------------------------------------------------------------------
int main(int argc, char** argv)
{
	srand((int)time(0));

	camera = new CGCamera();

	player1 = new CGPlayer(true, rgbaBlue);
	player2 = new CGPlayer(false, rgbaBlue);

	puk = new CGPuk(0.3F, rgbaRed);

	pg = new CGPlayGround(5, 8, rgbaGreen, rgbaOrange);

	box.createBox();

	CG1Helper::initApplication(ourContext, FRAME_WIDTH, FRAME_HEIGHT, FRAME_SCALE);
	CG1Helper::setProgramStep(programStep);
	CG1Helper::runApplication();

	return 0;
}
