#ifndef MYSQLBENCHMARK_MYSQLUTILS_H
#define MYSQLBENCHMARK_MYSQLUTILS_H

#include <chrono>
#include <exception>
#include <memory>
#include <optional>
#include <iostream>
#include <mysql.h>

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

void myClose(MYSQL*) {
   //mysql_close
}

auto
mySqlConnect(MYSQL &mysql, MySqlConnection connectionType, const char* host, const char* user, const char* password,
             const char* database) {
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
            throw;
      }
   }();
   std::cout << '\n';

   mysql_options(&mysql, MYSQL_OPT_PROTOCOL, &protocolType);

   if (!mysql_real_connect(&mysql, host, user, password, database, 0, nullptr, 0)) {
      throw std::runtime_error(std::string("Couldn't connect: ") + mysql_error(&mysql));
   }

   return std::unique_ptr<MYSQL, decltype(&myClose)>(&mysql, &myClose);
}

auto mySqlQuery(MYSQL &mysql, const std::string &query) {
   if (mysql_real_query(&mysql, query.data(), static_cast<unsigned long>(query.length()))) {
      throw std::runtime_error(std::string("Couldn't execute query: ") + query + "\n" + mysql_error(&mysql));
   }
}

auto mySqlUseResult(MYSQL &mysql) {
   auto result = mysql_use_result(&mysql);
   if (result == nullptr) {
      throw std::runtime_error(std::string("Couldn't fetch query ") + mysql_error(&mysql));
   }
   return std::unique_ptr<MYSQL_RES, decltype(&mysql_free_result)>(result, &mysql_free_result);
}

auto mySqlNumFields(MYSQL_RES* result) {
   return mysql_num_fields(result);
}

std::optional<MYSQL_ROW> mySqlFetchRow(MYSQL_RES* result) {
   auto row = mysql_fetch_row(result);
   if (row == nullptr) {
      return std::nullopt;
   }
   return row;
}

auto mySqlCreateStatement(MYSQL &mysql) {
   auto statement = mysql_stmt_init(&mysql);
   if (statement == nullptr) {
      throw std::runtime_error(std::string("Couldn't allocate statement ") + mysql_error(&mysql));
   }

   return std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(statement, &mysql_stmt_close);
}

/**
 * value shall not contain a semicolon ;
 */
auto mySqlPrepareStatement(MYSQL_STMT* statement, const std::string &value) {
   if (mysql_stmt_prepare(statement, value.c_str(), static_cast<unsigned long>(value.length())) != 0) {
      throw std::runtime_error(
            std::string("Couldn't prepare statement: ") + value + "\n" + mysql_stmt_error(statement));
   }
}

auto mySqlExectureStatement(MYSQL_STMT* statement) {
   if (mysql_stmt_execute(statement) != 0) {
      throw std::runtime_error(std::string("Couldn't execute statement ") + mysql_stmt_error(statement));
   }
}

auto mySqlRequestCursor(MYSQL_STMT* statement) {
   auto type = (unsigned long) CURSOR_TYPE_READ_ONLY;
   if (mysql_stmt_attr_set(statement, STMT_ATTR_CURSOR_TYPE, &type) != 0) {
      throw std::runtime_error(std::string("Couldn't request cursor ") + mysql_stmt_error(statement));
   }
}

auto mySqlBindResult(MYSQL_STMT* statement, MYSQL_BIND* bind) {
   if (mysql_stmt_bind_result(statement, bind) != 0) {
      throw std::runtime_error(std::string("Couldn't bind result ") + mysql_stmt_error(statement));
   }
}

auto mySqlStatementFetch(MYSQL_STMT* statement) {
   if (mysql_stmt_fetch(statement) != 0) {
      throw std::runtime_error(std::string("Couldn't fetch data ") + mysql_stmt_error(statement));
   }
}

auto mySqlStatementStore(MYSQL_STMT* statement) {
   if (mysql_stmt_store_result(statement) != 0) {
      throw std::runtime_error(std::string("Couldn't store result data ") + mysql_stmt_error(statement));
   }
}

auto mySqlStatementClose(MYSQL &mysql, MYSQL_STMT* statement) {
   if (mysql_stmt_close(statement) != 0) {
      throw std::runtime_error(std::string("Couldn't close statement ") + mysql_error(&mysql));
   }
}

auto mySqlBindParam(MYSQL_STMT* statement, MYSQL_BIND* bind) {
   if (mysql_stmt_bind_param(statement, bind) != 0) {
      throw std::runtime_error(std::string("Couldn't bind parameter ") + mysql_stmt_error(statement));
   }
}

#endif //MYSQLBENCHMARK_MYSQLUTILS_H
