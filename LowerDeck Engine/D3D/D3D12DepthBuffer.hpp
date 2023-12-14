#pragma once
#include <d3d12.h>
#include <wrl/client.h>

namespace D3D
{
	class D3D12Viewport;
	class D3D12Descriptor;
	class D3D12DescriptorHeap;

	/// <summary>
	/// Holds <c>ID3D12Resource ComPtr</c> for DepthStencil usage.
	/// </summary>
	class D3D12DepthBuffer
	{
	public:
		D3D12DepthBuffer() = default;
		/// <summary> Initializes underlaying resource. </summary>
		/// <param name="Viewport"> Needed to determine DepthStencil dimensions. </param>
		/// <param name="Format"> Default. </param>
		D3D12DepthBuffer(D3D12DescriptorHeap* pDepthHeap, D3D12Viewport* pViewport, DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT, bool bSRV = false);
		/// <summary>
		/// Calls <c>Release()</c> if haven't been called manually. 
		/// </summary>
		~D3D12DepthBuffer();

		/// <summary>
		/// Initialize.
		/// </summary>
		/// <param name="Viewport"> Needed to determine DepthStencil dimensions. </param>
		void Create(D3D12DescriptorHeap* pDepthHeap, D3D12Viewport* pViewport, bool bSRV = false);

		/// <summary>
		/// Calls ClearDepthStencilView for underlaying resource.
		/// </summary>
		void Clear();

		/// <summary>
		/// As it says.
		/// </summary>
		void OnResize(D3D12DescriptorHeap* pDepthHeap, D3D12Viewport* pViewport);

		ID3D12Resource* Get() { return m_Resource.Get(); }

		/// <summary> Getter for DSV Descriptor. </summary>
		/// <returns></returns>
		inline D3D12Descriptor DSV() { return m_DSV; }
		/// <summary>Getter for SRV Descriptor. </summary>
		/// <returns></returns>
		inline D3D12Descriptor SRV() { return m_SRV; }

		/// <summary>
		/// Safely releases underlaying resource.
		/// </summary>
		void Release();

	private:

		/// <summary> Holds actual DepthStencil resource. </summary>
		Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;
		/// <summary> Defaults to D32_FLOAT. </summary>
		DXGI_FORMAT m_Format{ DXGI_FORMAT_D32_FLOAT};

		/// <summary> Descriptor for allocating DepthStencilView usage. </summary>
		D3D12Descriptor m_DSV;
		/// <summary> 
		/// Descriptor for allocating SharedResourceView usage.<br/>
		/// Note: Use only when <c>D3D12DepthBuffer</c> is meant to be used in Shaders.
		/// </summary>
		D3D12Descriptor m_SRV;

	};
}
