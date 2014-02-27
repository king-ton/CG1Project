#include "CGQuadric.h"
#include "CGPlayGround.h"

class CGPlayer {
public:
	CGPlayer(bool hasPuk, float* color);

	// GETTER
	bool hasPuk();
	bool isKi();
	float getPaddelPos();
	float getPaddelSize();
	int getPoints();

	// SETTER
	void hasPuk(bool hasPuk);
	void isKi(bool ki);
	void setPaddelSize(float size);

	// METHODS
	void draw(CGContext* context, CGMatrix4x4* viewT, float offset);
	void move(CGPlayGround* pg, float offset);
	void win();

private:
	bool has_puk;
	bool ki;

	int points;

	float paddel_size;
	float paddel_pos;
	float* color;

	CGQuadric* paddel;
};