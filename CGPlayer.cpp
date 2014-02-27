#include "CGPlayer.h"

CGPlayer::CGPlayer(bool hasPuk, float* color) {
	has_puk = hasPuk;

	ki = false;
	paddel_pos = 0;
	paddel_size = 1.0F;
	points = 0;

	paddel = new CGQuadric();
	paddel->createBox();
	this->color = color;
}

// GETTER
bool CGPlayer::hasPuk() {
	return has_puk;
}
bool CGPlayer::isKi() {
	return ki;
}
float CGPlayer::getPaddelPos() {
	return paddel_pos;
}
float CGPlayer::getPaddelSize() {
	return paddel_size;
}
int CGPlayer::getPoints() {
	return points;
}

// SETTER
void CGPlayer::hasPuk(bool hasPuk) {
	has_puk = hasPuk;
}
void CGPlayer::isKi(bool ki) {
	this->ki = ki;
}
void CGPlayer::setPaddelSize(float size) {
	this->paddel_size = size;
}

// METHODS
void CGPlayer::draw(CGContext* context, CGMatrix4x4* viewT, float offset) {
	CGMatrix4x4 modelviewT = *viewT;
	modelviewT = modelviewT * CGMatrix4x4::getTranslationMatrix(offset, 0, paddel_pos);
	modelviewT = modelviewT * CGMatrix4x4::getScaleMatrix(0.1F, 0.4F, paddel_size);
	CGQuadric::renderQuadric(*paddel, context, modelviewT, color);
}
void CGPlayer::move(CGPlayGround* pg, float offset) {
	paddel_pos += offset;

	if (paddel_pos > (pg->getHeight() - paddel_size) || paddel_pos < (-(pg->getHeight() - paddel_size)))
		paddel_pos -= offset;
}
void CGPlayer::win() {
	points++;
}