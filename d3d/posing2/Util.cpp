#include "Util.h"

#include <windows.h>
#include <cassert>

size_t AlignmentSize(size_t size, size_t alignment)
{
	return size + alignment - (size % alignment);
}

std::string GetTexturePathFromModelAndTexPath(
    const std::string& modelPath,
    const std::string& texPath
)
{
    auto folderPath = modelPath.substr(0, modelPath.find_last_of('/'));
    return folderPath + "/" + texPath;
}

std::wstring GetWideString(const std::string& str)
{
    auto num1 = MultiByteToWideChar(
        CP_ACP,
        MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
        str.c_str(),
        -1,
        nullptr,
        0
    );

    std::wstring wstr(num1, 0);
    MultiByteToWideChar(
        CP_ACP,
        MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
        str.c_str(),
        -1,
        &wstr[0],
        num1
    );

    assert(wstr.size() == num1);

    return wstr;
}

std::string GetExtension(const std::string& path)
{
    auto pos = path.find_last_of('.');
    if (pos == std::string::npos)
    {
        return "";
    }

    return path.substr(pos + 1);
}

std::pair<std::string, std::string> SplitPath(const std::string& path, const char splitter)
{
    size_t i = path.find(splitter);
    return std::make_pair(path.substr(0, i), path.substr(i + 1));
}