/**********************************************************************************
* Ground.h
*
*
*
* LI WENHUI
* 2025/09/12
* *********************************************************************************/

#ifndef GROUND_H
#define GROUND_H


#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>


class Ground {
public:
	Ground();
	~Ground();

	bool Initialize(ID3D11Device* device, int numLines, float size);
	void Render(ID3D11DeviceContext* context, 
		const DirectX::XMMATRIX& view, 
		const DirectX::XMMATRIX& projection, 
		const DirectX::XMFLOAT4& tintColor);


private:
	void UpdateConstantBuffer(ID3D11DeviceContext* context,
		const DirectX::XMMATRIX& view,
		const DirectX::XMMATRIX& projection,
		const DirectX::XMFLOAT4& tintColor);


private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

	UINT m_indexCount = 0;
	DirectX::XMFLOAT4X4 m_worldMatrix;


};






#endif
