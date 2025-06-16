#pragma once
#include "Core/Core.h"

struct ID3D12Device;

namespace Ideal
{
	class D3D12DeviceChild
	{
	public:
		D3D12DeviceChild(ComPtr<ID3D12Device> InDevice)
		{
			Device = InDevice;
		}

		inline ComPtr<ID3D12Device> GetDevice() const
		{
			Check(Device != nullptr);
			return Device;
		}

	protected:
		ComPtr<ID3D12Device> Device;
	};
}

