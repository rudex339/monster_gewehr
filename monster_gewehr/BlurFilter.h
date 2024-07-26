#pragma once
class BlurFilter
{
private:
	ComPtr<ID3D12RootSignature> rootSig;

	const int MaxBlurRadius = 5;

	ID3D12Device* md3dDevice = nullptr;

	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mBlur0CpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mBlur0CpuUav;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mBlur1CpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mBlur1CpuUav;

	CD3DX12_GPU_DESCRIPTOR_HANDLE mBlur0GpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mBlur0GpuUav;

	CD3DX12_GPU_DESCRIPTOR_HANDLE mBlur1GpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mBlur1GpuUav;

	// Two for ping-ponging the textures.
	Microsoft::WRL::ComPtr<ID3D12Resource> mBlurMap0 = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mBlurMap1 = nullptr;
	
	ComPtr<ID3D12DescriptorHeap> mCbvSrvUavDescriptorHeap = nullptr;
private:
	ComPtr<ID3D12PipelineState> horzBlurPipeline;
	ComPtr<ID3D12PipelineState> vertBlurPipeline;

public:
	float m_Strength = 0.f;

public:
	BlurFilter() = default;
	BlurFilter(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void Execute(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* input);

	void BuildResource();

	void BuildPso();

	void BuildRootParameter();

	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
		UINT descriptorSize);
	void BuildDescriptors();

	std::vector<float> CalcGaussWeights(float sigma);

};

