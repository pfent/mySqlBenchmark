#include <chrono>
#include <iostream>
#include <mysql/mysql.h>
#include "mySqlUtils.h"

void doSmallTx(MYSQL &mysql) {
    mySqlQuery(mysql, "SELECT 1;");

    auto result = mySqlUseReult(mysql);
    auto numResults = mySqlNumFields(result.get());
    if (numResults != 1) {
        throw std::runtime_error("Unexpected number of fields");
    }

    auto row = *mySqlFetchRow(result.get());

    if (row[0][0] != '1') {
        throw std::runtime_error("Unexpected data returned");
    }

/* TODO
 * auto iterations = size_t(1e6);
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
 */
}

/**
 * MySql XAPI does not support shared memory access
 * https://dev.mysql.com/doc/dev/connector-cpp/8.0/devapi_ref.html
 * https://dev.mysql.com/doc/dev/connector-cpp/8.0/group__devapi.html
 *
 * The C++ API also does not support shared memory.
 *
 * So the C API it is.
 *
 * Connection type can be queried by:
 * SELECT connection_type FROM performance_schema.threads WHERE connection_type IS NOT NULL AND processlist_state IS NOT NULL;
 */
int main(int argc, const char *argv[]) {
    if (argc < 4) {
        std::cout << "Usage: mySqlBenchmark <user> <password> <database>\n";
        return 1;
    }
    auto user = argv[1];
    auto password = argv[2];
    auto database = argv[3];

    auto connections = {MySqlConnection::TCP, MySqlConnection::SharedMemory, MySqlConnection::NamedPipe,
                        MySqlConnection::Socket};

    auto mysql = MYSQL();
    mysql_init(&mysql);

    for (auto connection : connections) {
        try {
            const auto c = mySqlConnect(mysql, connection, user, password, database);

            doSmallTx(mysql);
        } catch (const std::runtime_error &e) {
            std::cout << e.what() << '\n';
        }
    }

    // TODO: query connection type:
    // select connection_type from performance_schema.threads where connection_type is not null and processlist_state is not null;

    return 0;
}