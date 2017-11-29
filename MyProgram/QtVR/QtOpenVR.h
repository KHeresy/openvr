#pragma once

// Standard Library
#include <array>
#include <map>

// OpenVR Heeader
#include <openvr.h>

// Qt Header
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLWidget>
#include <QMatrix4x4>
#include <QOpenGLFramebufferObject>
#include <QOpenGLBuffer>
#include <QOpenGLDebugLogger>

class QtOpenVR : public QOpenGLWidget, public QOpenGLFunctions_4_1_Core
{
	Q_OBJECT

public:
	QtOpenVR(QWidget *parent);
	~QtOpenVR();

protected:
	void initializeGL();
	void releaseGL();
	void resizeGL(int w, int h);
	void paintGL();

protected:
	bool initializeVR();
	void releaseVR();
	void updatePose();
	void submitBuffer();

	inline static QMatrix4x4 toQMatrix4(const vr::HmdMatrix34_t &mat)
	{
		return QMatrix4x4(
			mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
			mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
			mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
			0.0, 0.0, 0.0, 1.0f
		);
	}

	inline static QMatrix4x4 toQMatrix4(const vr::HmdMatrix44_t &mat)
	{
		return QMatrix4x4(
			mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
			mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
			mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
			mat.m[3][0], mat.m[3][1], mat.m[3][2], mat.m[3][3]
		);
	}

protected:
	class CEyeData
	{
	public:
		vr::EVREye					m_eIndex;
		QMatrix4x4					m_matPose;
		QMatrix4x4					m_matProjection;
		QOpenGLFramebufferObject*	m_pFrameBuffer;
		QRect						m_rectTarget;

	public:
		CEyeData()
		{
			m_pFrameBuffer = nullptr;
		}

		~CEyeData()
		{
			release();
		}

		void initialize(QtOpenVR& rQVR);

		void release()
		{
			delete m_pFrameBuffer;
			m_pFrameBuffer = nullptr;
		}
	};

protected:
	vr::IVRSystem*				m_pVRSystem;

	vr::TrackedDevicePose_t		m_aOvrDevicePose[vr::k_unMaxTrackedDeviceCount];
	std::array<uint32_t, 2>		m_aOvrFrameSize;
	QMatrix4x4					m_aDevicePose[vr::k_unMaxTrackedDeviceCount];
	QMatrix4x4					m_matHMDPose;

	std::array<CEyeData, 2>		m_aEyeDaya;
	QOpenGLFramebufferObject*	m_pResolveBuffer;

	QOpenGLDebugLogger		*m_Logger;

public:
	std::array<float,4> m_aClearColor;
	float	m_fNearDistance;
	float	m_fFarDistance;
};
