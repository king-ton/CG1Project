#include "CGPlayGround.h"

CGPlayGround::CGPlayGround(int height, int width, float* groundColor, float* borderColor) {
	this->height = height;
	this->width = width;

	ground = new CGQuadric();
	ground->createBox(width, 1, height);
	this->ground_color = groundColor;

	border = new CGQuadric();
	border->createBox(width, 1, 1);
	ground->setStandardColor(borderColor[0], borderColor[1], borderColor[2], borderColor[3]);
	this->border_color = borderColor;
}

// GETTER
int CGPlayGround::getHeight() {
	return height;
}
int CGPlayGround::getWidth() {
	return width;
}
CGQuadric* CGPlayGround::getGround() {
	return ground;
}
CGQuadric* CGPlayGround::getBorder() {
	return border;
}

// METHOD
void CGPlayGround::draw(CGContext* context, CGMatrix4x4* viewT) {
	CGMatrix4x4 modelviewT;

	// GROUND
	modelviewT = *viewT;
	modelviewT = modelviewT * CGMatrix4x4::getTranslationMatrix(0, -0.1F, 0);
	modelviewT = modelviewT * CGMatrix4x4::getScaleMatrix((float)width, 0.1F, (float)height);
	CGQuadric::renderQuadric(*ground, context, modelviewT, ground_color);

	// BORDER TOP
	modelviewT = *viewT;
	modelviewT = modelviewT * CGMatrix4x4::getTranslationMatrix(0, 0, (float)(-1) * height);
	modelviewT = modelviewT * CGMatrix4x4::getScaleMatrix((float)width, 0.4F, 0.05F);
	CGQuadric::renderQuadric(*border, context, modelviewT, border_color);

	// BORDER BOTTOM
	modelviewT = *viewT;
	modelviewT = modelviewT * CGMatrix4x4::getTranslationMatrix(0, 0, (float)height);
	modelviewT = modelviewT * CGMatrix4x4::getScaleMatrix((float)width, 0.4F, 0.05F);
	CGQuadric::renderQuadric(*border, context, modelviewT, border_color);
}