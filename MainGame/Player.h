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

public:
	void CreateCameraSet(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList);
	void UpdateCameraSet(ID3D12GraphicsCommandList *CommandList);

	virtual void Update(float ElapsedTime);
	virtual void Render(ID3D12GraphicsCommandList *CommandList);
};

