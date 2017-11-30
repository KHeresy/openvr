#include "QtGLBall.h"

#include <QMatrix4x4>

QtGLBall::QtGLBall(QObject *parent)
	: QObject(parent)
{
	m_pGLFunc = nullptr;
	m_pTexture = nullptr;
}

QtGLBall::~QtGLBall()
{
	releaseGL();
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

		// Data of Vertext buffer
		QVector<GLfloat> vPoints = {
			0, -0.5, -0.5, 0, 0,
			0, 0.5, -0.5, 0, 1,
			0, 0.0, 0.5, 1, 0
		};
		m_glVertexBuffer.allocate(vPoints.data(), vPoints.length() * sizeof(GLfloat));

		// bind shader
		m_glShaderProgram.bind();

		// setup shader
		m_glShaderProgram.setAttributeBuffer("vertex", GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
		m_glShaderProgram.enableAttributeArray("vertex");

		m_glShaderProgram.setAttributeBuffer("texCoord", GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));
		m_glShaderProgram.enableAttributeArray("texCoord");

		m_glShaderProgram.setUniformValue("diffuse", 0);

		m_pTexture = new QOpenGLTexture(QImage("C:\\Users\\Heresy\\Pictures\\viewer.jpg"));
	}

	return bOK;
}

void QtGLBall::releaseGL()
{
	m_pGLFunc = nullptr;
}

void QtGLBall::render(const QMatrix4x4 & matProjection, const QMatrix4x4 & matModelView, int iIndex)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	m_glVAO.bind();
	m_glShaderProgram.bind();
	m_pTexture->bind();

	m_glShaderProgram.setUniformValue("transform", matProjection * matModelView);
	m_glShaderProgram.setUniformValue("leftEye", false);
	m_glShaderProgram.setUniformValue("overUnder", false);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	
	m_pTexture->release();
	auto x = m_pTexture->textureId();
	m_glVAO.release();
	m_glShaderProgram.release();
	return;
}
