#include "stdafx.h"
#include "Camera.h"
#include "Player.h"

Camera::Camera()
{
	m_View = Matrix4x4::Identity();
	m_Projection = Matrix4x4::Identity();
	m_Viewport = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.f, 1.f };
	m_ScissorRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
	m_Right = XMFLOAT3(1.f, 0.f, 0.f);
	m_Up = XMFLOAT3(0.f, 1.f, 0.f);
	m_Look = XMFLOAT3(0.f, 0.f, 1.f);
	m_Position = XMFLOAT3(0.f, 0.f, 0.f);
	m_Pitch = 0.f;
	m_Yaw = 0.f;
	m_Roll = 0.f;
	m_LookAtWorld = XMFLOAT3(0.f, 0.f, 0.f);
	m_Offset = XMFLOAT3(0.f, 0.f, 0.f);
	m_TimeLeg = 0.f;
	m_Player = NULL;
}

Camera::~Camera()
{
	
}

void Camera::SetViewport(float TopLeftX, float TopLeftY, float Width, float Height, float MinDepth, float MaxDepth)
{
	m_Viewport.TopLeftX = TopLeftX;
	m_Viewport.TopLeftY = TopLeftY;
	m_Viewport.Width = Width;
	m_Viewport.Height = Height;
	m_Viewport.MinDepth = MinDepth;
	m_Viewport.MaxDepth = MaxDepth;
}

void Camera::SetScissorRect(LONG Left, LONG Top, LONG Right, LONG Bottom)
{
	m_ScissorRect.left = Left;
	m_ScissorRect.top = Top;
	m_ScissorRect.right = Right;
	m_ScissorRect.bottom = Bottom;
}

void Camera::SetLookAt(XMFLOAT3& LookAt)
{
	XMFLOAT4X4 MatrixLookAt = Matrix4x4::LookAtLH(m_Position, LookAt, m_Player->GetUp());

	m_Right = XMFLOAT3(MatrixLookAt._11, MatrixLookAt._21, MatrixLookAt._31);
	m_Up = XMFLOAT3(MatrixLookAt._12, MatrixLookAt._22, MatrixLookAt._32);
	m_Look = XMFLOAT3(MatrixLookAt._13, MatrixLookAt._23, MatrixLookAt._33);
}

void Camera::GenerateProjectionMatrix(float NearPlaneDistance, float FarPlaneDistance, float AspectRatio, float FoVAngle)
{
	m_Projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(FoVAngle), AspectRatio, NearPlaneDistance, FarPlaneDistance);
}

void Camera::RegenerateViewMatrix()
{
	m_Look = Vector3::Normalize(m_Look);
	m_Right = Vector3::CrossProduct(m_Up, m_Look, true);
	m_Up = Vector3::CrossProduct(m_Look, m_Right, true);

	m_View._11 = m_Right.x; m_View._12 = m_Up.x; m_View._13 = m_Look.x;
	m_View._21 = m_Right.y; m_View._22 = m_Up.y; m_View._23 = m_Look.y;
	m_View._31 = m_Right.z; m_View._32 = m_Up.z; m_View._33 = m_Look.z;

	m_View._41 = -Vector3::DotProduct(m_Position, m_Right);
	m_View._42 = -Vector3::DotProduct(m_Position, m_Up);
	m_View._43 = -Vector3::DotProduct(m_Position, m_Look);
}

void Camera::SetViewportAndScissorRect(ID3D12GraphicsCommandList *CommandList)
{
	CommandList->RSSetViewports(1, &m_Viewport);
	CommandList->RSSetScissorRects(1, &m_ScissorRect);
}

void Camera::CreateShaderVariable(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList)
{
	UINT ncbElementByte = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255);
	m_cbCamera = ::CreateBufferResource(Device, CommandList, NULL, ncbElementByte, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cbCamera->Map(0, NULL, (void**)&m_MappedCamera);
}

void Camera::UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList)
{
	XMFLOAT4X4 View;
	XMStoreFloat4x4(&View, XMMatrixTranspose(XMLoadFloat4x4(&m_View)));
	::memcpy(&m_MappedCamera->m_View, &View, sizeof(XMFLOAT4X4));

	XMFLOAT4X4 Projection;
	XMStoreFloat4x4(&Projection, XMMatrixTranspose(XMLoadFloat4x4(&m_Projection)));
	::memcpy(&m_MappedCamera->m_Projection, &Projection, sizeof(XMFLOAT4X4));

	::memcpy(&m_MappedCamera->m_Position, &m_Position, sizeof(XMFLOAT3));

	D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress = m_cbCamera->GetGPUVirtualAddress();
	CommandList->SetGraphicsRootConstantBufferView(0, GpuVirtualAddress);
}

void Camera::SetCameraOption()
{
	GenerateProjectionMatrix(1.01f, 5000.f, ASPECT_RATIO, 60.f);
	SetViewport(0.f, 0.f, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.f, 1.f);
	SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	SetOffset(XMFLOAT3(0.f, 50.f, -150.f));
}

void Camera::SetRotate(float Pitch, float Yaw, float Roll)
{
	XMMATRIX Rotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(Pitch), XMConvertToRadians(Yaw), XMConvertToRadians(Roll));
	m_Projection = Matrix4x4::Multiply(Rotate, m_Projection);
}

void Camera::Update(XMFLOAT3& LookAt, float ElapsedTime)
{
	if (m_Player) {
		XMFLOAT4X4 Rotate = Matrix4x4::Identity();

		XMFLOAT3 Right = m_Player->GetRight();
		XMFLOAT3 Up = m_Player->GetUp();
		XMFLOAT3 Look = m_Player->GetLook();

		Rotate._11 = Right.x, Rotate._12 = Right.y, Rotate._13 = Right.z;
		Rotate._21 = Up.x, Rotate._22 = Up.y, Rotate._23 = Up.z;
		Rotate._31 = Look.x, Rotate._32 = Look.y, Rotate._33 = Look.z;

		XMFLOAT3 Offset = Vector3::TransformCoord(m_Offset, Rotate);
		XMFLOAT3 Position = Vector3::Add(m_Player->GetPosition(), Offset);
		XMFLOAT3 Direction = Vector3::Subtract(Position, m_Position);
		float Length = Vector3::Length(Direction);
		Direction = Vector3::Normalize(Direction);
		float TimeLegScale = (m_TimeLeg) ? ElapsedTime * (1.f / m_TimeLeg) : 1.f;
		float Distance = Length * TimeLegScale;

		if (Distance > Length) Distance = Length;
		if (Length < 0.01f) Distance = Length;
		if (Distance > 0.f) {
			m_Position = Vector3::Add(m_Position, Direction, Distance);
		}
	}
}