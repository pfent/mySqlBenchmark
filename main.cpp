#include <chrono>
#include <iostream>
#include <mysqlx/xdevapi.h>

template<typename T>
auto bench(T &&fun) {
    const auto start = std::chrono::high_resolution_clock::now();

    fun();

    const auto end = std::chrono::high_resolution_clock::now();

    return std::chrono::duration<double>(end - start).count();
}

/**
 * See:
 * https://dev.mysql.com/doc/dev/connector-cpp/8.0/devapi_ref.html
 * https://dev.mysql.com/doc/dev/connector-cpp/8.0/group__devapi.html
 */
int main(int argc, const char *argv[]) {
    const auto url = [&] {
        if (argc < 2) {
            std::cout << "using default connection\n";
            return "mysqlx://root@127.0.0.1";
        }
        return argv[1];
    }();
    
    auto session = mysqlx::Session(url);
    /*
    auto schema = session.getDefaultSchema();
    auto tables = schema.getTableNames();

    for (const auto &table : tables) {
        std::cout << table << '\n';
    }
     */

    // TODO: query connection type:
    // select connection_type from performance_schema.threads where connection_type is not null and processlist_state is not null;

    auto statement = session.sql("SELECT 1;");
    auto iterations = size_t(1e6);
    auto timeTaken = bench([&] {
        for (size_t i = 0; i < iterations; ++i) {
            auto result = statement.execute();
            auto row = result.fetchOne();
            if (row[0].get<int>() != 1) {
                throw std::runtime_error{"unexpected return value"};
            }
        }
    });

    std::cout << iterations / timeTaken << "msg/s\n";

    return 0;
}