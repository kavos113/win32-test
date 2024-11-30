#pragma once
#include <string>

class Model
{
public:
    Model(const std::string& path)
        : path_(path)
    {

    }

    virtual void Render() = 0;
    virtual void Read() = 0;

protected:
    std::string path_;
};

