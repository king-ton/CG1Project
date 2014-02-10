// Welche Übung soll ausgeführt werden?
#define PROJECT

//---------------------------------------------------------------------------
// INCLUDES
//
// Hausaufgabe 1 - Aufgabe 2c   |  CGImageFile hinzugefügt
// Übung 09      - Aufgabe 1    |  CGMath und CG1Application_renderSphere
//							    |  hinzugefügt
// Hausaufgabe 3 - Aufgabe 1.2  |  CGQuadric hinzugefügt
// Übung 12      - Aufgabe 1a   |  TestDataSet hinzugefügt
// Hausaufgabe 4 - Aufgabe 1.2  |  CGLightSource hinzugefügt
//---------------------------------------------------------------------------
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
#define FRAME_WIDTH  64		// Framebuffer width.
#define FRAME_HEIGHT 48		// Framebuffer height.
#define FRAME_SCALE  10		// Integer scaling factors (zoom).

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
	int dx;
	int dy;
	int x;
	int y;
} Ball;

//---------------------------------------------------------------------------
// GLOBALE VARIABLEN
//---------------------------------------------------------------------------
CGContext *ourContext;

Player player1 = { 6, 23, 0 };
Player player2 = { 6, 23, 0 };
Ball ball = {0, 0, 0, 0}


//---------------------------------------------------------------------------
// VERTEX PROGRAMME
//---------------------------------------------------------------------------
#if defined(U1) || defined(U2) || defined(U3_1) || defined(U3_2) || defined(U3_3) || defined(U4) || defined(U5) || defined(U6) || defined(U6_4) || defined(HA1)
//---------------------------------------------------------------------------
// generic "passthorugh" vertex program
void passthroughVertexProgram(	const CGVertexAttributes& in,
								CGVertexVaryings& out,
								const CGUniformData& uniforms)
{
	out.varyings[CG_POSITION_VARYING] = in.attributes[CG_POSITION_ATTRIBUTE];
	out.varyings[CG_NORMAL_VARYING] = in.attributes[CG_NORMAL_ATTRIBUTE];
	out.varyings[CG_COLOR_VARYING] = in.attributes[CG_COLOR_ATTRIBUTE];
	out.varyings[CG_TEXCOORD_VARYING] = in.attributes[CG_TEXCOORD_ATTRIBUTE];
}
#endif

//---------------------------------------------------------------------------
// Übung 07 - Aufgabe 2b  |  Vertex-Programm erstellt, Transformation der
//							 Vertex-Position mithilfe der Projektions-Matrix
//---------------------------------------------------------------------------
#if defined(U7)
void projectionVertexProgram(	const CGVertexAttributes& in,
								CGVertexVaryings& out,
								const CGUniformData& uniforms)
{
	out.varyings[CG_POSITION_VARYING] = uniforms.projectionMatrix * in.attributes[CG_POSITION_ATTRIBUTE];

	out.varyings[CG_NORMAL_VARYING] = in.attributes[CG_NORMAL_ATTRIBUTE];
	out.varyings[CG_COLOR_VARYING] = in.attributes[CG_COLOR_ATTRIBUTE];
	out.varyings[CG_TEXCOORD_VARYING] = in.attributes[CG_TEXCOORD_ATTRIBUTE];
}
#endif

//---------------------------------------------------------------------------
// Übung 08 - Aufgabe 2a  |  Vertex-Programm erstellt, Transformation der
//						  |	 Vertex-Position mithilfe der ModelView-Matrix
//						  |  und der Projections-Matrix
//---------------------------------------------------------------------------
#if defined(U8) || defined(U12) || defined(HA3)
void modelViewProjectionVertexProgram(const CGVertexAttributes& in,
	CGVertexVaryings& out,
	const CGUniformData& uniforms)
{
	out.varyings[CG_POSITION_VARYING] = uniforms.projectionMatrix * uniforms.modelviewMatrix * in.attributes[CG_POSITION_ATTRIBUTE];

	out.varyings[CG_NORMAL_VARYING] = in.attributes[CG_NORMAL_ATTRIBUTE];
	out.varyings[CG_COLOR_VARYING] = in.attributes[CG_COLOR_ATTRIBUTE];
	out.varyings[CG_TEXCOORD_VARYING] = in.attributes[CG_TEXCOORD_ATTRIBUTE];
}
#endif

//---------------------------------------------------------------------------
// Übung 09 - Aufgabe 2   |  Emmisiver, ambienter und diffuser Anteil
//						  |  implementiert
// Übung 09 - Aufgabe 3   |  Spekularer Anteil implementiert
//---------------------------------------------------------------------------
#if defined(U9) || defined(U10) || defined(U12)
void perVertexLighingVertexProgram(	const CGVertexAttributes& in,
									CGVertexVaryings& out,
									const CGUniformData& uniforms)
{
	// Get hold of all vertex attributes.
	CGVec4 aPos = in.attributes[CG_POSITION_ATTRIBUTE];
	CGVec4 aNrm = in.attributes[CG_NORMAL_ATTRIBUTE];
	CGVec4 aClr = in.attributes[CG_COLOR_ATTRIBUTE];
	CGVec4 aTex = in.attributes[CG_TEXCOORD_ATTRIBUTE];

	// Get hold of all vertex varyings.
	CGVec4 &vPos = out.varyings[CG_POSITION_VARYING];
	CGVec4 &vNrm = out.varyings[CG_NORMAL_VARYING];
	CGVec4 &vClr = out.varyings[CG_COLOR_VARYING];
	CGVec4 &vTex = out.varyings[CG_TEXCOORD_VARYING];

	// Default program copies all attributes into all varyings used:
	vPos = aPos; vNrm = aNrm; vClr = aClr; vTex = aTex;

	// Transform from Object Space into Eye Space.
	vPos = uniforms.modelviewMatrix * vPos;
	vNrm = CGMath::normalize(uniforms.normalMatrix * vNrm);

	CGVec4 emis, ambi, diff, spec;
	// TODO: for now, set them all to 0
	emis.set(0.0f, 0.0f, 0.0f, 0.0f); ambi.set(0.0f, 0.0f, 0.0f, 0.0f);
	diff.set(0.0f, 0.0f, 0.0f, 0.0f); spec.set(0.0f, 0.0f, 0.0f, 0.0f);

	emis = uniforms.materialEmission;
	ambi = CGMath::mul(uniforms.materialAmbient, uniforms.light0Ambient);

	// L is vector direction from current point (vPos) to the light source (m_uniforms.light0Position)
	CGVec4 L = CGMath::normalize(CGMath::sub(uniforms.light0Position, vPos));
	// calculate dot product of nrm and L
	float NdotL = CGMath::dot(vNrm, L);

	if (NdotL > 0.0F) {
		// diffuse
		diff = CGMath::scale(CGMath::mul(uniforms.materialDiffuse, uniforms.light0Diffuse), NdotL);

		// E is direction from current point (pos) to eye position
		CGVec4 ePos; ePos.set(0.0f, 0.0f, 0.0f, 1.0f);
		CGVec4 E = CGMath::normalize(CGMath::sub(ePos, vPos));
		// H is half vector between L and E
		CGVec4 H = CGMath::normalize(CGMath::add(L, E));

		// specular
		float NdotH = CGMath::dot(vNrm, H);

		if (NdotH > 0.0F)
			spec = CGMath::scale(CGMath::mul(uniforms.materialSpecular, uniforms.light0Specular), pow(NdotH, uniforms.materialShininess));
	}
	// sum up the final output color
	vClr = CGMath::add(CGMath::add(CGMath::add(ambi, diff), spec), emis);
	// clamp color values to range [0,1]
	vClr[A] = uniforms.materialDiffuse[A];
	vClr = CGMath::clamp(vClr, 0, 1);

	// Transform from Eye Space into Clip Space.
	vPos = uniforms.projectionMatrix * vPos;
}
#endif

//---------------------------------------------------------------------------
// Übung 10 - Aufgabe 2   |  Funktion erstellt
//---------------------------------------------------------------------------
#if defined(U10) || defined(U11) || defined(U12) || defined(HA4)
void perPixelLighingVertexProgram(	const CGVertexAttributes& in,
									CGVertexVaryings& out,
									const CGUniformData& uniforms)
{
	// Get hold of all vertex attributes.
	CGVec4 aPos = in.attributes[CG_POSITION_ATTRIBUTE];
	CGVec4 aNrm = in.attributes[CG_NORMAL_ATTRIBUTE];
	CGVec4 aClr = in.attributes[CG_COLOR_ATTRIBUTE];
	CGVec4 aTex = in.attributes[CG_TEXCOORD_ATTRIBUTE];

	// Get hold of all vertex varyings.
	CGVec4 &vPos = out.varyings[CG_POSITION_VARYING];
	CGVec4 &vNrm = out.varyings[CG_NORMAL_VARYING];
	CGVec4 &vClr = out.varyings[CG_COLOR_VARYING];
	CGVec4 &vTex = out.varyings[CG_TEXCOORD_VARYING];
	CGVec4 &vPEs = out.varyings[CG_POSITION_EYESPACE_VARYING];

	// Default program copies all attributes into all varyings used:
	vPos = aPos; vNrm = aNrm; vClr = aClr; vTex = aTex;

	// Transform from Object Space into Eye Space.
	vPEs = uniforms.modelviewMatrix * aPos;
	vNrm = uniforms.normalMatrix* vNrm;
	CGMath::normalize(vNrm);

	// Transform from Eye Space into Clip Space.
	vPos = uniforms.projectionMatrix * vPEs;
}
#endif

//---------------------------------------------------------------------------
// FRAGMENT PROGRAMME
//---------------------------------------------------------------------------
#if defined(U1) || defined(U2) || defined(U3_1) || defined(U3_2) || defined(U3_3) || defined(U4) || defined(U5) || defined(U6) || defined(U6_4) || defined(U7) || defined(U8) || defined(U9) || defined(U10) || defined(U12) || defined(HA1) || defined(HA3)
//---------------------------------------------------------------------------
// generic "passthorugh" fragment program
void passthroughFragmentProgram(const CGFragmentData& in,
								CGVec4& out,
								const CGUniformData& uniforms)
{
	out = in.varyings[CG_COLOR_VARYING];
}
#endif

//---------------------------------------------------------------------------
// Übung 10 - Aufgabe 2   |  Funktion implementiert
// Übung 11 - Aufgabe 2e  |  Textur-Eigenschaften werden berücksichtigt
//---------------------------------------------------------------------------
#if defined(U10) || defined(U11) || defined(U12) || defined(HA4)
void perPixelLighingFragmentProgram(const CGFragmentData& in,
	CGVec4& out,
	const CGUniformData& uniforms)
{
	CGVec4 clr = in.varyings[CG_COLOR_VARYING];
	CGVec4 nrm = in.varyings[CG_NORMAL_VARYING];
	CGVec4 txc = in.varyings[CG_TEXCOORD_VARYING];
	CGVec4 pos = in.varyings[CG_POSITION_EYESPACE_VARYING];

	// renormalize the normal
	nrm = CGMath::normalize(nrm);

	// Compute Blinn-Phong reflection model.
	CGVec4 emis, ambiScene, ambi, diff, spec;
	emis.set(0.0f, 0.0f, 0.0f, 0.0f); ambi.set(0.0f, 0.0f, 0.0f, 0.0f);
	diff.set(0.0f, 0.0f, 0.0f, 0.0f); spec.set(0.0f, 0.0f, 0.0f, 0.0f);
	// emission
	emis = uniforms.materialEmission;
	// ambient of the scene
	ambiScene = CGMath::mul(uniforms.materialAmbient, uniforms.sceneAmbient);
	// ambient
	ambi = CGMath::mul(uniforms.materialAmbient, uniforms.light0Ambient);

	// L is vector direction from current point (pos) to the light source (m_uniforms.light0Position)
	CGVec4 L;
	if (uniforms.light0Position[W] == 0)
		L = CGMath::normalize(uniforms.light0Position);
	else
		L = CGMath::normalize(CGMath::sub(uniforms.light0Position, pos));

	// calculate dot product of nrm and L
	float NdotL = CGMath::dot(nrm, L);

	if (NdotL>0)
	{
		// diffuse
		diff = CGMath::scale(CGMath::mul(uniforms.materialDiffuse, uniforms.light0Diffuse), NdotL);

		// E is direction from current point (pos) to eye position
		CGVec4 ePos; ePos.set(0.0f, 0.0f, 0.0f, 1.0f);
		CGVec4 E = CGMath::normalize(CGMath::sub(ePos, pos));
		// H is half vector between L and E
		CGVec4 H = CGMath::normalize(CGMath::add(L, E));

		// specular
		float NdotH = CGMath::dot(nrm, H);

		if (NdotH > 0.0F)
			spec = CGMath::scale(CGMath::mul(uniforms.materialSpecular, uniforms.light0Specular), pow(NdotH, uniforms.materialShininess));
	}

	// sum up the final output color
	clr = CGMath::add(CGMath::add(CGMath::add(ambi, diff), spec), emis);

	// Berücksichtigung der Textur-eigenschaften
	if (uniforms.sampler.texture != NULL)
		clr = CGMath::mul(clr, uniforms.sampler.texture->sample(txc));

	// Explicitly set alpha of the color
	clr[A] = uniforms.materialDiffuse[A];
	// clamp color values to range [0,1]
	clr = CGMath::clamp(clr, 0, 1);
	out = clr;
}
#endif

//---------------------------------------------------------------------------
// Hausaufgabe 3 - Aufgabe 1.2  |  Funktion erstellt
//---------------------------------------------------------------------------
#if defined(HA3)
void normalVertexProgram(	const CGVertexAttributes& in,
							CGVertexVaryings& out,
							const CGUniformData& uniforms)
{
	out.varyings[CG_POSITION_VARYING] = in.attributes[CG_POSITION_ATTRIBUTE];
	out.varyings[CG_NORMAL_VARYING] = in.attributes[CG_NORMAL_ATTRIBUTE];
	out.varyings[CG_TEXCOORD_VARYING] = in.attributes[CG_TEXCOORD_ATTRIBUTE];

	// Transform from Object Space into Eye Space.
	out.varyings[CG_POSITION_VARYING] = uniforms.modelviewMatrix * out.varyings[CG_POSITION_VARYING];

	// Transform from Eye Space into Clip Space.
	out.varyings[CG_POSITION_VARYING] = uniforms.projectionMatrix * out.varyings[CG_POSITION_VARYING];

	// Set normal as color, transformed into [0,1]-range
	out.varyings[CG_COLOR_VARYING][R] = 0.5f * out.varyings[CG_NORMAL_VARYING][X] + 0.5f;
	out.varyings[CG_COLOR_VARYING][G] = 0.5f * out.varyings[CG_NORMAL_VARYING][Y] + 0.5f;
	out.varyings[CG_COLOR_VARYING][B] = 0.5f * out.varyings[CG_NORMAL_VARYING][Z] + 0.5f;
	out.varyings[CG_COLOR_VARYING][A] = 1.0f;
}
#endif

//---------------------------------------------------------------------------
// Erstellt die View-Matrix
//
// Übung 08 - Aufgabe 1a  |  Funktion erstellt
// Übung 08 - Aufgabe 4a  |  Funktion implementiert
// Übung 09 - Aufgabe 1   |  Refaktorisierung 
//---------------------------------------------------------------------------
#if defined(U8) || defined(U9) || defined(U10) || defined(U11) || defined(HA3) || defined(HA4)
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
#endif

//---------------------------------------------------------------------------
// Hausaufgabe 3 - Aufgabe 1.2  |  Funktion erstellt
//---------------------------------------------------------------------------
#if defined(U11) || defined(U12) || defined(HA3) || defined(HA4)
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
#endif

void drawFrame()
{
	float pos[4 * 3 * 2];
	float color[4 * 4 * 2];

	ourContext->cgVertexAttribPointer(CG_POSITION_ATTRIBUTE, pos);
	ourContext->cgVertexAttribPointer(CG_COLOR_ATTRIBUTE, color);

	ourContext->cgDrawArrays(CG_LINES, 0, 2 * 4);
}