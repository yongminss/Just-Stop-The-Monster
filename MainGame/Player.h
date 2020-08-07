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

	float           m_fPitch;
	float  			m_fYaw;
	float           m_fRoll;

	bool			m_MoveUp = false;
	bool			m_MoveDown = false;
	bool			m_MoveLeft = false;
	bool			m_MoveRight = false;

	HWND			m_hWnd = NULL;
	POINT			m_OldCursorPos{};

public:
	
	Camera *GetCamera() { return(m_Camera); }

	void SetMoveUp(bool IsTrue) { m_MoveUp = IsTrue; }
	void SetMoveDown(bool IsTrue) { m_MoveDown = IsTrue; }
	void SetMoveLeft(bool IsTrue) { m_MoveLeft = IsTrue; }
	void SetMoveRight(bool IsTrue) { m_MoveRight = IsTrue; }
	void SetCursorPos(POINT CursourPos) { m_OldCursorPos = CursourPos; }

	bool GetMoveInfo() { bool IsTrue = false; (m_MoveUp || m_MoveDown || m_MoveLeft || m_MoveRight) ? IsTrue = true : IsTrue = false; return IsTrue; }

	void SetMoveReturn(float ElapsedTime);

	bool GetForwardInfo() { bool IsTrue = false; (m_MoveUp && !m_MoveDown && !m_MoveLeft && !m_MoveRight) ? IsTrue = true : IsTrue = false; return IsTrue; }

	void CreateCameraSet(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList);
	void UpdateCameraSet(ID3D12GraphicsCommandList *CommandList);
	
	virtual void SetRotate(float Pitch, float Yaw, float Roll);

	void PRotate(float x, float y, float z);
	void CamRotate(float x, float y, float z);
	
	virtual void MoveForward(float Distance);
	virtual void MoveRight(float Distance);

	void Move(float ElapsedTime);

	virtual void OnPrepareRender();

	virtual void Update(float ElapsedTime);
	virtual void Render(ID3D12GraphicsCommandList *CommandList);
};

