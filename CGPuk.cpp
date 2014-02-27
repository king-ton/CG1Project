#include "CGPuk.h"

CGPuk::CGPuk(float size, float* color) {
	x = z = dx = dz = 0;
	this->size = size;

	disk.createDisk(15, 2);
	cylinder.createCylinder(15, 1);
	this->color = color;
}

// GETTER
float CGPuk::getX() {
	return x;
}
float CGPuk::getZ() {
	return z;
}
float CGPuk::getDX() {
	return dx;
}
float CGPuk::getDZ() {
	return dz;
}
float CGPuk::getSize() {
	return size;
}
CGQuadric* CGPuk::getCylinder() {
	return &cylinder;
}

// SETTER
void CGPuk::setX(float x) {
	this->x = x;
}
void CGPuk::setZ(float z) {
	this->z = z;
}

void CGPuk::setDX(float dx) {
	this->dx = dx;
}
void CGPuk::setDZ(float dz) {
	this->dz = dz;
}
void CGPuk::setSize(float size) {
	this->size = size;
}

// METHOD
bool CGPuk::isMove() {
	return !(dx == 0 && dz == 0);
}
void CGPuk::draw(CGContext* context, CGMatrix4x4* viewT) {
	CGMatrix4x4 modelviewT = *viewT;
	modelviewT = modelviewT * CGMatrix4x4::getTranslationMatrix(x, 0, z);
	modelviewT = modelviewT * CGMatrix4x4::getScaleMatrix(size, 0.3F, size);
	modelviewT = modelviewT * CGMatrix4x4::getRotationMatrixX(-90);
	CGQuadric::renderQuadric(cylinder, context, modelviewT, color);

	modelviewT = modelviewT * CGMatrix4x4::getTranslationMatrix(0, 0, 0.5F);
	CGQuadric::renderQuadric(disk, context, modelviewT, color);
}
void CGPuk::move() {
	x += dx;
	z += dz;
}
void CGPuk::stop() {
	dx = dz = 0;
}