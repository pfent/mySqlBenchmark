#ifndef MYSQLBENCHMARK_MYSQLUTILS_H
#define MYSQLBENCHMARK_MYSQLUTILS_H

#include <chrono>
#include <exception>
#include <memory>
#include <optional>
#include <iostream>
#include <mysql/mysql.h>

template<typename T>
auto bench(T &&fun) {
    const auto start = std::chrono::high_resolution_clock::now();

    fun();

    const auto end = std::chrono::high_resolution_clock::now();

    return std::chrono::duration<double>(end - start).count();
}

enum class MySqlConnection {
    TCP,
    Socket,
    NamedPipe,
    SharedMemory
};

auto mySqlConnect(MYSQL &mysql, MySqlConnection connectionType, const char* user, const char* password, const char* database) {
    std::cout << "Connecting via ";
    auto protocolType = [&] {
        switch (connectionType) {
            case MySqlConnection::TCP:
                std::cout << "TCP";
                return mysql_protocol_type::MYSQL_PROTOCOL_TCP;
            case MySqlConnection::Socket:
                std::cout << "Socket";
                return mysql_protocol_type::MYSQL_PROTOCOL_SOCKET;
            case MySqlConnection::NamedPipe:
                std::cout << "Named Pipe";
                return mysql_protocol_type::MYSQL_PROTOCOL_PIPE;
            case MySqlConnection::SharedMemory:
                std::cout << "Shared Memory";
                return mysql_protocol_type::MYSQL_PROTOCOL_MEMORY;
            default:
                __builtin_unreachable();
        }
    }();
    std::cout << '\n';

    mysql_options(&mysql, MYSQL_OPT_PROTOCOL, &protocolType);

    if (not mysql_real_connect(&mysql, nullptr, user, password, database, 0, nullptr, 0)) {
        throw std::runtime_error(std::string("Couldn't connect: ") + mysql_error(&mysql));
    }

    return std::unique_ptr<MYSQL, decltype(&mysql_close)>(&mysql, &mysql_close);
}

auto mySqlQuery(MYSQL &mysql, const std::string &query) {
    if (mysql_real_query(&mysql, query.data(), query.length())) {
        throw std::runtime_error(std::string("Couldn't execute query ") + mysql_error(&mysql));
    }
}

auto mySqlUseReult(MYSQL &mysql) {
    auto result = mysql_use_result(&mysql);
    if (result == nullptr) {
        throw std::runtime_error(std::string("Couldn't fetch query ") + mysql_error(&mysql));
    }
    return std::unique_ptr<MYSQL_RES, decltype(&mysql_free_result)>(result, &mysql_free_result);
}

auto mySqlNumFields(MYSQL_RES *result) {
    return mysql_num_fields(result);
}

std::optional<MYSQL_ROW> mySqlFetchRow(MYSQL_RES *result) {
    auto row = mysql_fetch_row(result);
    if (row == nullptr) {
        return std::nullopt;
    }
    return row;
}

#endif //MYSQLBENCHMARK_MYSQLUTILS_H
