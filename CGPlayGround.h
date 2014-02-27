#include "CGQuadric.h"

class CGPlayGround {
public:
	CGPlayGround(int height, int width, float* groundColor, float* borderColor);

	// GETTER
	int getHeight();
	int getWidth();

	CGQuadric* getGround();
	CGQuadric* getBorder();

	// SETTER
	void setHeight(int height);
	void setWidth(int width);

	// METHOD
	void draw(CGContext* context, CGMatrix4x4* viewT);

private:
	int width;
	int height;

	float* border_color;
	float* ground_color;

	CGQuadric* ground;
	CGQuadric* border;
};