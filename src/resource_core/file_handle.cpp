#include "file_handle.hpp"
#include "resource_error.hpp"

namespace lab4::resource
{

FileHandle::FileHandle(const std::string& path, const std::string& mode) : file_(std::fopen(path.c_str(), mode.c_str()))
{
    if (!file_)
    {
        throw ResourceError("Failed to open file: " + path);
    }
}

void FileHandle::close()
{
    file_.reset();
}

} // namespace lab4::resource