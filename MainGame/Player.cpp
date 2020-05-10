#include "stdafx.h"
#include "Player.h"

Player::Player(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature)
{
	GameObject* Model = LoadGeometryAndAnimationFromFile(Device, CommandList, GraphicsRootSignature, "Model/Soldier_Player.bin", NULL, true);
	SetChild(Model, false);

	m_Right = XMFLOAT3(1.f, 0.f, 0.f);
	m_Up = XMFLOAT3(0.f, 1.f, 0.f);
	m_Look = XMFLOAT3(0.f, 0.f, 1.f);
	m_Position = XMFLOAT3(-1200.f, -15.f, 100.f);

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
	if (Yaw != 0.f) {
		XMMATRIX Rotate = XMMatrixRotationAxis(XMLoadFloat3(&m_Up), XMConvertToRadians(Yaw));
		m_Look = Vector3::TransformNormal(m_Look, Rotate);
		m_Right = Vector3::TransformNormal(m_Right, Rotate);

		m_Camera->SetRotate(-Pitch, -Yaw, -Roll);
	}
}

void Player::MoveForward(float Distance)
{
	XMFLOAT3 Shift = XMFLOAT3(0.f, 0.f, 0.f);
	Shift = Vector3::Add(Shift, m_Look, Distance);

	m_Position = Vector3::Add(m_Position, Shift);
}

void Player::MoveRight(float Distance)
{
	XMFLOAT3 Shift = XMFLOAT3(0.f, 0.f, 0.f);
	Shift = Vector3::Add(Shift, m_Right, Distance);

	m_Position = Vector3::Add(m_Position, Shift);
}

void Player::Move(float ElapsedTime)
{
	switch (m_Direction) {
	case 1:
		MoveForward(250.f * ElapsedTime);
		break;

	case 2:
		MoveForward(-(250.f * ElapsedTime));
		break;

	case 3:
		MoveRight(-(250.f * ElapsedTime));
		break;

	case 4:
		MoveRight(250.f * ElapsedTime);
		break;

	default:
		break;
	}
}

void Player::OnPrepareRender()
{
	m_TransformPos._11 = m_Right.x, m_TransformPos._12 = m_Right.y, m_TransformPos._13 = m_Right.z;
	m_TransformPos._21 = m_Up.x, m_TransformPos._22 = m_Up.y, m_TransformPos._23 = m_Up.z;
	m_TransformPos._31 = m_Look.x, m_TransformPos._32 = m_Look.y, m_TransformPos._33 = m_Look.z;
	m_TransformPos._41 = m_Position.x, m_TransformPos._42 = m_Position.y, m_TransformPos._43 = m_Position.z;

	UpdateTransform(NULL);
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