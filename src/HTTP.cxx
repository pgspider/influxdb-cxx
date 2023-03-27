// MIT License
//
// Copyright (c) 2022 TOSHIBA CORPORATION
// Copyright (c) 2020-2022 offa
// Copyright (c) 2019 Adam Wegrzynek
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

///
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#include "HTTP.h"
#include "InfluxDBException.h"
#include "Query.h"


namespace influxdb::transports
{
    namespace
    {
        size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
        {
            static_cast<std::string*>(userp)->append(static_cast<char*>(contents), size * nmemb);
            return size * nmemb;
        }

        void setConnectionOptions(CURL* handle)
        {
            curl_easy_setopt(handle, CURLOPT_TCP_KEEPIDLE, 120L);
            curl_easy_setopt(handle, CURLOPT_TCP_KEEPINTVL, 60L);
            curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);
        }

        CURL* createReadHandle()
        {
            if (CURL* readHandle = curl_easy_init(); readHandle != nullptr)
            {
                setConnectionOptions(readHandle);
                return readHandle;
            }

            throw InfluxDBException{__func__, "Failed to initialize read handle"};
        }

        CURL* createWriteHandle(const std::string& url)
        {
            if (CURL* writeHandle = curl_easy_init(); writeHandle != nullptr)
            {
                setConnectionOptions(writeHandle);
                curl_easy_setopt(writeHandle, CURLOPT_URL, url.c_str());
                curl_easy_setopt(writeHandle, CURLOPT_POST, 1);
                return writeHandle;
            }

            throw InfluxDBException{__func__, "Failed to initialize write handle"};
        }

        std::string curl_easy_escape_wrapper(std::string str)
        {
          char *escapedStr = curl_easy_escape(NULL, str.c_str(), static_cast<int>(str.size()));
          std::string res = std::string(escapedStr);
          curl_free(escapedStr);
          return res;
        }
    }

HTTP::HTTP(const internal::ConnectionInfo &conn)
{
  obtainInfluxServiceUrl(conn);
  initCurlWrite(conn);
  initCurlRead(conn);
  obtainDatabaseName(conn);
  this->dbVersion = conn.dbVersion;
}

HTTP::~HTTP()
{
  curl_easy_cleanup(writeHandle);
  curl_easy_cleanup(readHandle);
  curl_global_cleanup();
}

void HTTP::initCurlWrite(internal::ConnectionInfo conn)
{
  if (const CURLcode globalInitResult = curl_global_init(CURL_GLOBAL_ALL); globalInitResult != CURLE_OK)
  {
    throw InfluxDBException(__func__, curl_easy_strerror(globalInitResult));
  }

  if (conn.dbName.size() <= 0)
  {
    throw InfluxDBException(__func__, "Database not specified");
  }

  mWriteUrl = mInfluxDbServiceUrl;
  mWriteUrl += "/write?db=";
  mWriteUrl += curl_easy_escape_wrapper(conn.dbName);
  if (conn.dbVersion == 2 && conn.rp.size() > 0)
  {
    mWriteUrl += "&rp=" + curl_easy_escape_wrapper(conn.rp);
  }

  writeHandle = createWriteHandle(mWriteUrl);
}

void HTTP::initCurlRead(internal::ConnectionInfo conn)
{
  if (conn.dbName.size() <= 0)
  {
    throw InfluxDBException(__func__, "Database not specified");
  }
  mReadUrl = mInfluxDbServiceUrl;
  mReadUrl += "/query?db=";
  mReadUrl += curl_easy_escape_wrapper(conn.dbName);
  if (conn.dbVersion == 2 && conn.rp.size() > 0)
  {
    mReadUrl += "&rp=" + curl_easy_escape_wrapper(conn.rp);
  }

  readHandle = createReadHandle();
}

std::string HTTP::query(const std::string &query, const InfluxDBParams &params)
{
  std::string buffer;
  auto fullUrl = mReadUrl + "&q=" + curl_easy_escape_wrapper(query);
  if (params.size() > 0)
  {
    fullUrl += "&params=" + curl_easy_escape_wrapper(params.toJSON());
  }
  curl_easy_setopt(readHandle, CURLOPT_URL, fullUrl.c_str());
  curl_easy_setopt(readHandle, CURLOPT_WRITEDATA, &buffer);
  const CURLcode response = curl_easy_perform(readHandle);
  long responseCode{0};
  curl_easy_getinfo(readHandle, CURLINFO_RESPONSE_CODE, &responseCode);
  treatCurlResponse(response, responseCode, buffer);
  return buffer;
}

void HTTP::enableBasicAuth(const std::string &auth)
{
  curl_easy_setopt(writeHandle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
  curl_easy_setopt(writeHandle, CURLOPT_USERPWD, auth.c_str());
  curl_easy_setopt(readHandle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
  curl_easy_setopt(readHandle, CURLOPT_USERPWD, auth.c_str());
}

void HTTP::enableTokenAuth(const std::string &token)
{
  struct curl_slist *list = NULL;
  std::string auth = "Authorization: Token " + token;
  list = curl_slist_append(list, auth.c_str());
  curl_easy_setopt(readHandle, CURLOPT_HTTPHEADER, list);
  curl_easy_setopt(writeHandle, CURLOPT_HTTPHEADER, list);
}

void HTTP::send(std::string &&lineprotocol)
{
  std::string buffer;
  curl_easy_setopt(writeHandle, CURLOPT_WRITEDATA, &buffer);
  curl_easy_setopt(writeHandle, CURLOPT_POSTFIELDS, lineprotocol.c_str());
  curl_easy_setopt(writeHandle, CURLOPT_POSTFIELDSIZE, static_cast<long>(lineprotocol.length()));
  const CURLcode response = curl_easy_perform(writeHandle);
  long responseCode{0};
  curl_easy_getinfo(writeHandle, CURLINFO_RESPONSE_CODE, &responseCode);
  treatCurlResponse(response, responseCode, buffer);
}

void HTTP::treatCurlResponse(const CURLcode &response, long responseCode, std::string buffer) const
{
  if (response != CURLE_OK)
  {
    throw ConnectionError(__func__, curl_easy_strerror(response));
  }
  //
  // Influx API response codes:
  // https://docs.influxdata.com/influxdb/v1.7/tools/api/#status-codes-and-responses-2
  //
  if (responseCode == 404)
  {
    throw NonExistentDatabase(__func__, "Nonexistent database: " + internal::parseErrorMessage(buffer));
  }
  if ((responseCode >= 400) && (responseCode < 500))
  {
    throw BadRequest(__func__, "Bad request: " + internal::parseErrorMessage(buffer));
  }
  if (responseCode >= 500)
  {
    throw ServerError(__func__, "Influx server error: " + internal::parseErrorMessage(buffer));
  }
}

void HTTP::obtainInfluxServiceUrl(internal::ConnectionInfo conn)
{
  mInfluxDbServiceUrl = conn.host + ":" + std::to_string(conn.port);
}

void HTTP::obtainDatabaseName(internal::ConnectionInfo conn)
{
  mDatabaseName = conn.dbName;
}

std::string HTTP::databaseName() const
{
  return mDatabaseName;
}

std::string HTTP::influxDbServiceUrl() const
{
  return mInfluxDbServiceUrl;
}

void HTTP::createDatabase()
{
  if (dbVersion != 1)
    throw InfluxDBException{__func__, "Does not support create database for InfluxDB version 2."};

  this->query("CREATE DATABASE " + mDatabaseName);
}

} // namespace influxdb
