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

TEST(transaction, sync_commit) {

  Connection cnx;
  cnx.connect("postgresql://postgres@localhost");

  cnx.execute("DROP TABLE IF EXISTS tmpTrans");
  cnx.execute("CREATE TABLE tmpTrans(a VARCHAR(10))");
  cnx.begin();
  cnx.execute("INSERT INTO tmpTrans (a) VALUES ('hello')");
  cnx.commit();
  EXPECT_STREQ(cnx.execute("SELECT a FROM tmpTrans WHERE a='hello'").result().get<std::string>(0).c_str(), "hello");
  cnx.execute("DROP TABLE tmpTrans");

}

TEST(transaction, sync_rollback) {

  Connection cnx;
  cnx.connect("postgresql://postgres@localhost");

  cnx.execute("DROP TABLE IF EXISTS tmpTrans");
  cnx.execute("CREATE TABLE tmpTrans(a VARCHAR(10))");
  cnx.begin();
  cnx.execute("INSERT INTO tmpTrans (a) VALUES ('hello')");
  cnx.rollback();

  auto &result = cnx.execute("SELECT a FROM tmpTrans").result();
  EXPECT_FALSE(result.begin() != result.end());
  cnx.execute("DROP TABLE tmpTrans");

}
