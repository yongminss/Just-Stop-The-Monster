#include "stdafx.h"
#include "Player.h"

Player::Player(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature)
{
	GameObject* Model = LoadGeometryAndAnimationFromFile(Device, CommandList, GraphicsRootSignature, "Model/Soldier_Player.bin", NULL, true);
	SetChild(Model, false);

	m_Right = XMFLOAT3(1.f, 0.f, 0.f);
	m_Up = XMFLOAT3(0.f, 1.f, 0.f);
	m_Look = XMFLOAT3(0.f, 0.f, 1.f);

	m_Position = XMFLOAT3(0.f, 0.f, 0.f);

	m_CamRight = XMFLOAT3(1.f, 0.f, 0.f);
	m_CamUp = XMFLOAT3(0.f, 1.f, 0.f);
	m_CamLook = XMFLOAT3(0.f, 0.f, 1.f);

	m_Camera = new Camera();

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	CreateCameraSet(Device, CommandList);
}

Player::~Player()
{

}

void Player::SetMoveReturn(float ElapsedTime)
{
	if (m_MoveUp) MoveForward(-260.f * ElapsedTime);
	if (m_MoveDown) MoveForward((260.f * ElapsedTime));
	if (m_MoveLeft) MoveRight((260.f * ElapsedTime));
	if (m_MoveRight) MoveRight(-(260.f * ElapsedTime));
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

void Player::CamRotate(float x, float y, float z)
{
	if (y != 0.0f) {
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_Up), XMConvertToRadians(y));
		m_CamLook = Vector3::TransformNormal(m_CamLook, xmmtxRotate);
		m_CamRight = Vector3::TransformNormal(m_CamRight, xmmtxRotate);
		m_CamUp = Vector3::TransformNormal(m_CamUp, xmmtxRotate);
	}
	if (x != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_CamRight), XMConvertToRadians(x));
		m_CamLook = Vector3::TransformNormal(m_CamLook, xmmtxRotate);
		m_CamUp = Vector3::TransformNormal(m_CamUp, xmmtxRotate);
	}
	m_CamLook = Vector3::Normalize(m_CamLook);
	m_CamRight = Vector3::CrossProduct(m_CamUp, m_CamLook, true);
	m_CamUp = Vector3::CrossProduct(m_CamLook, m_CamRight, true);
}

void Player::PRotate(float x, float y, float z)
{
	/*���� x-���� �߽����� ȸ���ϴ� ���� ���� �յڷ� ���̴� ���ۿ� �ش��Ѵ�. �׷��Ƿ� x-���� �߽����� ȸ���ϴ�
	������ -89.0~+89.0�� ���̷� �����Ѵ�. x�� ������ m_fPitch���� ���� ȸ���ϴ� �����̹Ƿ� x��ŭ ȸ���� ����
	Pitch�� +89�� ���� ũ�ų� -89�� ���� ������ m_fPitch�� +89�� �Ǵ� -89���� �ǵ��� ȸ������(x)�� �����Ѵ�.*/
	if (x != 0.0f)
	{
		m_fPitch += x;
		//cout << "before x: " << x << " Pitch" << m_fPitch << endl;
		if (m_fPitch > +30.0f) { x -= (m_fPitch - 30.0f); m_fPitch = 30.0f; }
		if (m_fPitch < -10.0f) { x -= (m_fPitch + 10.0f); m_fPitch = -10.0f; }
		//cout << "after x: " << x <<" Pitch" << m_fPitch << endl;
	}
	if (y != 0.0f)
	{
		//���� y-���� �߽����� ȸ���ϴ� ���� ������ ������ ���̹Ƿ� ȸ�� ������ ������ ����. 
		m_fYaw += y;
		if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
		if (m_fYaw < 0.0f) m_fYaw += 360.0f;
	}

	//ī�޶� x, y, z ��ŭ ȸ���Ѵ�. �÷��̾ ȸ���ϸ� ī�޶� ȸ���ϰ� �ȴ�. m_pCamera->Rotate(x, y, z);
	/*�÷��̾ ȸ���Ѵ�. 1��Ī ī�޶� �Ǵ� 3��Ī ī�޶󿡼� �÷��̾��� ȸ���� ���� y-�࿡���� �Ͼ��. �÷��̾�
	�� ���� y-��(Up ����)�� �������� ���� z-��(Look ����)�� ���� x-��(Right ����)�� ȸ����Ų��. �⺻������ Up ��
	�͸� �������� ȸ���ϴ� ���� �÷��̾ �ȹٷ� ���ִ� ���� �����Ѵٴ� �ǹ��̴�.*/
	
	if (y != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_Up), XMConvertToRadians(y));
		m_Look = Vector3::TransformNormal(m_Look, xmmtxRotate);
		m_Right = Vector3::TransformNormal(m_Right, xmmtxRotate);
	}
	m_Look = Vector3::Normalize(m_Look);
	m_Right = Vector3::CrossProduct(m_Up, m_Look, true);
	m_Up = Vector3::CrossProduct(m_Look, m_Right, true);

	//cout << "Right x: " << m_Right.x << " y: " << m_Right.y << " z: " << m_Right.z << endl;
	//if (x != 0.0f) {
	CamRotate(x, y, z);
}

void Player::SetRotate(float Pitch, float Yaw, float Roll)
{
	if (Pitch != 0.f) {
		XMMATRIX Rotate = XMMatrixRotationAxis(XMLoadFloat3(&m_Right), XMConvertToRadians(Pitch));
		m_Look = Vector3::TransformNormal(m_Look, Rotate);
		m_Up = Vector3::TransformNormal(m_Up, Rotate);
	}
	if (Yaw != 0.f) {
		XMMATRIX Rotate = XMMatrixRotationAxis(XMLoadFloat3(&m_Up), XMConvertToRadians(Yaw));
		m_Look = Vector3::TransformNormal(m_Look, Rotate);
		m_Right = Vector3::TransformNormal(m_Right, Rotate);
	}
	m_Look = Vector3::Normalize(m_Look);
	m_Right = Vector3::CrossProduct(m_Up, m_Look, true);
	m_Up = Vector3::CrossProduct(m_Look, m_Right, true);
	//m_Camera->SetRotate(-Pitch, -Yaw, -Roll);
}

void Player::MoveForward(float Distance)
{
	XMFLOAT3 Shift = XMFLOAT3(0.f, 0.f, 0.f);

	if (GetNowAnimationNum() == 27) Distance *= 2;
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
	if (m_MoveUp && m_MoveDown) m_MoveUp = false, m_MoveDown = false;
	if (m_MoveLeft && m_MoveRight) m_MoveLeft = false, m_MoveRight = false;

	if (m_MoveUp) {
		if (m_MoveLeft) {
			ElapsedTime *= 1.5f;
			MoveForward(125.f * ElapsedTime);
			MoveRight(-(125.f * ElapsedTime));
		}
		else if (m_MoveRight) {
			ElapsedTime *= 1.5f;
			MoveForward(125.f * ElapsedTime);
			MoveRight(125.f * ElapsedTime);
		}
		else {
			MoveForward(250.f * ElapsedTime);
		}
	}
	else if (m_MoveDown) {
		if (m_MoveLeft) {
			ElapsedTime *= 1.5f;
			MoveForward(-(125.f * ElapsedTime));
			MoveRight(-(125.f * ElapsedTime));
		}
		else if (m_MoveRight) {
			ElapsedTime *= 1.5f;
			MoveForward(-(125.f * ElapsedTime));
			MoveRight(125.f * ElapsedTime);
		}
		else {
			MoveForward(-(250.f * ElapsedTime));
		}
	}
	else if (m_MoveLeft) {
		MoveRight(-(250.f * ElapsedTime));
	}
	else if (m_MoveRight) {
		MoveRight(250.f * ElapsedTime);
	}

	if (m_MoveUp) {
		if (m_MoveLeft) {
			SetEnable(6);
		}
		else if (m_MoveRight) {
			SetEnable(5);
		}
		else {
			SetEnable(1);
		}
	}
	else if (m_MoveDown) {
		if (m_MoveLeft) {
			SetEnable(8);
		}
		else if (m_MoveRight) {
			SetEnable(7);
		}
		else {
			SetEnable(2);
		}
	}
	else if (m_MoveLeft) {
		SetEnable(4);
	}
	else if (m_MoveRight) {
		SetEnable(3);
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
	// Player�� �̵� ���̸� Move �Լ� ȣ�� -> �̵��� ������ IDLE ���·� �ִϸ��̼� ��ȯ
	if (GetMoveInfo()) 
		Move(ElapsedTime);
	else 
		SetEnable(0);
	Animate(ElapsedTime);
	
	//cout << "Position : " << m_TransformPos._41 << ", " << m_TransformPos._42 << ", " << m_TransformPos._43 << endl;
}

void Player::Render(ID3D12GraphicsCommandList *CommandList)
{
	GameObject::Render(CommandList);
}