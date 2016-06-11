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
#include "gtest/gtest.h"
#include "postgres-connection.h"
#include "postgres-exceptions.h"

using namespace db::postgres;

TEST(Synchronous, connect) {
  
  Connection cnx;
  EXPECT_NO_THROW(cnx.connect("postgresql://postgres@localhost").close());
  EXPECT_THROW(cnx.connect("postgresql://invalid_user@localhost"), ConnectionException);
  
  // cnx.execute("SELECT $1", 23);
}

TEST(Synchronous, execute_select_no_row) {

  Connection cnx;
  cnx.connect("postgresql://postgres@localhost");

  int32_t actual = 0;

  auto &result = cnx.execute("SELECT 1 WHERE 1=2").result();
  for (auto &row: result) {
    actual += row.get<int32_t>(0);
  }

  EXPECT_EQ(actual, 0);

}

TEST(Synchronous, execute_select_multiple_rows) {

  Connection cnx;
  cnx.connect("postgresql://postgres@localhost");

  int32_t actual = 0;

  auto &result = cnx.execute("SELECT generate_series(1, 3)").result();
  for (auto &row: result) {
    actual += row.get<int32_t>(0);
  }

  EXPECT_EQ(actual, 6);

}

TEST(Synchronous, execute_result_datatypes) {

  Connection cnx;
  cnx.connect("postgresql://postgres@localhost");

  EXPECT_EQ(cnx.execute("SELECT 1000").result().get<int32_t>(0), 1000);
  EXPECT_EQ(cnx.execute("SELECT true").result().get<bool>(0), true);
  EXPECT_EQ(cnx.execute("SELECT false").result().get<bool>(0), false);
  return;

  // cnx.execute("SELECT $1", 23);
}