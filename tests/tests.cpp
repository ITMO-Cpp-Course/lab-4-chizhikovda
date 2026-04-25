#include <catch2/catch_all.hpp>
#include <cstdio>
#include <fstream>
#include <resource_core/file_handle.hpp>
#include <resource_core/resource_error.hpp>
#include <resource_core/resource_manager.hpp>

using namespace lab4::resource;

static std::string createTempFile(const std::string& content)
{
    std::string name = "temp_test_file.txt";
    std::ofstream out(name); // открываем для записи (перезаписываем)
    out << content;          // пишем содержимое
    out.close();
    return name;
}

TEST_CASE("RAII: FileHandle opens and closes file automatically")
{
    std::string fname = createTempFile("hello");
    {
        FileHandle fh(fname, "r"); // открываем файл
        REQUIRE(fh.is_open() == true);
    } // здесь fh уничтожается и файл закрывается

    // Проверяем, что файл освобождён: можем открыть его снова
    std::FILE* f = std::fopen(fname.c_str(), "r");
    REQUIRE(f != nullptr);
    std::fclose(f);
    std::remove(fname.c_str()); // удаляем временный файл
}

TEST_CASE("FileHandle move semantics")
{
    std::string fname = createTempFile("test");
    FileHandle fh1(fname, "r"); // открываем файл
    std::FILE* raw = fh1.get(); // запоминаем указатель

    FileHandle fh2 = std::move(fh1); // перемещаем владение
    REQUIRE(fh1.is_open() == false);
    REQUIRE(fh2.is_open() == true);
    REQUIRE(fh2.get() == raw); // ресурс тот же

    std::remove(fname.c_str());
}

TEST_CASE("ResourceManager caches and shares FileHandle")
{
    std::string fname = createTempFile("data");
    ResourceManager mgr;

    auto f1 = mgr.getFile(fname, "r"); // первый запрос – создаётся новый
    auto f2 = mgr.getFile(fname, "r"); // второй запрос – должен вернуть тот же
    REQUIRE(f1.get() == f2.get());     // указатели совпадают
    REQUIRE(mgr.cacheSize() == 1);     // в кеше одна запись

    f2.reset();                        // освобождаем одну сильную ссылку
    REQUIRE(mgr.cacheSize() == 1);     // кеш всё ещё хранит слабую ссылку
    auto f3 = mgr.getFile(fname, "r"); // снова запрашиваем
    REQUIRE(f3.get() == f1.get());     // должен вернуть тот же объект

    f1.reset(); // освобождаем все сильные ссылки
    f3.reset();
    mgr.cleanExpired();            // чистим кеш от просроченных
    REQUIRE(mgr.cacheSize() == 0); // кеш пуст

    std::remove(fname.c_str());
}

TEST_CASE("ResourceError is thrown on open failure")
{

    REQUIRE_THROWS_AS(FileHandle fh("nonexistent_file_12345.txt"),
                      ResourceError); // Попытка открыть несуществующий файл должна выбросить ResourceError
}