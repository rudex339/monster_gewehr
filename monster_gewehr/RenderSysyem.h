#pragma once
#include <map>
#include "Object.h"

////////////////////////////////////////////////////////////////////////////////

struct ModelInfo {
	ModelInfo() {}
	std::string ModelTag;
};


////////////////////////////////////////////////////////////////////////////////

class Shader {
private:
public:
	void prepareShader() {};
};


Shader* ShaderBuild(std::string ShaderName);;

////////////////////////////////////////////////////////////////////////////////


class RenderSystem :public ECS::EntitySystem
{
public:
	RenderSystem(ID3D12GraphicsCommandList* commandList);
	void CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
    ~RenderSystem(){}
	virtual void configure(World* world);
	virtual void unconfigure(World* world);
	virtual void tick(World* world, float deltaTime);	

	void addObject(std::string model_name, char* model_root);
	void buildObject(ID3D12Device* pd3dDevice);
private:
	
	void render(Object* p_object);
	void CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int resources);
private:

	std::unordered_map<std::string, std::unique_ptr<Shader>> m_ShaderList;
	std::unordered_map<std::string, std::unique_ptr<Object>> m_ObjectList;

	std::string cur_Shader = "";

protected:
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pd3dCbvSrvDescriptorHeap;

	D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorNextHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorNextHandle;
};

