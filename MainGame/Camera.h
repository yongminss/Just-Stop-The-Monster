#pragma once

#define ASPECT_RATIO (float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))

struct VS_CB_CAMERA_INFO
{
	XMFLOAT4X4	m_View;
	XMFLOAT4X4	m_Projection;
	XMFLOAT3	m_Position;
};

class Player;

class Camera
{
public:
	Camera();
	~Camera();

protected:
	XMFLOAT3			m_Right;
	XMFLOAT3			m_Up;
	XMFLOAT3			m_Look;
	XMFLOAT3			m_Position;

	float				m_Pitch;
	float				m_Yaw;
	float				m_Roll;

	XMFLOAT3			m_LookAtWorld;
	XMFLOAT3			m_Offset;
	float				m_TimeLeg;

	XMFLOAT4X4			m_View;
	XMFLOAT4X4			m_Projection;

	D3D12_VIEWPORT		m_Viewport;
	D3D12_RECT			m_ScissorRect;

	Player				*m_Player;

	ID3D12Resource		*m_cbCamera = NULL;
	VS_CB_CAMERA_INFO	*m_MappedCamera = NULL;

private:


public:
	void SetViewport(float TopLeftX, float TopLeftY, float Width, float Height, float MinDepth, float MaxDepth);
	void SetScissorRect(LONG Left, LONG Top, LONG Right, LONG Bottom);
	void SetLookAt(XMFLOAT3& LookAt);

	void GenerateProjectionMatrix(float NearPlaneDistance, float FarPlaneDistance, float AspectRatio, float FoVAngle);

	void RegenerateViewMatrix();

	void SetViewportAndScissorRect(ID3D12GraphicsCommandList *CommandList);

	void CreateShaderVariable(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList);
	void UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList);

	void SetCameraOption();

	void SetPlayer(Player *Player) { m_Player = Player; }
	
	void SetOffset(XMFLOAT3 Offset) { m_Offset = Offset; }
	void SetPosition(XMFLOAT3 Position) { m_Position = Position; }
	void SetRotate(float Pitch, float Yaw, float Roll);

	XMFLOAT3& GetOffset() { return m_Offset; }

	void Update(XMFLOAT3& LookAt, float ElapsedTime);
};

