#pragma once
#include <cstdio>
#include <memory>
#include <string> // для path, mode

namespace lab4::resource
{

class FileHandle
{
  public:
    FileHandle(const std::string& path, const std::string& mode);

    bool is_open() const noexcept
    {
        return file_ != nullptr;
    }

    void close();

  private:
    struct FileCloser
    {
        void operator()(std::FILE* f) const
        {
            if (f)
                std::fclose(f);
        }
    };
    std::unique_ptr<std::FILE, FileCloser> file_;
};

} // namespace lab4::resource