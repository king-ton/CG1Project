#include "CGQuadric.h"
#include "CGMatrix.h"

class CGPuk {
public:
	CGPuk(float size, float* color);

	// GETTER
	float getX();
	float getZ();
	float getDX();
	float getDZ();
	float getSize();
	CGQuadric* getCylinder();

	// SETTER
	void setX(float x);
	void setZ(float z);
	void setDX(float dx);
	void setDZ(float dz);
	void setSize(float size);

	// METHODS
	bool isMove();
	void draw(CGContext* context, CGMatrix4x4* viewT);
	void move();
	void stop();
private:
	float x;
	float z;
	float dx;
	float dz;

	float size;
	float* color;

	CGQuadric disk;
	CGQuadric cylinder;
};