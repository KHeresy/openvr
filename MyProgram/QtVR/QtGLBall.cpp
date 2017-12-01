#include "QtGLBall.h"

#include <QtMath>
#include <QMatrix4x4>

QtGLBall::QtGLBall(QObject *parent)
	: QObject(parent), m_glVertexBuffer(QOpenGLBuffer::VertexBuffer), m_glIndexBuffer(QOpenGLBuffer::IndexBuffer)
{
	m_pGLFunc = nullptr;
	m_pTexture = nullptr;

	m_matTransform1.setToIdentity();
	m_matTransform2.setToIdentity();
}

QtGLBall::~QtGLBall()
{
	releaseGL();
}

void QtGLBall::buildBall(float fSize, unsigned int uNumW, unsigned int uNumH)
{
	m_vVertexPoints.clear();
	m_vIndexPoints.clear();

	unsigned int uSize = uNumW * uNumH;
	m_vVertexPoints.reserve(uSize * 5);
	float fXRatio = 360.0 / (uNumW - 1);
	float fYRatio = 180.0 / (uNumH - 1);
	float fX;
	float fY;

	//TODO: have problem at op and bottom
	for (int y = 0; y < uNumH; ++y )
	{
		for (int x = 0; x < uNumW; ++x )
		{
			int iIdx = y * uNumW + x;
			fX = (-180.0 + x * fXRatio) * M_PI / 180.0;
			fY = (-90.0 + y * fYRatio) * M_PI / 180.0;

			m_vVertexPoints.push_back(fSize*cos(fY)*cos(fX));
			m_vVertexPoints.push_back(fSize*cos(fY)*sin(fX));
			m_vVertexPoints.push_back(fSize*sin(fY));
			m_vVertexPoints.push_back(x / (uNumW - 1.0));
			m_vVertexPoints.push_back(1.0 - (y / (uNumH - 1.0)));
		}
	}

	for (int y = 0; y < uNumH; ++y)
	{
		for (int x = 0; x < uNumW; ++x)
		{
			m_vIndexPoints.push_back(y*uNumW + x);
			m_vIndexPoints.push_back((y+1)*uNumW + x);
		}
	}

	m_matTransform1.rotate(-90, 1, 0, 0);
	m_matTransform2.translate(0, 1 + fSize / 2, 0);
}

bool QtGLBall::initializeGL(QOpenGLContext * pContext)
{
	m_pGLFunc = pContext->versionFunctions<QOpenGLFunctions_4_1_Core>();
	if (!m_pGLFunc)
		return false;

	bool bOK = false;

	// compile shader
	if (m_glShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/QtVR/shader/unlit.vert") &&
		m_glShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/QtVR/shader/unlit.frag"))
	{
		bOK = m_glShaderProgram.link();
	}

	// build VAO
	if (bOK)
	{
		// Vertex Array Object
		m_glVAO.create();
		m_glVAO.bind();

		// Vertext buffer
		m_glVertexBuffer.create();
		m_glVertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
		m_glVertexBuffer.bind();
		m_glVertexBuffer.allocate(m_vVertexPoints.data(), m_vVertexPoints.length() * sizeof(GLfloat));

		// Index Array
		m_glIndexBuffer.create();
		m_glIndexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
		m_glIndexBuffer.bind();
		m_glIndexBuffer.allocate(m_vIndexPoints.data(), m_vIndexPoints.length() * sizeof(GLuint));

		// bind shader
		m_glShaderProgram.bind();

		// setup shader
		m_glShaderProgram.setAttributeBuffer("vertex", GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
		m_glShaderProgram.enableAttributeArray("vertex");

		m_glShaderProgram.setAttributeBuffer("texCoord", GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));
		m_glShaderProgram.enableAttributeArray("texCoord");

		m_glShaderProgram.setUniformValue("diffuse", 0);

		m_pTexture = new QOpenGLTexture(QImage("C:\\Users\\Heresy\\Pictures\\test.jpg"));
	}

	return bOK;
}

void QtGLBall::releaseGL()
{
	m_pGLFunc = nullptr;
}

void QtGLBall::render(const QMatrix4x4 & matProjection, const QMatrix4x4 & matModelView, int iIndex)
{
	m_pGLFunc->glEnable(GL_DEPTH_TEST);
	m_pGLFunc->glEnable(GL_TEXTURE_2D);

	m_glVAO.bind();
	m_glShaderProgram.bind();
	m_pTexture->bind();

	m_glShaderProgram.setUniformValue("transform", matProjection * matModelView * m_matTransform2 * m_matTransform1);
	m_glShaderProgram.setUniformValue("leftEye", false);
	m_glShaderProgram.setUniformValue("overUnder", false);
	m_pGLFunc->glDrawElements(GL_TRIANGLE_STRIP, m_vIndexPoints.size(), GL_UNSIGNED_INT, 0);
	
	m_pTexture->release();
	m_glVAO.release();
	m_glShaderProgram.release();
	return;
}
