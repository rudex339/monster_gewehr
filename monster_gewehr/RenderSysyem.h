#pragma once
#include <map>

ECS_TYPE_IMPLEMENTATION;

using namespace ECS;

////////////////////////////////////////////////////////////////////////////////

struct ModelInfo {
	ModelInfo() {}
	std::string ModelTag;
};


class Object {//다음 목표
private:
	Object* m_pParent = NULL;
	Object* m_pChild = NULL;
	Object* m_pSibling = NULL;
public:
	std::string ShaderTag;
	
	int count_resource{ 0 };
	void setResource() {};
	void render() {};
};

////////////////////////////////////////////////////////////////////////////////

class Shader {
private:
public:
	void prepareShader() {};
};


Shader* ShaderBuild(std::string ShaderName) {
	//std::unique_ptr<shader> shader_ptr
	//return std::move(shader_ptr)
	return NULL;
};

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

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorNextHandle;
};

