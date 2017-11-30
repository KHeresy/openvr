#include "QtGLBall.h"

#include <QMatrix4x4>

QtGLBall::QtGLBall(QObject *parent)
	: QObject(parent)
{
	m_pGLFunc = nullptr;
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
	return true;
}

void QtGLBall::releaseGL()
{
	m_pGLFunc = nullptr;
}

void QtGLBall::render(const QMatrix4x4 & matProjection, const QMatrix4x4 & matModelView, int iIndex)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(matProjection.constData());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(matModelView.constData());

	//renderEye(rEyeData.m_eIndex);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0, -0.5, -0.5);
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0, 0.5, -0.5);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0, 0.0, 0.5);
	glEnd();

}
