/**********************************************************************************
 * Camera.h
 *
 *
 *
 * LI WENHUI
 * 2025/09/09
 **********************************************************************************/

#ifndef CAMERA_H
#define CAMERA_H


class Camera {
public:
	Camera();
	~Camera();

    DirectX::XMMATRIX GetViewMatrix() const;



private:
    void UpdateViewMatrix();

private:
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_focusPoint;
    DirectX::XMFLOAT3 m_upDirection;
    DirectX::XMFLOAT4X4 m_viewMatrix;




};


#endif
