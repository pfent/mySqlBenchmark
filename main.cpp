#include <iostream>
#include <mysqlx/xdevapi.h>

int main(int argc, const char *argv[]) {
    const auto url = [&] {
        if (argc < 2) {
            std::cout << "using default connection\n";
            return "mysqlx://root@127.0.0.1";
        }
        return argv[1];
    }();

    auto session = mysqlx::Session(url);
    auto schema = session.getDefaultSchema();
    auto tables = schema.getTableNames();

    for (const auto &table : tables) {
        std::cout << table << '\n';
    }

    auto statement = session.sql("SELECT 1;");
    auto result = statement.execute();
    auto row = result.fetchOne();
    std::cout << row[0] << '\n';
    assert(row[0].get<int>() == 1);

    return 0;
}