/**
 * Copyright (c) 2016 Philippe FERDINAND
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/
#pragma once

#include "libpq-fe.h"
#include "postgres-params.h"
#include "postgres-result.h"

#include <functional>
#include <memory>

namespace db {
  namespace postgres {

    const int32_t DAYS_UNIX_TO_J2000_EPOCH = int32_t(10957);
    const int64_t MICROSEC_UNIX_TO_J2000_EPOCH = int64_t(946684800) * 1000000;

    typedef struct {
      int32_t epoch_date;
      operator int32_t() const { return epoch_date; }
    } date_t;

    typedef struct {
      int64_t epoch_time;
      operator int64_t() const { return epoch_time; }
    } timestamptz_t;

    typedef struct {
      int64_t epoch_time;
      operator int64_t() const { return epoch_time; }
    } timestamp_t;

    typedef struct {
      int64_t time;
      int32_t offset;
    } timetz_t;

    typedef struct {
      int64_t time;
      operator int64_t() const { return time; }
    } time_t;

    typedef struct {
      int64_t time;
      int32_t days;
      int32_t months;
    } interval_t;

    class Connection : public std::enable_shared_from_this<Connection> {

      friend class Result;

      public:
      
        Connection();

        /**
         * Destructor.
         **/
        ~Connection();
      
        /**
         * Open a connection to the database.
         * 
         * @param connInfo A postgresql connection string.
         * @param async    Enabling of the asynchonous processing.
         * @return the connection itself.
         * @see https://www.postgresql.org/docs/9.5/static/libpq-connect.html#LIBPQ-CONNSTRING
         * @example postgresql://[user[:password]@][netloc][:port][/dbname][?param1=value1&...]
         **/
        Connection &connect(const char *connInfo, bool async = false);
      
        /**
         * Close the database connection.
         **/
        void close() noexcept;
      
        /**
         * Transactions
         *
         * Helper methods to deal with transactions. The main benefit of using
         * those methods rather than executing the SQL commands is on nested
         * transactions. If a code start a transaction and call another code
         * also starting a transaction, thoses methods will create only one
         * transaction started at the first all to `begin()` and commited at the
         * last call to `commit()`.
         **/

        /**
         * Start a transaction.
         **/
        Connection &begin();

        /**
         * Commit a transaction.
         **/
        Connection &commit();

        /**
         * Rollback a transaction.
         **/
        Connection &rollback();
      
        /**
         * Cancel queries in progress.
         **/
        Connection &cancel();

        template<typename... Args>
        Connection &execute(const char *sql, Args... args) {
          Params params(sizeof...(args));
          params.bind(args...);
          execute(sql, params);
          return *this;
        }

        /**
         * Access to the result of the last execution.
         **/
        Result &result() {
          return result_;
        }
      
        Connection &once(std::function<bool (const Result &result)> callback);
        Connection &each(std::function<bool (const Result &result)> callback);
        Connection &done(std::function<void (int count)> callback);
        Connection &always(std::function<void ()> callback);
        Connection &error(std::function<void (std::exception_ptr reason)> callback);
      
    protected:

        /**
         * Process available data retreived from the server.
         * 
         * This method must be called by the owner of the event loop when 
         * some data are available in the connection.
         * 
         * @return true if more data are expected. In this case the event loop 
         *         owner must call again `consumeInput()` when more data will  
         *         become available.
         **/
        bool consumeInput();
      
        /**
         * Flush the data pending to be send throught the connection.
         * 
         * This method must be called by the event loop owner when the 
         * connection is ready to accept to write data to the server.
         * 
         * @return true if not all data have been flushed. In this case the 
         *         event loop owner must call again `flush()` when the server
         *         is ready to access more data.
         **/
        bool flush();
      
        /**
         * Get the native socket identifier.
         * 
         * See [PQsocket](https://www.postgresql.org/docs/current/static/libpq-status.html).
         * @return The file descriptor number of the connection socket to the server.
         **/
        int socket() const noexcept;
      
        std::string lastError() const;

      private:
        PGconn *pgconn_;
        Result  result_;
        bool async_;

        /**
         * Current transaction level.
         *
         * 0 → no transaction in progress.
         * 1 → one transaction in progress.
         * 2 → one transaction in progress + 1 nested transaction.
         * n → one transaction in progress + (n-1) nested transactions.
         **/
        int transaction_;

        void execute(const char *sql, const Params &params);
      
        /**
         * Callbacks for asynchonous operations in non bloking mode.
         **/
        std::function<bool (const Result &)>     iterator_; // once and each
        std::function<void (int)>                done_;
        std::function<void (std::exception_ptr)> error_;
        std::function<void ()>                   always_;
      
        operator PGconn *() {
          return pgconn_;
        }

        Connection(const Connection&) = delete;
        Connection(const Connection&&) = delete;
        Connection& operator = (const Connection&) = delete;
        Connection& operator = (const Connection&&) = delete;
    };
    
  } // namespace postgres  
}   // namespace db
