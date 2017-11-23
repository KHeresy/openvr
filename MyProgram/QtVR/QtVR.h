#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtVR.h"

class QtVR : public QMainWindow
{
	Q_OBJECT

public:
	QtVR(QWidget *parent = Q_NULLPTR);

private:
	Ui::QtVRClass ui;
};
