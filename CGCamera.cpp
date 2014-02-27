#include "CGCamera.h"
#include "CGMath.h"

CGCamera::CGCamera() {

	cguLookAt(0, 10, 10, 0, 0, 0, 0, 5, 0);

	project_Matrix = CGMatrix4x4::getFrustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 50.0f);

	is_2D = false;
	rotate = 0;
}

// GETTER
CGMatrix4x4 CGCamera::getProjectMatrix() {
	return project_Matrix;
}
CGMatrix4x4 CGCamera::getViewMatrix() {
	return view_Matrix;
}
bool CGCamera::is2D() {
	return is_2D;
}

// SETTER
void CGCamera::is2D(bool state) {
	is_2D = state;

	if (is_2D)
		cguLookAt(0, 10, 0, 0, 0, 0, 0, 0, -1);
	else 
		cguLookAt(0, 10, 10, 0, 0, 0, 0, 5, 0);
}
void CGCamera::setProjectMatrix(CGMatrix4x4 projectMatrix) {
	project_Matrix = projectMatrix;
}
void CGCamera::setViewMatrix(CGMatrix4x4 viewMatrix) {
	view_Matrix = viewMatrix;
}

// METHODS
void CGCamera::draw() {
	if (rotate != 0)
		view_Matrix = view_Matrix * CGMatrix4x4::getRotationMatrixY(rotate);
}

void CGCamera::cguLookAt(float eyeX, float eyeY, float eyeZ,
	float centerX, float centerY, float centerZ,
	float upX, float upY, float upZ) {

	CGMatrix4x4 V;
	CGVec4 f, s, u, up;

	f.set(centerX - eyeX, centerY - eyeY, centerZ - eyeZ, 0.0F);
	f = CGMath::normalize(f);

	up.set(upX, upY, upZ, 0);
	s = CGMath::normalize(CGMath::cross(f, up));

	u = CGMath::cross(s, f);

	f = CGMath::scale(f, -1);

	float R[16] = { s[X], s[Y], s[Z], 0,
		u[X], u[Y], u[Z], 0,
		f[X], f[Y], f[Z], 0,
		0, 0, 0, 1 };
	V.setFloatsFromRowMajor(R);
	V = V * CGMatrix4x4::getTranslationMatrix((-1)*eyeX, (-1)*eyeY, (-1)*eyeZ);

	view_Matrix = V;
}