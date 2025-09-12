/**********************************************************************************
* Player.h
*
*
*
* LI WENHUI
* 2025/09/10
* *********************************************************************************/

#ifndef PLAYER_H
#define PLAYER_H

#include <d3d11.h>         
#include <DirectXMath.h>    
#include <wrl.h>  

class Player {
public:
	Player();
	~Player();

	bool Load(
		ID3D11Device* device,
		ID3D11DeviceContext* context
	);
	void Render(ID3D11DeviceContext* context, const DirectX::XMMATRIX& view,
		const DirectX::XMMATRIX& projection, const DirectX::XMFLOAT4& tintColor);
	void SetRotationY(float yaw);
	void SetTargetVelocity(const DirectX::XMFLOAT3& targetVelocity);
	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMFLOAT3 GetRotation() const;
	void Update(float deltaTime);

	DirectX::XMFLOAT3 GetVelocity() const;
	DirectX::XMFLOAT3 GetTargetVelocity() const;


private:
	void InitVertexData(ID3D11Device* device, ID3D11DeviceContext* context);
	void UpdateConstantBuffer(ID3D11DeviceContext* context,
		const DirectX::XMMATRIX& view,
		const DirectX::XMMATRIX& projection,
		const DirectX::XMFLOAT4& tintColor);

private:
	DirectX::XMFLOAT3 m_position{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_rotation{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_scale{ 1.0f, 1.0f, 1.0f };
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	UINT m_indexCount = 0;
	DirectX::XMFLOAT3 m_velocity = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_targetVelocity = { 0.0f, 0.0f, 0.0f };
};







#endif
