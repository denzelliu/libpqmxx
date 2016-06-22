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

TEST(params_sync, datatypes) {

  Connection cnx;
  cnx.connect("postgresql://postgres@localhost");

  EXPECT_EQ(32767, cnx.execute("SELECT $1::smallint", int16_t(32767)).result().get<int16_t>(0));
  EXPECT_EQ(2147483647, cnx.execute("SELECT $1", 2147483647).result().get<int32_t>(0));
  EXPECT_EQ(9223372036854775807, cnx.execute("SELECT $1", int64_t(9223372036854775807)).result().get<int64_t>(0));
  EXPECT_FLOAT_EQ(0.45567, cnx.execute("SELECT $1::real", float(0.45567)).result().get<float>(0));
  EXPECT_FLOAT_EQ(0.45567, cnx.execute("SELECT $1::double precision", double(0.45567)).result().get<double>(0));
  EXPECT_EQ(true, cnx.execute("SELECT $1", true).result().get<bool>(0));
  EXPECT_EQ(false, cnx.execute("SELECT $1", false).result().get<bool>(0));
  EXPECT_STREQ("hello", cnx.execute("SELECT $1", "hello").result().get<std::string>(0).c_str());
  EXPECT_STREQ("hello", cnx.execute("SELECT $1", std::string("hello")).result().get<std::string>(0).c_str());
  EXPECT_EQ('X', cnx.execute("SELECT $1", 'X').result().get<char>(0));

}

TEST(params_sync, utf8) {

  Connection cnx;
  cnx.connect("postgresql://postgres@localhost");

  EXPECT_STREQ(u8"Günter", cnx.execute("SELECT $1", u8"Günter").result().get<std::string>(0).c_str());
  EXPECT_STREQ(u8"メインページ", cnx.execute("SELECT $1", u8"メインページ").result().get<std::string>(0).c_str());

}

TEST(params_sync, date_time) {

  Connection cnx;
  cnx.connect("postgresql://postgres@localhost");

  EXPECT_STREQ("1970-01-01", cnx.execute("SELECT to_char($1, 'YYYY-MM-DD')", date_t {0}).result().get<std::string>(0).c_str());
  EXPECT_STREQ("2014-11-01 05:14:00", cnx.execute("SELECT to_char($1 at time zone 'America/New_York', 'YYYY-MM-DD HH24:MI:SS')", timestamptz_t {1414833240000000}).result().get<std::string>(0).c_str());

  auto timetz = cnx.execute("SELECT $1", timetz_t{860123, 7*3600}).result().get<timetz_t>(0);
  EXPECT_EQ(860123, timetz.time);
  EXPECT_EQ(25200, timetz.offset);

  EXPECT_EQ(39602000101, cnx.execute("SELECT $1", db::postgres::time_t{39602000101}).result().get<db::postgres::time_t>(0));

  auto interval = cnx.execute("SELECT $1", interval_t {7384000000, 7, 4}).result().get<interval_t>(0);

  EXPECT_EQ(7384000000, interval.time);
  EXPECT_EQ(7, interval.days);
  EXPECT_EQ(4, interval.months);

  cnx.execute("set timezone TO 'America/New_York'");
  EXPECT_STREQ("2014-11-01 05:14:00", cnx.execute("SELECT to_char($1, 'YYYY-MM-DD HH24:MI:SS')", timestamp_t {1414818840000000}).result().get<std::string>(0).c_str());

}


TEST(param_sync, bytea_type) {

  Connection cnx;
  cnx.connect("postgresql://postgres@localhost");

  std::vector<uint8_t> expected;
  expected.push_back(0xDE);
  expected.push_back(0xAD);
  expected.push_back(0xBE);
  expected.push_back(0xEF);

  std::vector<uint8_t> actual = cnx.execute("SELECT $1::bytea", expected).result().get<std::vector<uint8_t>>(0);
  EXPECT_TRUE(expected.size() == actual.size() && std::equal(actual.begin(), actual.end(), expected.begin()));

}

