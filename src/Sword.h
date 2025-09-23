/**********************************************************************************
* Sword.h
*
*
*
* LI WENHUI
* 2025/09/22
* *********************************************************************************/

#ifndef SWORD_H
#define SWORD_H

class Sword {
public:
	Sword();
	~Sword();

	bool Load(
		ID3D11Device* device, 
		const std::string& meshPath, 
		const std::string& offsetPath,
		ID3D11InputLayout* inputLayout,
		ID3D11VertexShader* vertexShader,
		ID3D11PixelShader* pixelShader
	);
	DirectX::XMMATRIX GetOffsetMatrix();
	void Render(ID3D11DeviceContext* context, const DirectX::XMMATRIX& worldMatrix, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& projection, const DirectX::XMFLOAT4& tintColor);

private:
	void UpdateConstantBuffer(ID3D11DeviceContext* context, const DirectX::XMMATRIX& worldMatrix, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& projection, const DirectX::XMFLOAT4& tintColor);


private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
	UINT m_indexCount = 0;

	DirectX::XMFLOAT4X4 m_offsetMatrix;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
};


#endif 
