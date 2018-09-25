#include <chrono>
#include <iostream>
#include <mysql.h>
#include "mySqlUtils.h"

void doSmallTx(MYSQL &mysql) {
    const auto iterations = size_t(1e6);
    std::cout << iterations << " very small tx\n";

    const auto timeTaken = bench([&] {
        for (size_t i = 0; i < iterations; ++i) {
            mySqlQuery(mysql, "SELECT 1;");

            auto result = mySqlUseResult(mysql);
            const auto numResults = mySqlNumFields(result.get());
            if (numResults != 1) {
                throw std::runtime_error("Unexpected number of fields");
            }

            const auto row = *mySqlFetchRow(result.get());

            if (row[0][0] != '1') {
                throw std::runtime_error("Unexpected data returned");
            }
        }
    });

    std::cout << iterations / timeTaken << " msg/s\n";
}

void doSmallTx2(MYSQL &mysql) {
    const auto iterations = size_t(1e6);
    std::cout << iterations << " very small prepared statements\n";

    auto statement = mySqlCreateStatement(mysql);
    mySqlPrepareStatement(statement.get(), "SELECT 1");
    auto result = long();
    auto resultBind = MYSQL_BIND();
    resultBind.buffer_type = MYSQL_TYPE_LONG;
    resultBind.buffer = &result;
    resultBind.buffer_length = sizeof(result);

    mySqlBindResult(statement.get(), &resultBind);

    const auto timeTaken = bench([&] {
        for (size_t i = 0; i < iterations; ++i) {
            mySqlExectureStatement(statement.get());
            mySqlStatementFetch(statement.get());

            if (result != 1) {
                throw std::runtime_error("Unexpected data returned");
            }
        }
    });

    std::cout << iterations / timeTaken << " msg/s\n";
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
    if (argc < 3) {
        std::cout << "Usage: mySqlBenchmark <user> <password> <database>\n";
        return 1;
    }
    auto user = argv[1];
    auto password = argv[2];
    auto database = argc > 3 ? argv[3] : "";

    auto connections = {MySqlConnection::TCP, MySqlConnection::SharedMemory, MySqlConnection::NamedPipe,
                        MySqlConnection::Socket};

    auto mysql = MYSQL();
    mysql_init(&mysql);

    for (auto connection : connections) {
        try {
			mysql_init(&mysql);
            const auto c = mySqlConnect(mysql, connection, user, password, database);

            doSmallTx(mysql);
            doSmallTx2(mysql);
        } catch (const std::runtime_error &e) {
            std::cout << e.what() << '\n';
        }
        std::cout << '\n';
    }

    // TODO: query connection type:
    // select connection_type from performance_schema.threads where connection_type is not null and processlist_state is not null;

    return 0;
}