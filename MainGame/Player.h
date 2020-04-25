#pragma once

#include "GameObject.h"
#include "Camera.h"

class Player : public GameObject
{
public:
	Player(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature);
	~Player();

protected:
	Camera			*m_Camera = NULL;

	int				m_Direction = 0;
	HWND			m_hWnd = NULL;
	POINT			m_OldCursorPos{};

public:
	void SetDirection(int Direction) { m_Direction = Direction; }
	void SetCursorPos(POINT CursourPos) { m_OldCursorPos = CursourPos; }

	void CreateCameraSet(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList);
	void UpdateCameraSet(ID3D12GraphicsCommandList *CommandList);
	
	virtual void SetRotate(float Pitch, float Yaw, float Roll);

	void Move(float ElapsedTime);

	virtual void Update(float ElapsedTime);
	virtual void Render(ID3D12GraphicsCommandList *CommandList);
};

