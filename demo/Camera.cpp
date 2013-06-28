#include "Camera.h"

Camera* Camera::cameraInstance = 0;

Camera* Camera::CameraInstance() {
    if(cameraInstance == 0)
    { cameraInstance = new Camera(); }

    return cameraInstance;
}

Camera::Camera() {
    m_position = D3DXVECTOR3(5.0f, 0.0f, -5.0f);		//DEFAULT 0.0f 0.0f 0.0f
    m_right    = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
    m_up       = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    m_look     = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

    D3DXMatrixIdentity(&m_view);
    D3DXMatrixIdentity(&m_viewMatrix);
}

Camera::~Camera() {}

D3DXVECTOR3& Camera::position() {
    return m_position;
}

D3DXMATRIX Camera::getviewMatrix() const {
    return m_view;
}

D3DXMATRIX Camera::getprojectionMatrix() const {
    return m_viewMatrix;
}

void Camera::setLens(float fovY, float aspect, float znear, float zfar) {
    D3DXMatrixPerspectiveFovLH(&m_viewMatrix, fovY, aspect, znear, zfar);
}

void Camera::strafe(float d) {
    m_position += d*m_right;
}

void Camera::walk(float d) {
    m_position += d*m_look;
}

void Camera::pitch(float angle) {
    D3DXMATRIX R;
    D3DXMatrixRotationAxis(&R, &m_right, angle);

    D3DXVec3TransformNormal(&m_up, &m_up, &R);
    D3DXVec3TransformNormal(&m_look, &m_look, &R);
}

void Camera::rotateY(float angle) {
    D3DXMATRIX R;
    D3DXMatrixRotationY(&R, angle);

    D3DXVec3TransformNormal(&m_right, &m_right, &R);
    D3DXVec3TransformNormal(&m_up, &m_up, &R);
    D3DXVec3TransformNormal(&m_look, &m_look, &R);
}

void Camera::rebuildView() {
    // Keep camera's axes orthogonal to each other and of unit length.
    D3DXVec3Normalize(&m_look, &m_look);

    D3DXVec3Cross(&m_up, &m_look, &m_right);
    D3DXVec3Normalize(&m_up, &m_up);

    D3DXVec3Cross(&m_right, &m_up, &m_look);
    D3DXVec3Normalize(&m_right, &m_right);

    // Fill in the view matrix entries.
    float x = -D3DXVec3Dot(&m_position, &m_right);
    float y = -D3DXVec3Dot(&m_position, &m_up);
    float z = -D3DXVec3Dot(&m_position, &m_look);

    m_view(0,0) = m_right.x;
    m_view(1,0) = m_right.y;
    m_view(2,0) = m_right.z;
    m_view(3,0) = x;

    m_view(0,1) = m_up.x;
    m_view(1,1) = m_up.y;
    m_view(2,1) = m_up.z;
    m_view(3,1) = y;

    m_view(0,2) = m_look.x;
    m_view(1,2) = m_look.y;
    m_view(2,2) = m_look.z;
    m_view(3,2) = z;

    m_view(0,3) = 0.0f;
    m_view(1,3) = 0.0f;
    m_view(2,3) = 0.0f;
    m_view(3,3) = 1.0f;
}
