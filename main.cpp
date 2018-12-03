#include <chrono>
#include <iostream>
#include <mysql.h>
#include <vector>
#include <random>
#include <algorithm>
#include "mySqlUtils.h"
#include "ycsb.h"

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
         result = 0;
      }
   });

   std::cout << iterations / timeTaken << " msg/s\n";
}

static auto db = YcsbDatabase();

void prepareYcsb(MYSQL &mysql) {
   auto create = std::string("CREATE TEMPORARY TABLE Ycsb ( ycsb_key INTEGER PRIMARY KEY NOT NULL, ");
   for (size_t i = 1; i < ycsb_field_count; ++i) {
      create += "v" + std::to_string(i) + " CHAR(" + std::to_string(ycsb_field_length) + ") NOT NULL, ";
   }
   create += "v" + std::to_string(ycsb_field_count) + " CHAR(" + std::to_string(ycsb_field_length) + ") NOT NULL";
   create += ");";
   mySqlQuery(mysql, create);

   int i = 0;
   for (auto&[key, value] : db.database) {
      if (i % 1000 == 0) {
         std::cout << static_cast<double>(i) * 100 / ycsb_tuple_count << "%\n";
      }
      ++i;
      auto statement = std::string("INSERT INTO Ycsb VALUES ");
      statement += "(" + std::to_string(key) + ", ";
      for (auto &v : value.rows) {
         statement += "'";
         statement += v.data();
         statement += "', ";
      }
      statement.resize(statement.length() - 2); // remove last comma
      statement += ")";
      statement += ";";
      mySqlQuery(mysql, statement);
   }
}

void doLargeResultSet(MYSQL &mysql) {
   prepareYcsb(mysql);

   const auto resultSizeMB = static_cast<double>(ycsb_tuple_count) * ycsb_field_count * ycsb_field_length / 1024 / 1024;
   std::cout << "benchmarking " << resultSizeMB << "MB data transfer" << '\n';

   auto statement = mySqlCreateStatement(mysql);
   mySqlPrepareStatement(statement.get(), "SELECT value FROM Ycsb");

   auto resultKey = int();
   auto result = std::array<std::array<char, ycsb_field_length>, ycsb_field_count>();
   MYSQL_BIND resultBind[11];
   resultBind[0] = MYSQL_BIND();
   resultBind[0].buffer_type = MYSQL_TYPE_LONG;
   resultBind[0].buffer = &resultKey;
   resultBind[0].buffer_length = sizeof(resultKey);
   for (size_t i = 1; i < 10; ++i) {
      resultBind[i] = MYSQL_BIND();
      resultBind[i].buffer_type = MYSQL_TYPE_VARCHAR;
      resultBind[i].buffer = &result[i];
      resultBind[i].buffer_length = sizeof(result[i]);
   }

   mySqlBindResult(statement.get(), resultBind);

   auto timeTaken = bench([&] {
      mySqlExectureStatement(statement.get());

      for (size_t i = 0; i < ycsb_tuple_count; ++i) {
         mySqlStatementFetch(statement.get());
      }
   });

   std::cout << " " << resultSizeMB / timeTaken << " MB/s\n";
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
int main(int argc, const char* argv[]) {
   if (argc < 3) {
      std::cout << "Usage: mySqlBenchmark <user> <password> <database>\n";
      return 1;
   }
   auto user = argv[1];
   auto password = argv[2];
   auto database = argc > 3 ? argv[3] : "mysql";

   auto connections = {
         MySqlConnection::TCP,
         MySqlConnection::SharedMemory,
         MySqlConnection::NamedPipe,
         MySqlConnection::Socket
   };

   auto mysql = MYSQL();
   mysql_init(&mysql);

   for (auto connection : connections) {
      try {
         mysql_init(&mysql);
         const auto c = mySqlConnect(mysql, connection, user, password, database);

         std::cout << "Preparing YCSB temporary table\n";
         prepareYcsb(mysql);

         //doSmallTx2(mysql);
         doLargeResultSet(mysql);
      } catch (const std::runtime_error &e) {
         std::cout << e.what() << '\n';
      }
      std::cout << '\n';
   }

   // TODO: query connection type:
   // select connection_type from performance_schema.threads where connection_type is not null and processlist_state is not null;

   return 0;
}