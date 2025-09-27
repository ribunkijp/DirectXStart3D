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
#include <vector>
#include <string>

class Sword;

enum class PlayerState {
	Idle,
	Run,
	Walk,
	Attack0,
	AttackRo,
	JumpStart,
	JumpAir,
	JumpFall
};

struct Mesh
{
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	UINT indexCount = 0;
	UINT materialIndex = 0;
};
struct Material
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseTexture;
};
struct Bone // 一个骨骼
{
	std::string name;
	DirectX::XMFLOAT4X4 offsetMatrix; // 逆绑定姿态矩阵
	int parentIndex;
};
struct Skeleton // 骨架
{
	std::vector<Bone> bones;
};
struct Keyframe // 动画中的一个关键帧
{
	float timeStamp;
	DirectX::XMFLOAT3 translation;
	DirectX::XMFLOAT4 rotationQuaternion;
	DirectX::XMFLOAT3 scale;
};
struct BoneAnimation // 一个骨骼在一段动画中的所有关键帧
{
	std::string boneName;
	std::vector<Keyframe> keyframes;
};
struct AnimationClip // 完整动画
{
	std::string name;
	float duration;
	float ticksPerSecond;
	bool isLooping = true;
	std::vector<BoneAnimation> channels;
};


struct SkinningCB
{
	DirectX::XMFLOAT4X4 boneTransforms[128];
};



class Player {
public:
	Player();
	~Player();

	bool Load(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		const std::string& basePath,
		ID3D11InputLayout* inputLayout,
		ID3D11VertexShader* vertexShader,
		ID3D11PixelShader* pixelShader
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

	void SetState(PlayerState newState);

	PlayerState GetCurrentState() const;

	bool IsAnimationFinished() const;
	void ResetAnimationFinishedFlag();

	bool IsGrounded() const;
	void ApplyGravity(float deltaTime);

private:
	void UpdateConstantBuffer(ID3D11DeviceContext* context,
		const DirectX::XMMATRIX& view,
		const DirectX::XMMATRIX& projection,
		const DirectX::XMFLOAT4& tintColor);
	void UpdateAnimation(float deltaTime);
	int FindKeyframeIndexBefore(float animationTime, const std::vector<Keyframe>& keyframes);
	

private:
	std::vector<Mesh> m_meshes;
	std::vector<Material> m_materials;

	Skeleton m_skeleton;
	std::vector<AnimationClip> m_animations;

	int m_currentAnimationClipIndex = -1;
	float m_animationTime = 0.0f; 
	std::vector<DirectX::XMFLOAT4X4> m_finalBoneMatrices; // 最终骨骼矩阵

	DirectX::XMFLOAT3 m_position{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_rotation{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_scale{ 1.0f, 1.0f, 1.0f };

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_skinningConstantBuffer;

	DirectX::XMFLOAT3 m_velocity = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_targetVelocity = { 0.0f, 0.0f, 0.0f };

	std::unique_ptr<Sword> m_sword;

	std::vector<DirectX::XMFLOAT4X4> m_globalTransforms;

	PlayerState m_currentState = PlayerState::Run;

	int m_previousAnimationClipIndex = -1;
	float m_previousAnimationTime = 0.0f;
	float m_blendFactor = 1.0f;
	float m_blendDuration = 0.25f;

	bool m_isCurrentAnimationFinished = false;

	bool m_isGrounded = false;
	float m_gravity = -18.0f;
	float m_jumpForce = 7.0f;
	float m_verticalVelocity = 0.0f;// y

 };


#endif
