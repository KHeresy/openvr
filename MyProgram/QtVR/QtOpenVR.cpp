#include "QtOpenVR.h"

#include <QMessageBox>

QtOpenVR::QtOpenVR(QWidget *parent) : QOpenGLWidget(parent)
{
	m_pVRSystem			= nullptr;
	m_pResolveBuffer	= nullptr;

	m_fNearDistance = 0.1f;
	m_fFarDistance = 5.0f;

	m_aEyeDaya[0].m_eIndex = vr::Eye_Left;
	m_aEyeDaya[1].m_eIndex = vr::Eye_Right;
}

QtOpenVR::~QtOpenVR()
{
}

void QtOpenVR::initializeGL()
{
	initializeOpenGLFunctions();

	m_Logger = new QOpenGLDebugLogger(this);
	connect(m_Logger, &QOpenGLDebugLogger::messageLogged, [](QOpenGLDebugMessage message) {
		auto s = message.message();
		auto st = message.severity();
		qDebug() << message;
	});

	if (m_Logger->initialize())
	{
		m_Logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
		m_Logger->enableMessages();
	}

	glClearColor(0.4, 0.4, 0, 4);
	glEnable(GL_DEPTH_TEST);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	initializeVR();
}

void QtOpenVR::resizeGL(int w, int h)
{
}

void QtOpenVR::paintGL()
{
	if (m_pVRSystem)
	{
		updatePose();

		glViewport(0, 0, m_aOvrFrameSize[0], m_aOvrFrameSize[1]);
		QRect rectSource(0, 0, m_aOvrFrameSize[0], m_aOvrFrameSize[1]);

		for (auto& rEyeData : m_aEyeDaya)
		{
			glEnable(GL_MULTISAMPLE);

			rEyeData.m_pFrameBuffer->bind();

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glMultMatrixf(rEyeData.m_matProjection.constData());
			
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMultMatrixf((m_matHMDPose * rEyeData.m_matPose).constData());

			//renderEye(rEyeData.m_eIndex);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);

			glBegin(GL_TRIANGLES);
			glColor3f(1.0, 0.0, 0.0);
			glVertex3f(0,-0.5, -0.5);
			glColor3f(0.0, 1.0, 0.0);
			glVertex3f(0, 0.5, -0.5);
			glColor3f(0.0, 0.0, 1.0);
			glVertex3f(0, 0.0, 0.5);
			glEnd();

			rEyeData.m_pFrameBuffer->release();

			QOpenGLFramebufferObject::blitFramebuffer(m_pResolveBuffer, rEyeData.m_rectTarget, rEyeData.m_pFrameBuffer, rectSource);
		}

		submitBuffer();
	}

	glViewport(0, 0, width(), height());
	glDisable(GL_MULTISAMPLE);
	//glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	//glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0, -0.5, -0.5);
	glVertex3f(0, 0.5, -0.5);
	glVertex3f(0, 0.5, 0.5);
	glVertex3f(0, 0.5, 0.5);
	glEnd();

	update();
}

bool QtOpenVR::initializeVR()
{
	vr::EVRInitError error = vr::VRInitError_None;
	m_pVRSystem = vr::VR_Init(&error, vr::VRApplication_Scene);

	if (error != vr::VRInitError_None)
	{
		m_pVRSystem = nullptr;

		QString message = vr::VR_GetVRInitErrorAsEnglishDescription(error);
		qCritical() << message;
		QMessageBox::critical(this, "Unable to init VR", message);
		return false;
	}

	// setup frame buffers for eyes
	m_pVRSystem->GetRecommendedRenderTargetSize(&m_aOvrFrameSize[0], &m_aOvrFrameSize[1]);

	// get eye matrices
	for (auto& rEyeData : m_aEyeDaya)
		rEyeData.initialize(*this);

	m_aEyeDaya[0].m_rectTarget = QRect(0, 0, m_aOvrFrameSize[0], m_aOvrFrameSize[1]);
	m_aEyeDaya[1].m_rectTarget = QRect(m_aOvrFrameSize[0], 0, m_aOvrFrameSize[0], m_aOvrFrameSize[1]);

	QOpenGLFramebufferObjectFormat resolveFormat;
	resolveFormat.setInternalTextureFormat(GL_RGBA8);

	m_pResolveBuffer = new QOpenGLFramebufferObject(m_aOvrFrameSize[0] * 2, m_aOvrFrameSize[1], resolveFormat);

	// turn on compositor
	if (!vr::VRCompositor())
	{
		QString message = "Compositor initialization failed. See log file for details";
		qCritical() << message;
		QMessageBox::critical(this, "Unable to init VR", message);
		return false;
	}

	return true;
}

void QtOpenVR::updatePose()
{
	vr::VRCompositor()->WaitGetPoses(m_aOvrDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	for (unsigned int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i)
	{
		if (m_aOvrDevicePose[i].bPoseIsValid)
			m_aDevicePose[i] = toQMatrix4(m_aOvrDevicePose[i].mDeviceToAbsoluteTracking);
	}

	if (m_aOvrDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
		m_matHMDPose = m_aDevicePose[vr::k_unTrackedDeviceIndex_Hmd].inverted();
}

void QtOpenVR::submitBuffer()
{
	static vr::VRTextureBounds_t leftRect = { 0.0f, 0.0f, 0.5f, 1.0f };
	static vr::VRTextureBounds_t rightRect = { 0.5f, 0.0f, 1.0f, 1.0f };

	if (m_pVRSystem)
	{
		vr::Texture_t composite = { (void*)m_pResolveBuffer->texture(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &composite, &leftRect);
		vr::VRCompositor()->Submit(vr::Eye_Right, &composite, &rightRect);
	}
}

void QtOpenVR::CEyeData::initialize(QtOpenVR& rQVR)
{
	m_matProjection = toQMatrix4(rQVR.m_pVRSystem->GetProjectionMatrix(m_eIndex, rQVR.m_fNearDistance, rQVR.m_fFarDistance));
	m_matPose = toQMatrix4(rQVR.m_pVRSystem->GetEyeToHeadTransform(m_eIndex)).inverted();

	QOpenGLFramebufferObjectFormat buffFormat;
	buffFormat.setAttachment(QOpenGLFramebufferObject::Depth);
	buffFormat.setInternalTextureFormat(GL_RGBA8);
	buffFormat.setSamples(4);

	m_pFrameBuffer = new QOpenGLFramebufferObject(rQVR.m_aOvrFrameSize[0], rQVR.m_aOvrFrameSize[1], buffFormat);
}
