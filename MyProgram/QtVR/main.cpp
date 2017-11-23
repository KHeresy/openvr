#include "QtVR.h"
#include <QSurfaceFormat>
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	// Set Default OpenGL Surface Format
	QSurfaceFormat glFormat;
	glFormat.setVersion(4, 1);
	//glFormat.setProfile(QSurfaceFormat::CoreProfile);
	glFormat.setProfile(QSurfaceFormat::CompatibilityProfile);
	glFormat.setSwapInterval(0);
	QSurfaceFormat::setDefaultFormat(glFormat);

	QApplication a(argc, argv);
	QtVR w;
	w.show();
	return a.exec();
}
