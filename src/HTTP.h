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
/// \author Adam Wegrzynek
///

#ifndef INFLUXDATA_TRANSPORTS_HTTP_H
#define INFLUXDATA_TRANSPORTS_HTTP_H

#include "Transport.h"
#include "ConnectionInfo.h"
#include <curl/curl.h>
#include <memory>
#include <string>

namespace influxdb::transports
{

/// \brief HTTP transport
class HTTP : public Transport
{
public:
  /// Constructor
  explicit HTTP(const internal::ConnectionInfo &conn);

  /// Default destructor
  ~HTTP() override;

  /// Sends point via HTTP POST
  ///  \throw InfluxDBException	when CURL fails on POSTing or response code != 200
  void send(std::string &&lineprotocol) override;

  /// Queries database
  /// \throw InfluxDBException	when CURL GET fails
  std::string query(const std::string &query, const InfluxDBParams &params = InfluxDBParams()) override;

  /// Creates database used at url if it does not exists
  /// \throw InfluxDBException	when CURL POST fails
  void createDatabase() override;

  /// Enable Basic Auth
  /// \param auth <username>:<password>
  void enableBasicAuth(const std::string &auth);

  /// Enable Token Auth
  void enableTokenAuth(const std::string &token);

  /// Get the database name managed by this transport
  [[nodiscard]] std::string databaseName() const;

  /// Get the influx service url which transport connects to
  [[nodiscard]] std::string influxDbServiceUrl() const;

private:

  /// Obtain InfluxDB service url from the url passed
  void obtainInfluxServiceUrl(internal::ConnectionInfo conn);

  /// Obtain database name from the url passed
  void obtainDatabaseName(internal::ConnectionInfo conn);

  /// Initializes CURL for writing and common options
  /// \throw InfluxDBException	if database not specified
  void initCurlWrite(internal::ConnectionInfo conn);

  /// Initializes CURL for reading
  /// \throw InfluxDBException	if database not specified
  void initCurlRead(internal::ConnectionInfo conn);

  /// treats responses of CURL requests
  void treatCurlResponse(const CURLcode &response, long responseCode, std::string buffer) const;

  /// CURL pointer configured for writing points
  CURL *writeHandle;

  /// InfluxDB write URL
  std::string mWriteUrl;

  /// CURL pointer configured for querying
  CURL *readHandle;

  /// InfluxDB read URL
  std::string mReadUrl;

  /// InfluxDB service URL
  std::string mInfluxDbServiceUrl;

  /// Database name used
  std::string mDatabaseName;

  /// Database version
  uint8_t     dbVersion;
};

} // namespace influxdb

#endif // INFLUXDATA_TRANSPORTS_HTTP_H
