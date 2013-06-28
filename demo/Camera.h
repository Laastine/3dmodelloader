#ifndef CAMERA_H
#define CAMERA_H

#include "d3dUtil.h"

class Camera {
    public:
        ~Camera();

        static Camera* CameraInstance();

        D3DXVECTOR3& position();

        D3DXMATRIX getviewMatrix() const;
        D3DXMATRIX getprojectionMatrix() const;

        void setLens(float fovY, float aspect, float znear, float zfar);

        void strafe(float d);
        void walk(float d);

        void pitch(float angle);
        void rotateY(float angle);

        void rebuildView();

    protected:

    private:
        Camera();	//Private constructor so that it can't be called
        static Camera* cameraInstance;

        D3DXVECTOR3 m_position;
        D3DXVECTOR3 m_right;
        D3DXVECTOR3 m_up;
        D3DXVECTOR3 m_look;

        D3DXMATRIX m_view;
        D3DXMATRIX m_viewMatrix;
};

#endif //CAMERA_H