#include "stdafx.h"
#include "Mesh.h"
#include "GameObject.h"

Mesh::Mesh()
{

}

Mesh::Mesh(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList)
{

}

Mesh::~Mesh()
{
	if (m_VertexBuffer) m_VertexBuffer->Release();
	if (m_IndexBuffer) m_IndexBuffer->Release();
	if (m_VertexUploadBuffer) m_VertexUploadBuffer->Release();
}

void Mesh::Render(ID3D12GraphicsCommandList *CommandList)
{
	CommandList->IASetPrimitiveTopology(m_PrimitiveTopology);
	CommandList->IASetVertexBuffers(m_nSlot, 1, &m_VertexBufferView);

	if (m_IndexBuffer) {
		CommandList->IASetIndexBuffer(&m_IndexBufferView);
		CommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
	}
	else
		CommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);

}

void Mesh::Render(ID3D12GraphicsCommandList *CommandList, int nSubSet)
{
	UpdateShaderVariable(CommandList);

	OnPreRender(CommandList, NULL);

	CommandList->IASetPrimitiveTopology(m_PrimitiveTopology);

	if ((m_nSubMesh > 0) && (nSubSet < m_nSubMesh)) {
		CommandList->IASetIndexBuffer(&(m_SubSetIndexBufferView[nSubSet]));
		CommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else
		CommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
}

void Mesh::Render(ID3D12GraphicsCommandList *CommandList, UINT InstanceNum, D3D12_VERTEX_BUFFER_VIEW InstanceBufferView)
{
	OnPreRender(CommandList, NULL, InstanceBufferView);

	UpdateShaderVariable(CommandList);

	CommandList->IASetPrimitiveTopology(m_PrimitiveTopology);

	if (m_nSubMesh > 0) {
		CommandList->IASetIndexBuffer(&(m_SubSetIndexBufferView[0]));
		CommandList->DrawIndexedInstanced(m_pnSubSetIndices[0], InstanceNum, 0, 0, 0);
	}
	else
		CommandList->DrawInstanced(m_nVertices, InstanceNum, m_nOffset, 0);
}

CubeMesh::CubeMesh(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList)
{
	m_nVertices = 8;
	m_nStride = sizeof(DiffusedVertex);
	m_PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// 직육면체 - 꼭지점 8개의 정점
	DiffusedVertex Vertices[8];

	Vertices[0] = DiffusedVertex(XMFLOAT3(-100, +100, -100), XMFLOAT4(1.f, 0.f, 0.f, 1.f));
	Vertices[1] = DiffusedVertex(XMFLOAT3(+100, +100, -100), XMFLOAT4(1.f, 0.f, 0.f, 1.f));
	Vertices[2] = DiffusedVertex(XMFLOAT3(+100, +100, +100), XMFLOAT4(1.f, 0.f, 0.f, 1.f));
	Vertices[3] = DiffusedVertex(XMFLOAT3(-100, +100, +100), XMFLOAT4(1.f, 0.f, 0.f, 1.f));
	Vertices[4] = DiffusedVertex(XMFLOAT3(-100, -100, -100), XMFLOAT4(1.f, 0.f, 0.f, 1.f));
	Vertices[5] = DiffusedVertex(XMFLOAT3(+100, -100, -100), XMFLOAT4(1.f, 0.f, 0.f, 1.f));
	Vertices[6] = DiffusedVertex(XMFLOAT3(+100, -100, +100), XMFLOAT4(1.f, 0.f, 0.f, 1.f));
	Vertices[7] = DiffusedVertex(XMFLOAT3(-100, -100, +100), XMFLOAT4(1.f, 0.f, 0.f, 1.f));

	m_VertexBuffer = ::CreateBufferResource(Device, CommandList, Vertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_VertexUploadBuffer);

	m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = m_nStride;
	m_VertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}

// 텍스쳐 맵핑을 진행할 메쉬
TextureMesh::TextureMesh(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, float width, float height, float depth, float x, float y, float z, int type, int ImageType)
{
	m_nVertices = 6;
	m_nStride = sizeof(TextureVertex);
	m_PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	TextureVertex Vertices[6];

	float objX = width + x;
	float objY = height + y;
	float objZ = depth + z;

	float tempX = 0.f;
	float tempY = 0.f;
	// Title
	if (Title == ImageType) tempY = +0.5f;
	// Single
	if (Single == ImageType) tempY = -0.15f;
	// Muiti
	if (Multi == ImageType) tempY = -0.55f;
	// RoomList
	if (RoomList == ImageType) tempX = 0.0f, tempY = 0.0f;
	// Make Room
	if (MakeRoom == ImageType) tempX = 0.0f, tempY = 0.0f;
	// Join Room
	if (JoinRoom == ImageType) tempX = 0.0f, tempY = 0.0f;
	// Room1
	if (Room_1 == ImageType) tempX = 0.05f, tempY = 0.5f;
	// Room2
	if (Room_2 == ImageType) tempX = 0.05f, tempY = 0.15f;
	// Room3
	if (Room_3 == ImageType) tempX = 0.05f, tempY = -0.2f;
	// Room4
	if (Room_4 == ImageType) tempX = 0.05f, tempY = -0.55f;
	// Stage Select
	if (Select_Stage == ImageType) tempX = -0.5f,tempY = +0.3f;
	// Stage Left
	if (Stage_Left == ImageType) tempX = -0.5f, tempY = +0.3f;
	// Stage Right
	if (Stage_Right == ImageType) tempX = -0.5f, tempY = +0.3f;
	// Weapon and Skill Select
	if (Start_Button == ImageType) tempX = -0.5f, tempY = -0.4f;
	// Player Infomation
	if (PlayerInfo == ImageType) tempX = +0.5f, tempY = -0.1f;
	// Player 1
	if (Player_1 == ImageType) tempX = +0.5f, tempY = -0.1f;
	// Player 2
	if (Player_2 == ImageType) tempX = +0.5f, tempY = -0.3f;
	// My Player
	if (MyPlayer == ImageType) tempX = +0.5f, tempY = -0.3f;
	// Charactor Information
	if (UI_PlayerInfo == ImageType) tempX = -0.6f, tempY = +0.85f;
	// Traplist UI
	if (UI_TrapList == ImageType) tempY = -0.8f;
	// Back_Button
	if (Back_Button == ImageType) tempX = -0.7f, tempY = 0.7f;
	// Stage Num 1
	if (Num_1 == ImageType) tempX = -0.5f, tempY = 0.22f;
	// Stage Num 2
	if (Num_2 == ImageType) tempX = -0.5f, tempY = 0.22f;
	// Stage Num 3
	if (Num_3 == ImageType) tempX = -0.5f, tempY = 0.22f;
	// Stage Num 4
	if (Num_4 == ImageType) tempX = -0.5f, tempY = 0.22f;
	// HpBar UI
	if (UI_HpBar <= ImageType && ImageType <= UI_HpBar + 9) tempX = -0.735f + (float)(ImageType - UI_HpBar) * 0.051f, tempY = 0.897f;
	// Bullet UI
	if (UI_Bullet <= ImageType && ImageType <= UI_Bullet + 9) tempX = -0.735f, tempY = -0.9f + (float)(ImageType - UI_Bullet) * 0.05f;

	switch (type)
	{
	case 0: // 앞
	{
		Vertices[0] = TextureVertex(XMFLOAT3(+objX + tempX, +objY + tempY, objZ), XMFLOAT2(1.0f, 0.0f));
		Vertices[1] = TextureVertex(XMFLOAT3(+objX + tempX, -objY + tempY, objZ), XMFLOAT2(1.0f, 1.0f));
		Vertices[2] = TextureVertex(XMFLOAT3(-objX + tempX, -objY + tempY, objZ), XMFLOAT2(0.0f, 1.0f));

		Vertices[3] = TextureVertex(XMFLOAT3(-objX + tempX, -objY + tempY, objZ), XMFLOAT2(0.0f, 1.0f));
		Vertices[4] = TextureVertex(XMFLOAT3(-objX + tempX, +objY + tempY, objZ), XMFLOAT2(0.0f, 0.0f));
		Vertices[5] = TextureVertex(XMFLOAT3(+objX + tempX, +objY + tempY, objZ), XMFLOAT2(1.0f, 0.0f));
	}
	break;

	case 1: // 뒤
	{
		Vertices[0] = TextureVertex(XMFLOAT3(-objX, +objY, objZ), XMFLOAT2(1.0f, 0.0f));
		Vertices[1] = TextureVertex(XMFLOAT3(-objX, -objY, objZ), XMFLOAT2(1.0f, 1.0f));
		Vertices[2] = TextureVertex(XMFLOAT3(+objX, -objY, objZ), XMFLOAT2(0.0f, 1.0f));

		Vertices[3] = TextureVertex(XMFLOAT3(+objX, -objY, objZ), XMFLOAT2(0.0f, 1.0f));
		Vertices[4] = TextureVertex(XMFLOAT3(+objX, +objY, objZ), XMFLOAT2(0.0f, 0.0f));
		Vertices[5] = TextureVertex(XMFLOAT3(-objX, +objY, objZ), XMFLOAT2(1.0f, 0.0f));
	}
	break;

	case 2: // 왼쪽
	{
		Vertices[0] = TextureVertex(XMFLOAT3(objX, +objY, +objZ), XMFLOAT2(1.0f, 0.0f));
		Vertices[1] = TextureVertex(XMFLOAT3(objX, -objY, +objZ), XMFLOAT2(1.0f, 1.0f));
		Vertices[2] = TextureVertex(XMFLOAT3(objX, -objY, -objZ), XMFLOAT2(0.0f, 1.0f));

		Vertices[3] = TextureVertex(XMFLOAT3(objX, -objY, -objZ), XMFLOAT2(0.0f, 1.0f));
		Vertices[4] = TextureVertex(XMFLOAT3(objX, +objY, -objZ), XMFLOAT2(0.0f, 0.0f));
		Vertices[5] = TextureVertex(XMFLOAT3(objX, +objY, +objZ), XMFLOAT2(1.0f, 0.0f));
	}
	break;

	case 3: // 오른쪽
	{
		Vertices[0] = TextureVertex(XMFLOAT3(objX, +objY, -objZ), XMFLOAT2(1.0f, 0.0f));
		Vertices[1] = TextureVertex(XMFLOAT3(objX, -objY, -objZ), XMFLOAT2(1.0f, 1.0f));
		Vertices[2] = TextureVertex(XMFLOAT3(objX, -objY, +objZ), XMFLOAT2(0.0f, 1.0f));

		Vertices[3] = TextureVertex(XMFLOAT3(objX, -objY, +objZ), XMFLOAT2(0.0f, 1.0f));
		Vertices[4] = TextureVertex(XMFLOAT3(objX, +objY, +objZ), XMFLOAT2(0.0f, 0.0f));
		Vertices[5] = TextureVertex(XMFLOAT3(objX, +objY, -objZ), XMFLOAT2(1.0f, 0.0f));
	}
	break;

	case 4: // 위
	{
		Vertices[0] = TextureVertex(XMFLOAT3(+objX, objY, -objZ), XMFLOAT2(1.0f, 0.0f));
		Vertices[1] = TextureVertex(XMFLOAT3(+objX, objY, +objZ), XMFLOAT2(1.0f, 1.0f));
		Vertices[2] = TextureVertex(XMFLOAT3(-objX, objY, +objZ), XMFLOAT2(0.0f, 1.0f));

		Vertices[3] = TextureVertex(XMFLOAT3(-objX, objY, +objZ), XMFLOAT2(0.0f, 1.0f));
		Vertices[4] = TextureVertex(XMFLOAT3(-objX, objY, -objZ), XMFLOAT2(0.0f, 0.0f));
		Vertices[5] = TextureVertex(XMFLOAT3(+objX, objY, -objZ), XMFLOAT2(1.0f, 0.0f));
	}
	break;

	case 5: // 아래
	{
		Vertices[0] = TextureVertex(XMFLOAT3(+objX, objY, +objZ), XMFLOAT2(1.0f, 0.0f));
		Vertices[1] = TextureVertex(XMFLOAT3(+objX, objY, -objZ), XMFLOAT2(1.0f, 1.0f));
		Vertices[2] = TextureVertex(XMFLOAT3(-objX, objY, -objZ), XMFLOAT2(0.0f, 1.0f));

		Vertices[3] = TextureVertex(XMFLOAT3(-objX, objY, -objZ), XMFLOAT2(0.0f, 1.0f));
		Vertices[4] = TextureVertex(XMFLOAT3(-objX, objY, +objZ), XMFLOAT2(0.0f, 0.0f));
		Vertices[5] = TextureVertex(XMFLOAT3(+objX, objY, +objZ), XMFLOAT2(1.0f, 0.0f));
	}
	break;

	default:
		break;
	}

	m_VertexBuffer = CreateBufferResource(Device, CommandList, Vertices, m_nStride*m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_VertexUploadBuffer);
	
	m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = m_nStride;
	m_VertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}


// 3D 모델에 사용할 메쉬
StandardMesh::StandardMesh(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList) : Mesh(Device, CommandList)
{

}

StandardMesh::~StandardMesh()
{

}

void StandardMesh::OnPreRender(ID3D12GraphicsCommandList *CommandList, void *Context)
{
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView[5] = { m_PositionBufferView, m_TextureCoord0BufferView, m_NormalBufferView, m_TangentBufferView, m_BiTangentBufferView };
	CommandList->IASetVertexBuffers(m_nSlot, 5, VertexBufferView);
}

void StandardMesh::OnPreRender(ID3D12GraphicsCommandList *CommandList, void *Context, D3D12_VERTEX_BUFFER_VIEW InstanceBufferView)
{
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView[6] = { m_PositionBufferView, m_TextureCoord0BufferView, m_NormalBufferView, m_TangentBufferView, m_BiTangentBufferView, InstanceBufferView };
	CommandList->IASetVertexBuffers(m_nSlot, 6, VertexBufferView);
}

void StandardMesh::LoadMeshFromFile(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, FILE *InFile)
{
	char Token[64] = { '\0' };
	BYTE nStrLength = 0;

	int nPosition = 0, nColor = 0, nNormal = 0, nTangent = 0, nBiTangent = 0, nTextureCoord = 0, nIndice = 0, nSubMesh = 0, nSubIndice = 0;

	UINT nRead = (UINT)::fread(&m_nVertices, sizeof(int), 1, InFile);
	nRead = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, InFile);
	nRead = (UINT)::fread(&m_strMeshName, sizeof(char), nStrLength, InFile);
	m_strMeshName[nStrLength] = '\0';

	for (; ;) {
		nRead = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, InFile);
		nRead = (UINT)::fread(&Token, sizeof(char), nStrLength, InFile);
		Token[nStrLength] = '\0';

		if (!strcmp(Token, "<Bounds>:")) {
			nRead = (UINT)::fread(&m_AABBCenter, sizeof(XMFLOAT3), 1, InFile);
			nRead = (UINT)::fread(&m_AABBExtent, sizeof(XMFLOAT3), 1, InFile);
			m_xmBoundingBox.Center = m_AABBCenter;
			m_xmBoundingBox.Extents = m_AABBExtent;
		}
		else if (!strcmp(Token, "<Positions>:")) {
			nRead = (UINT)::fread(&nPosition, sizeof(int), 1, InFile);
			if (nPosition > 0) {
				m_nType |= VERTEXT_POSITION;
				m_Position = new XMFLOAT3[nPosition];
				nRead = (UINT)::fread(m_Position, sizeof(XMFLOAT3), nPosition, InFile);

				m_PositionBuffer = ::CreateBufferResource(Device, CommandList, m_Position, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_PositionUploadBuffer);

				m_PositionBufferView.BufferLocation = m_PositionBuffer->GetGPUVirtualAddress();
				m_PositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_PositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(Token, "<Colors>:")) {
			nRead = (UINT)::fread(&nColor, sizeof(int), 1, InFile);
			if (nColor > 0) {
				m_nType |= VERTEXT_COLOR;
				m_Color = new XMFLOAT4[nColor];
				nRead = (UINT)::fread(m_Color, sizeof(XMFLOAT4), nColor, InFile);
			}
		}
		else if (!strcmp(Token, "<TextureCoords0>:")) {
			nRead = (UINT)::fread(&nTextureCoord, sizeof(int), 1, InFile);
			if (nTextureCoord > 0) {
				m_nType |= VERTEXT_TEXTURE_COORD0;
				m_TextureCoord0 = new XMFLOAT2[nTextureCoord];
				nRead = (UINT)::fread(m_TextureCoord0, sizeof(XMFLOAT2), nTextureCoord, InFile);

				m_TextureCoord0Buffer = ::CreateBufferResource(Device, CommandList, m_TextureCoord0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_TextureCoord0UploadBuffer);

				m_TextureCoord0BufferView.BufferLocation = m_TextureCoord0Buffer->GetGPUVirtualAddress();
				m_TextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_TextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(Token, "<TextureCoords1>:")) {
			nRead = (UINT)::fread(&nTextureCoord, sizeof(int), 1, InFile);
			if (nTextureCoord > 0) {
				m_nType |= VERTEXT_TEXTURE_COORD1;
				m_TextureCoord1 = new XMFLOAT2[nTextureCoord];
				nRead = (UINT)::fread(m_TextureCoord1, sizeof(XMFLOAT2), nTextureCoord, InFile);

				m_TextureCoord1Buffer = ::CreateBufferResource(Device, CommandList, m_TextureCoord1, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_TextureCoord1UploadBuffer);

				m_TextureCoord1BufferView.BufferLocation = m_TextureCoord1Buffer->GetGPUVirtualAddress();
				m_TextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_TextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(Token, "<Normals>:")) {
			nRead = (UINT)::fread(&nNormal, sizeof(int), 1, InFile);
			if (nNormal > 0) {
				m_nType |= VERTEXT_NORMAL;
				m_Normal = new XMFLOAT3[nNormal];
				nRead = (UINT)::fread(m_Normal, sizeof(XMFLOAT3), nNormal, InFile);

				m_NormalBuffer = ::CreateBufferResource(Device, CommandList, m_Normal, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_NormalUploadBuffer);

				m_NormalBufferView.BufferLocation = m_NormalBuffer->GetGPUVirtualAddress();
				m_NormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_NormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(Token, "<Tangents>:")) {
			nRead = (UINT)::fread(&nTangent, sizeof(int), 1, InFile);
			if (nTangent > 0) {
				m_nType |= VERTEXT_TANGENT;
				m_Tangent = new XMFLOAT3[nTangent];
				nRead = (UINT)::fread(m_Tangent, sizeof(XMFLOAT3), nTangent, InFile);

				m_TangentBuffer = ::CreateBufferResource(Device, CommandList, m_Tangent, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_TangentUploadBuffer);

				m_TangentBufferView.BufferLocation = m_TangentBuffer->GetGPUVirtualAddress();
				m_TangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_TangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(Token, "<BiTangents>:")) {
			nRead = (UINT)::fread(&nBiTangent, sizeof(int), 1, InFile);
			if (nBiTangent > 0) {
				m_BiTangent = new XMFLOAT3[nBiTangent];
				nRead = (UINT)::fread(m_BiTangent, sizeof(XMFLOAT3), nBiTangent, InFile);

				m_BiTangentBuffer = ::CreateBufferResource(Device, CommandList, m_BiTangent, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_BiTangentUploadBuffer);

				m_BiTangentBufferView.BufferLocation = m_BiTangentBuffer->GetGPUVirtualAddress();
				m_BiTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_BiTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(Token, "<SubMeshes>:")) {
			nRead = (UINT)::fread(&(m_nSubMesh), sizeof(int), 1, InFile);
			if (m_nSubMesh > 0) {
				m_pnSubSetIndices = new int[m_nSubMesh];
				m_ppnSubSetIndices = new UINT*[m_nSubMesh];

				m_SubSetIndexBuffer = new ID3D12Resource*[m_nSubMesh];
				m_SubSetIndexUploadBuffer = new ID3D12Resource*[m_nSubMesh];
				m_SubSetIndexBufferView = new D3D12_INDEX_BUFFER_VIEW[m_nSubMesh];

				for (int i = 0; i < m_nSubMesh; ++i) {
					nRead = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, InFile);
					nRead = (UINT)::fread(Token, sizeof(char), nStrLength, InFile);
					Token[nStrLength] = '\0';
					if (!strcmp(Token, "<SubMesh>:")) {
						int nIndex = 0;
						nRead = (UINT)::fread(&nIndex, sizeof(int), 1, InFile);
						nRead = (UINT)::fread(&(m_pnSubSetIndices[i]), sizeof(int), 1, InFile);
						if (m_pnSubSetIndices[i] > 0) {
							m_ppnSubSetIndices[i] = new UINT[m_pnSubSetIndices[i]];
							nRead = (UINT)::fread(m_ppnSubSetIndices[i], sizeof(UINT), m_pnSubSetIndices[i], InFile);

							m_SubSetIndexBuffer[i] = ::CreateBufferResource(Device, CommandList, m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_SubSetIndexUploadBuffer[i]);

							m_SubSetIndexBufferView[i].BufferLocation = m_SubSetIndexBuffer[i]->GetGPUVirtualAddress();
							m_SubSetIndexBufferView[i].Format = DXGI_FORMAT_R32_UINT;
							m_SubSetIndexBufferView[i].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[i];
						}
					}
				}
			}
		}
		else if (!strcmp(Token, "</Mesh>")) {
			break;
		}
	}
}

// 애니메이션을 하는 3D 모델에 사용할 메쉬
SkinnedMesh::SkinnedMesh(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList) : StandardMesh(Device, CommandList)
{

}

SkinnedMesh::~SkinnedMesh()
{
	
}

void SkinnedMesh::CreateShaderVariable(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList)
{
	UINT nElementByte = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255);
	m_cbBoneOffset = ::CreateBufferResource(Device, CommandList, NULL, nElementByte, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_cbBoneOffset->Map(0, NULL, (void **)&m_BoneOffsetPos);

	m_cbBoneTransform = ::CreateBufferResource(Device, CommandList, NULL, nElementByte, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_cbBoneTransform->Map(0, NULL, (void **)&m_BoneTransformPos);
}

void SkinnedMesh::UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList)
{
	if (m_cbBoneOffset && m_cbBoneTransform) {
		D3D12_GPU_VIRTUAL_ADDRESS cbBoneOffsetGpuVirtualAddress = m_cbBoneOffset->GetGPUVirtualAddress();
		CommandList->SetGraphicsRootConstantBufferView(10, cbBoneOffsetGpuVirtualAddress);
		D3D12_GPU_VIRTUAL_ADDRESS cbBoneTransformGpuVirtualAddress = m_cbBoneTransform->GetGPUVirtualAddress();
		CommandList->SetGraphicsRootConstantBufferView(11, cbBoneTransformGpuVirtualAddress);
		//cout << "game start";
		for (int i = 0; i < m_nSkinningBone; ++i) {
			XMStoreFloat4x4(&m_BoneOffsetPos[i], XMMatrixTranspose(XMLoadFloat4x4(&m_BindPoseBoneOffset[i])));
			XMStoreFloat4x4(&m_BoneTransformPos[i], XMMatrixTranspose(XMLoadFloat4x4(&m_SkinningBoneFrameCache[i]->m_WorldPos)));
			//cout << i << " " << m_BoneOffsetPos[i]._11 << endl;
		}
	}
}

void SkinnedMesh::OnPreRender(ID3D12GraphicsCommandList *CommandList, void *Context)
{
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView[7] = { m_PositionBufferView, m_TextureCoord0BufferView, m_NormalBufferView, m_TangentBufferView, m_BiTangentBufferView,m_BoneIndexBufferView , m_BoneWeightBufferView };
	CommandList->IASetVertexBuffers(m_nSlot, 7, VertexBufferView);
}

void SkinnedMesh::OnPreRender(ID3D12GraphicsCommandList *CommandList, void *Context, D3D12_VERTEX_BUFFER_VIEW InstanceBufferView)
{
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView[8] = { m_PositionBufferView, m_TextureCoord0BufferView, m_NormalBufferView, m_TangentBufferView, m_BiTangentBufferView, m_BoneIndexBufferView, m_BoneWeightBufferView, InstanceBufferView };
	CommandList->IASetVertexBuffers(m_nSlot, 8, VertexBufferView);
}

void SkinnedMesh::LoadSkinInfoFromFile(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, FILE *InFile)
{
	char Token[64] = { '\0' };
	BYTE nStrLength = 0;

	UINT nRead = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, InFile);
	nRead = (UINT)::fread(m_SkinnedMeshName, sizeof(char), nStrLength, InFile);
	m_SkinnedMeshName[nStrLength] = '\0';

	for (; ;) {
		nRead = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, InFile);
		nRead = (UINT)::fread(Token, sizeof(char), nStrLength, InFile);
		Token[nStrLength] = '\0';

		if (!strcmp(Token, "<BonesPerVertex>:"))
			nRead = (UINT)::fread(&m_nBonePerVertex, sizeof(int), 1, InFile);
		else if (!strcmp(Token, "<Bounds>:")) {
			nRead = (UINT)::fread(&m_AABBCenter, sizeof(XMFLOAT3), 1, InFile);
			nRead = (UINT)::fread(&m_AABBExtent, sizeof(XMFLOAT3), 1, InFile);
		}
		else if (!strcmp(Token, "<BoneNames>:")) {
			nRead = (UINT)::fread(&m_nSkinningBone, sizeof(int), 1, InFile);
			if (m_nSkinningBone > 0) {
				m_SkinningBoneName = new char[m_nSkinningBone][64];
				m_SkinningBoneFrameCache = new GameObject*[m_nSkinningBone];

				for (int i = 0; i < m_nSkinningBone; ++i) {
					nRead = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, InFile);
					nRead = (UINT)::fread(m_SkinningBoneName[i], sizeof(char), nStrLength, InFile);
					m_SkinningBoneName[i][nStrLength] = '\0';

					m_SkinningBoneFrameCache[i] = NULL;
				}
			}
		}
		else if (!strcmp(Token, "<BoneOffsets>:")) {
			nRead = (UINT)::fread(&m_nSkinningBone, sizeof(int), 1, InFile);
			if (m_nSkinningBone > 0) {
				m_BindPoseBoneOffset = new XMFLOAT4X4[m_nSkinningBone];
				nRead = (UINT)::fread(m_BindPoseBoneOffset, sizeof(float), 16 * m_nSkinningBone, InFile);
			}
		}
		else if (!strcmp(Token, "<BoneWeights>:")) {
			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			nRead = (UINT)::fread(&m_nVertices, sizeof(int), 1, InFile);
			if (m_nVertices > 0) {
				m_BoneIndice = new XMUINT4[m_nVertices];
				m_BoneWeight = new XMFLOAT4[m_nVertices];

				nRead = (UINT)::fread(m_BoneIndice, sizeof(XMUINT4), m_nVertices, InFile);
				m_BoneIndexBuffer = ::CreateBufferResource(Device, CommandList, m_BoneIndice, sizeof(XMUINT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_BoneIndexUploadBuffer);

				m_BoneIndexBufferView.BufferLocation = m_BoneIndexBuffer->GetGPUVirtualAddress();
				m_BoneIndexBufferView.StrideInBytes = sizeof(XMUINT4);
				m_BoneIndexBufferView.SizeInBytes = sizeof(XMUINT4) * m_nVertices;

				nRead = (UINT)::fread(m_BoneWeight, sizeof(XMFLOAT4), m_nVertices, InFile);
				m_BoneWeightBuffer = ::CreateBufferResource(Device, CommandList, m_BoneWeight, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_BoneWeightUploadBuffer);

				m_BoneWeightBufferView.BufferLocation = m_BoneWeightBuffer->GetGPUVirtualAddress();
				m_BoneWeightBufferView.StrideInBytes = sizeof(XMFLOAT4);
				m_BoneWeightBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;
			}
		}
		else if (!strcmp(Token, "</SkinningInfo>")) {
			break;
		}
	}
}