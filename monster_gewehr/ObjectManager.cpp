//-----------------------------------------------------------------------------
// File: ObjectManager.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "ObjectManager.h"

ID3D12DescriptorHeap *ObjectManager::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	ObjectManager::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	ObjectManager::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	ObjectManager::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	ObjectManager::m_d3dSrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	ObjectManager::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	ObjectManager::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	ObjectManager::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	ObjectManager::m_d3dSrvGPUDescriptorNextHandle;

ObjectManager::ObjectManager()
{
}

ObjectManager::~ObjectManager()
{
}

void ObjectManager::BuildDefaultLightsAndMaterials()
{
}

void ObjectManager::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	
	CreateCbvSrvDescriptorHeaps(pd3dDevice, 10, 55); //사용되는 텍스쳐 갯수 

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature); 

	BuildDefaultLightsAndMaterials();

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);//1

	m_pBox = new Box(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);//1

	XMFLOAT3 xmf3Scale(7.74f, 8.0f, 7.74f);
	XMFLOAT4 xmf4Color(0.0f, 0.5f, 0.5f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		_T("Terrain/terrain.raw"), 513,513, xmf3Scale, xmf4Color);//2
	//4104/330
	m_ModelList.insert({"Soldier",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Soldier.bin", NULL) });

	m_ModelList.insert({ "Armor",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/equip_heavy.bin", NULL) });


	m_ModelList.insert({ "benelliM4",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/benelliM4.bin", NULL) });
	m_ModelList.insert({ "M4A1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/m4a1.bin", NULL) });
	m_ModelList.insert({ "M110",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/M110.bin", NULL) });
	

	m_ModelList.insert({ "Souleater",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/souleater.bin", NULL) });
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//load scene object
	m_ModelList.insert({ "S_Old_Military_Trunk_uh2ucdkfa_lod3_Var1_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/S_Old_Military_Trunk_uh2ucdkfa_lod3_Var1_LOD1.bin", NULL) });
	m_ModelList.insert({ "S_Tarped_Crate_vh3lbfy_lod5_Var2_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/S_Tarped_Crate_vh3lbfy_lod5_Var2_LOD1.bin", NULL) });
	m_ModelList.insert({ "S_Wooden_Crate_wlkjbhn_lod5_Var1_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/S_Wooden_Crate_wlkjbhn_lod5_Var1_LOD1.bin", NULL) });
	m_ModelList.insert({ "S_Wooden_Military_Crate_ucooedgfa_lod5_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/S_Wooden_Military_Crate_ucooedgfa_lod5_LOD1.bin", NULL) });
	m_ModelList.insert({ "SM_ac_building_001_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SM_ac_building_001_LOD1.bin", NULL) });
	m_ModelList.insert({ "SM_ac_building_003_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SM_ac_building_003_LOD1.bin", NULL) });
	m_ModelList.insert({ "SM_ac_building_006_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SM_ac_building_006_LOD1.bin", NULL) });
	m_ModelList.insert({ "SM_ac_building_009_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SM_ac_building_009_LOD1.bin", NULL) });
	m_ModelList.insert({ "SM_ac_building_010_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SM_ac_building_010_LOD1.bin", NULL) });
	m_ModelList.insert({ "SM_ac_building_013_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SM_ac_building_013_LOD1.bin", NULL) });
	m_ModelList.insert({ "SM_ac_building_017_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SM_ac_building_017_LOD1.bin", NULL) });
	m_ModelList.insert({ "SM_ac_building_17_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SM_ac_building_17_LOD1.bin", NULL) });
	m_ModelList.insert({ "SM_ac_building_018_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SM_ac_building_018_LOD1.bin", NULL) });
	m_ModelList.insert({ "SM_ac_building_023_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SM_ac_building_023_LOD1.bin", NULL) });
	m_ModelList.insert({ "SM_ac_building_024_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SM_ac_building_024_LOD1.bin", NULL) });
	m_ModelList.insert({ "SM_ac_building_033_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SM_ac_building_033_LOD1.bin", NULL) });
	m_ModelList.insert({ "SM_ac_building_039_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SM_ac_building_039_LOD1.bin", NULL) });
	m_ModelList.insert({ "SM_ac_building_041_LOD1",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SM_ac_building_041_LOD1.bin", NULL) });

	m_ModelList.insert({ "Cube",
		 (std::shared_ptr<CLoadedModelInfo>)GameObjectModel::LoadGeometryAndAnimationFromFile
		 (pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Cube.bin", NULL,10,10) });

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	m_EmitterList.insert({ "explosion",
		 (std::shared_ptr<GameObjectModel>)
		(new MultiSpriteObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature))});

	m_EmitterList.insert({ "light",
		 (std::shared_ptr<GameObjectModel>)
		(new MultiSpriteObject2(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature)) });

	m_nHierarchicalGameObjects = 0;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void ObjectManager::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();


	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
			m_ppShaders[i]->Release();
		}
		delete[] m_ppShaders;
	}

	for (auto obj : m_ModelList) {
		obj.second->m_pModelRootObject->Release();
	}

	if (m_pTerrain) delete m_pTerrain;
	if (m_pSkyBox) delete m_pSkyBox;


	ReleaseShaderVariables();
}

ID3D12RootSignature *ObjectManager::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[10];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 6; //t6: gtxtAlbedoTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7; //t7: gtxtSpecularTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 8; //t8: gtxtNormalTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 9; //t9: gtxtMetallicTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 10; //t10: gtxtEmissionTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 11; //t11: gtxtEmissionTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 12; //t12: gtxtEmissionTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 1; //t1: gtxtTerrainBaseTexture
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[9].NumDescriptors = 1;
	pd3dDescriptorRanges[9].BaseShaderRegister = 2; //t2: gtxtTerrainDetailTexture
	pd3dDescriptorRanges[9].RegisterSpace = 0;
	pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[16];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 33;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[11].Descriptor.ShaderRegister = 7; //Skinned Bone Offsets
	pd3dRootParameters[11].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[12].Descriptor.ShaderRegister = 8; //Skinned Bone Transforms
	pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[14].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[14].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[15].Descriptor.ShaderRegister = 9; //Camera
	pd3dRootParameters[15].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void ObjectManager::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void ObjectManager::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void ObjectManager::ReleaseShaderVariables()
{
}

void ObjectManager::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

	for (int i = 0; i < m_nShaders; i++) m_ppShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nHierarchicalGameObjects; i++) m_ppHierarchicalGameObjects[i]->ReleaseUploadBuffers();
}

void ObjectManager::CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
}

D3D12_GPU_DESCRIPTOR_HANDLE ObjectManager::CreateConstantBufferViews(ID3D12Device *pd3dDevice, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}

void ObjectManager::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetResource(i);
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			pTexture->SetGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int j = 0; j < nRootParameters; j++) pTexture->SetRootParameterIndex(j, nRootParameterStartIndex + j);
}

bool ObjectManager::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool ObjectManager::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		break;
	default:
		break;
	}
	return(false);
}

bool ObjectManager::ProcessInput(UCHAR *pKeysBuffer)
{
	return(false);
}

void ObjectManager::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;

	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Animate(fTimeElapsed);
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->AnimateObjects(fTimeElapsed);
}

void ObjectManager::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{	
}

