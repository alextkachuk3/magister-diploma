#include <d3d12.h>
#include <fstream>
#include <vector>
#include <stdexcept>

class Dx12ShaderBytecode
{
public:
	Dx12ShaderBytecode(const std::wstring& filePath);
	D3D12_SHADER_BYTECODE GetBytecode() const;

private:
	std::vector<char> data;
	D3D12_SHADER_BYTECODE bytecode;
};
