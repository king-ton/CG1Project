#include "CGMatrix.h"

class CGCamera {
public:
	CGCamera();

	// GETTER
	CGMatrix4x4 getProjectMatrix();
	CGMatrix4x4 getViewMatrix();
	bool is2D();

	// SETTER
	void is2D(bool state);
	void setProjectMatrix(CGMatrix4x4 projectMatrix);
	void setViewMatrix(CGMatrix4x4 viewMatrix);

	// METHODS
	void draw();

private:
	// -1 nach links, 1 nach rechts, 0 keine Rotation
	float rotate;

	bool is_2D;

	CGMatrix4x4 project_Matrix;
	CGMatrix4x4 view_Matrix;

	// METHODS
	void cguLookAt(float eyeX, float eyeY, float eyeZ,
		float centerX, float centerY, float centerZ,
		float upX, float upY, float upZ);
};