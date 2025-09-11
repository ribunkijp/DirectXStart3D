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
    void Rotate(float deltaYaw, float deltaPitch);
    void UpdateViewMatrix();
    void UpdateThirdPerson(const DirectX::XMFLOAT3& playerPosition, float playerYaw);
  ;

private:
    

private:
    DirectX::XMFLOAT3 m_position = { 6.0f, 2.0f, -10.0f };
    DirectX::XMFLOAT3 m_upDirection = { 0.0f, 1.0f, 0.0f };
    DirectX::XMFLOAT4X4 m_view = {};

    float m_yaw = 0.0f;   // 绕Y轴旋转
    float m_pitch = 0.0f; // 绕X轴旋转




};


#endif
