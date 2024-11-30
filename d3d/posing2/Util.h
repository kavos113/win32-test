#pragma once
#include <string>

size_t AlignmentSize(size_t size, size_t alignment);
std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const std::string& texPath);
std::wstring GetWideString(const std::string& str);
std::string GetExtension(const std::string& path);
std::pair<std::string, std::string> SplitPath(const std::string& path, const char splitter = '*');