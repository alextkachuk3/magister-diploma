#include "Dx12ShaderBytecode.h"

Dx12ShaderBytecode::Dx12ShaderBytecode(const std::wstring& filePath)
{
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open shader file.");
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	data.resize(size);
	if (!file.read(data.data(), size))
	{
		throw std::runtime_error("Failed to read shader file.");
	}

	bytecode.pShaderBytecode = data.data();
	bytecode.BytecodeLength = data.size();
}

D3D12_SHADER_BYTECODE Dx12ShaderBytecode::GetBytecode() const
{
	return bytecode;
}
