#include "stdafx.h"
#include "Player.h"

Player::Player(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature)
{
	GameObject* Model = LoadGeometryAndAnimationFromFile(Device, CommandList, GraphicsRootSignature, "Model/weak_infantry.bin", NULL, true);
	SetChild(Model, true);
	
	m_Right = XMFLOAT3(1.f, 0.f, 0.f);
	m_Up = XMFLOAT3(0.f, 1.f, 0.f);
	m_Look = XMFLOAT3(0.f, 0.f, 1.f);
	m_Position = XMFLOAT3(0.f, 0.f, 0.f);

	m_Camera = new Camera();

	CreateCameraSet(Device, CommandList);
}

Player::~Player()
{

}

void Player::CreateCameraSet(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList)
{
	if (m_Camera) {
		m_Camera->SetCameraOption();
		m_Camera->CreateShaderVariable(Device, CommandList);
		m_Camera->SetPlayer(this);
	}
}

void Player::UpdateCameraSet(ID3D12GraphicsCommandList *CommandList)
{
	if (m_Camera) {
		m_Camera->SetViewportAndScissorRect(CommandList);
		m_Camera->UpdateShaderVariable(CommandList);
	}
}

void Player::SetRotate(float Pitch, float Yaw, float Roll)
{
	XMMATRIX Rotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(Pitch), XMConvertToRadians(Yaw), XMConvertToRadians(Roll));
	m_TransformPos = Matrix4x4::Multiply(Rotate, m_TransformPos);

	UpdateTransform(NULL);
}

void Player::Move(float ElapsedTime)
{
	switch (m_Direction) {
	case 1:
		GameObject::MoveForward(250.f * ElapsedTime);
		break;

	case 2:
		GameObject::MoveForward(-(250.f * ElapsedTime));
		break;

	case 3:
		GameObject::MoveRight(-(250.f * ElapsedTime));
		break;

	case 4:
		GameObject::MoveRight(250.f * ElapsedTime);
		break;

	default:
		break;
	}
}

void Player::Update(float ElapsedTime)
{
	if (m_Camera) {
		m_Camera->Update(GetPosition(), ElapsedTime);
		m_Camera->RegenerateViewMatrix();
	}
	if (m_Direction != 0) Move(ElapsedTime);
	Animate(ElapsedTime);
}

void Player::Render(ID3D12GraphicsCommandList *CommandList)
{
	GameObject::Render(CommandList);
}