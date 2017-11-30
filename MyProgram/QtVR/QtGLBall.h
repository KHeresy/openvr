#pragma once

#include <QObject>
#include <QOpenGLFunctions_4_1_Core>

class QtGLBall : public QObject
{
	Q_OBJECT

public:
	QtGLBall(QObject *parent = nullptr);
	~QtGLBall();

	bool initializeGL(QOpenGLContext* pContext);
	void releaseGL();
	void render(const QMatrix4x4& matProjection, const QMatrix4x4& matModelView, int iIndex );

protected:
	QOpenGLFunctions_4_1_Core	*m_pGLFunc;
};
