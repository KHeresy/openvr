#include "QtOpenVR.h"

#include <QMessageBox>

QtOpenVR::QtOpenVR(QWidget *parent): m_glBall(this)
{
	m_pVRSystem			= nullptr;
	m_pResolveBuffer	= nullptr;

	m_aClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	m_fNearDistance = 0.1f;
	m_fFarDistance = 5.0f;

	m_aEyeDaya[0].m_eIndex = vr::Eye_Left;
	m_aEyeDaya[1].m_eIndex = vr::Eye_Right;

	m_glBall.buildBall(1,50,50);
}

QtOpenVR::~QtOpenVR()
{
}

void QtOpenVR::initializeGL()
{
	initializeOpenGLFunctions();

	#pragma region Qt OpenGL Logger
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
	#pragma endregion

	m_glBall.initializeGL(QOpenGLWidget::context());

	glClearColor(m_aClearColor[0], m_aClearColor[1], m_aClearColor[2], m_aClearColor[3]);
	glEnable(GL_DEPTH_TEST);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	initializeVR();
}

void QtOpenVR::releaseGL()
{
	makeCurrent();

	releaseVR();
	m_glBall.releaseGL();

	m_Logger->stopLogging();
	delete m_Logger;
	m_Logger = nullptr;

	doneCurrent();
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

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			m_glBall.render(rEyeData.m_matProjection, rEyeData.m_matPose*m_matHMDPose, 0);

			rEyeData.m_pFrameBuffer->release();

			QOpenGLFramebufferObject::blitFramebuffer(m_pResolveBuffer, rEyeData.m_rectTarget, rEyeData.m_pFrameBuffer, rectSource);
		}

		submitBuffer();
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, width(), height());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//glDisable(GL_MULTISAMPLE);
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0, 1.0, 1.0);

	glBindTexture(GL_TEXTURE_2D, m_pResolveBuffer->texture());

	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex3f(-1, -1, 0);
	glTexCoord2f(1, 0);
	glVertex3f(1, -1, 0);
	glTexCoord2f(1, 1);
	glVertex3f(1, 1, 0);
	glTexCoord2f(0, 1);
	glVertex3f(-1, 1,0);
	glEnd();

	update();
}

bool QtOpenVR::initializeVR()
{
	vr::EVRInitError error = vr::VRInitError_None;
	m_pVRSystem = vr::VR_Init(&error, vr::VRApplication_Scene);

	if (error == vr::VRInitError_None)
	{
		// get frame buffers for eyes
		m_pVRSystem->GetRecommendedRenderTargetSize(&m_aOvrFrameSize[0], &m_aOvrFrameSize[1]);
	}
	else
	{
		m_pVRSystem = nullptr;

		QString message = vr::VR_GetVRInitErrorAsEnglishDescription(error);
		qCritical() << message;
		QMessageBox::critical(this, "Unable to init VR", message);
		
		m_aOvrFrameSize = { 512,512 };
	}

	// get eye matrices
	for (auto& rEyeData : m_aEyeDaya)
		rEyeData.initialize(*this);

	m_aEyeDaya[0].m_rectTarget = QRect(0, 0, m_aOvrFrameSize[0], m_aOvrFrameSize[1]);
	m_aEyeDaya[1].m_rectTarget = QRect(m_aOvrFrameSize[0], 0, m_aOvrFrameSize[0], m_aOvrFrameSize[1]);

	QOpenGLFramebufferObjectFormat resolveFormat;
	resolveFormat.setInternalTextureFormat(GL_RGBA8);

	m_pResolveBuffer = new QOpenGLFramebufferObject(m_aOvrFrameSize[0] * 2, m_aOvrFrameSize[1], resolveFormat);

	// turn on compositor
	if (m_pVRSystem == nullptr || !vr::VRCompositor())
	{
		QString message = "Compositor initialization failed. See log file for details";
		qCritical() << message;
		QMessageBox::critical(this, "Unable to init VR", message);
		return false;
	}

	return true;
}

void QtOpenVR::releaseVR()
{
	if (m_pVRSystem)
	{
		vr::VR_Shutdown();
		m_pVRSystem = nullptr;

		for (auto& rEyeData : m_aEyeDaya)
			rEyeData.release();

		delete m_pResolveBuffer;
		m_pResolveBuffer = nullptr;
	}
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
	if (rQVR.m_pVRSystem != nullptr)
	{
		m_matProjection = toQMatrix4(rQVR.m_pVRSystem->GetProjectionMatrix(m_eIndex, rQVR.m_fNearDistance, rQVR.m_fFarDistance));
		m_matPose = toQMatrix4(rQVR.m_pVRSystem->GetEyeToHeadTransform(m_eIndex)).inverted();
	}

	QOpenGLFramebufferObjectFormat buffFormat;
	buffFormat.setAttachment(QOpenGLFramebufferObject::Depth);
	buffFormat.setInternalTextureFormat(GL_RGBA8);
	buffFormat.setSamples(4);

	m_pFrameBuffer = new QOpenGLFramebufferObject(rQVR.m_aOvrFrameSize[0], rQVR.m_aOvrFrameSize[1], buffFormat);
}
