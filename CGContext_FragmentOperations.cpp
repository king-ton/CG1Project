#include "CGContext.h"
#include <iostream>
#include <cassert>

//---------------------------------------------------------------------------
// CGCONTEXT (FRAGMENT OPERATIONS)
//---------------------------------------------------------------------------
void CGContext::m_cgPushFragment(CGFragmentData& fragment)
{
	// Wait for enough fragments to process.
	m_pipelineFragments[m_pipelineFragmentsCount++] = fragment;
	if(m_pipelineFragmentsCount == CG_MAX_FRAGMENTS_IN_PIPELINE)
		m_cgFlushFragments();
}
//---------------------------------------------------------------------------
void CGContext::m_cgFlushFragments()
{
	// Run fragment pipeline components for each fragment.
	// Uncomment following line for parallel fragment processing (if OpenMP is activated)

	//#pragma omp parallel for
	for(int i=0; i<m_pipelineFragmentsCount; ++i)
		m_cgFragmentPipeline(m_pipelineFragments[i]);

	// All fragments processed, clear pipeline.
	m_pipelineFragmentsCount = 0;
}
//---------------------------------------------------------------------------
void CGContext::m_cgFragmentPipeline(CGFragmentData& fragment)
{
	if(!m_cgFragmentClipping(fragment))
		return;

	m_cgFragmentProgram(fragment);

	if(m_capabilities.depthTest)
		if (!m_cgFragmentZTest(fragment))
			return;

	if(m_capabilities.blend)
		if (!m_cgFragmentBlending(fragment)) 
			return;

	m_cgFragmentWriteBuffer(fragment);
}

//---------------------------------------------------------------------------
// Fragmente außerhalb des Bereiches (viewport) und |z| > 1 werden verworfen
//
// Übung 03 - Aufgabe 3a  |  Fragmente außerhalb des Framebuffers werden
//						  |  verworfen
// Übung 05 - Aufgabe 3a  |  Fragmente außerhalb des Z-Bereichs werden
//						  |  verworfen
// Übung 07 - Aufgabe 3b  |  ViewPort wird berücksichtigt
//---------------------------------------------------------------------------
bool CGContext::m_cgFragmentClipping(CGFragmentData& fragment)
{
	if (fragment.coordinates[X] >= m_viewport[0] + m_viewport[2] ||
		fragment.coordinates[Y] >= m_viewport[1] + m_viewport[3] ||
		fragment.coordinates[X] < m_viewport[0] ||
		fragment.coordinates[Y] < m_viewport[1] ||
		fragment.varyings[CG_POSITION_VARYING][Z] < -1.0F ||
		fragment.varyings[CG_POSITION_VARYING][Z] > 1.0F)
		return false;
	return true;
}

//---------------------------------------------------------------------------
// Anwenden der Oberflächen-, Material- und Beleuchtungseigenschaften auf
// Fragment
//
// Übung 10      - Aufgabe 2   |  Funktion implementiert
// Übung 11      - Aufgabe 2e  |  Textur-Eigenschaften werden berücksichtigt
// Hausaufgabe 4 - Aufgabe 2   |  Verschiedene Lichtquellenobjekte sind
//							   |  jetzt möglich
//---------------------------------------------------------------------------
void CGContext::m_cgFragmentProgram(CGFragmentData& fragment)
{	
	const CGFragmentData& in = fragment;
	const CGUniformData& uniforms = m_uniforms;

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
	ambiScene.set(0.0f, 0.0f, 0.0f, 0.0f);

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
	float att, spotLight;
	if (uniforms.light0Position[W] != 1)
		att = 1.0F;
	else
		att = 1.0F / (
		uniforms.light0ConstantAttenuation +
		uniforms.light0LinearAttenuation * CGMath::length(CGMath::sub(uniforms.light0Position, pos)) +
		uniforms.light0QuadraticAttenuation * pow(CGMath::length(CGMath::sub(uniforms.light0Position, pos)), 2));

	if (uniforms.light0SpotCutoff == cos(180.0F))
		spotLight = 1;
	else {
		CGVec4 S = CGMath::normalize(uniforms.light0SpotDirection);
		float SdotL = CGMath::dot(S, CGMath::scale(L, -1));
		SdotL = SdotL > 0 ? SdotL : 0;
		if (SdotL < uniforms.light0SpotCutoff)
			spotLight = 0.0F;
		else
			spotLight = pow(SdotL, uniforms.light0SpotExponent);
	}

	clr = CGMath::add(CGMath::add(CGMath::scale(CGMath::add(CGMath::add(spec, diff), ambi), (att*spotLight)), ambiScene), emis);

	// Berücksichtigung der Textur-eigenschaften
	if (uniforms.sampler.texture != NULL)
		clr = CGMath::mul(clr, uniforms.sampler.texture->sample(txc));

	// Explicitly set alpha of the color
	clr[A] = uniforms.materialDiffuse[A];
	// clamp color values to range [0,1]
	clr = CGMath::clamp(clr, 0, 1.0F);
	fragment.varyings[CG_COLOR_VARYING] = clr;
}

//---------------------------------------------------------------------------
// Überprüft, ob das aktuelle Fragment hinter dem Fragment aus dem
// FrameBuffer liegt
//
// Übung 05 - Aufgabe 3d  |  Prüfen, ob Pixel dahinter liegt
//---------------------------------------------------------------------------
bool CGContext::m_cgFragmentZTest(CGFragmentData& fragment)
{
	static const float depthTolerance = 1e-6f;
	
	return m_frameBuffer.depthBuffer.get(fragment.coordinates[X], fragment.coordinates[Y]) >= fragment.varyings[CG_POSITION_VARYING][Z] - depthTolerance;
}

//---------------------------------------------------------------------------
// Übung 06 - Aufgabe 2a  |  Methode implementiert
//---------------------------------------------------------------------------
bool CGContext::m_cgFragmentBlending(CGFragmentData& fragment)
{
	float rgba[4];
	m_frameBuffer.colorBuffer.get(fragment.coordinates[X], fragment.coordinates[Y], rgba);

	float* color = fragment.varyings[CG_COLOR_VARYING].elements;
	for (int i = 0; i < 3; ++i)
		color[i] = color[i] * color[3] + rgba[i] * (1 - color[3]);

	return true;
}

//---------------------------------------------------------------------------
// Übung 05 - Aufgabe 3d  |  Tiefe setzen, falls Fragment davor
//---------------------------------------------------------------------------
void CGContext::m_cgFragmentWriteBuffer(CGFragmentData& fragment)
{
	// Write the current fragment into the framebuffer.
	// color into color buffer
	m_frameBuffer.colorBuffer.set(fragment.coordinates[X],
								  fragment.coordinates[Y],
								  fragment.varyings[CG_COLOR_VARYING].elements);

	// Schreibe Tiefe in Tiefen-Buffer
	m_frameBuffer.depthBuffer.set(	fragment.coordinates[X],
									fragment.coordinates[Y],
									fragment.varyings[CG_POSITION_VARYING][Z]);
}
//---------------------------------------------------------------------------
