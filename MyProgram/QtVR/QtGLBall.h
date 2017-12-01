#pragma once

#include <QObject>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>

class QtGLBall : public QObject
{
	Q_OBJECT

public:
	QtGLBall(QObject *parent = nullptr);
	~QtGLBall();

	void buildBall(float fSize, unsigned int uNumW, unsigned int uNumH);
	bool initializeGL(QOpenGLContext* pContext);
	void releaseGL();
	void render(const QMatrix4x4& matProjection, const QMatrix4x4& matModelView, int iIndex );

public:
	QMatrix4x4					m_matTransform1;
	QMatrix4x4					m_matTransform2;

protected:
	QOpenGLFunctions_4_1_Core	*m_pGLFunc;
	QOpenGLShaderProgram		m_glShaderProgram;
	QOpenGLBuffer				m_glVertexBuffer;
	QOpenGLBuffer				m_glIndexBuffer;
	QOpenGLVertexArrayObject	m_glVAO;
	QOpenGLTexture				*m_pTexture;

	QVector<GLfloat>			m_vVertexPoints;
	QVector<GLuint>				m_vIndexPoints;
};
