// MIT License
//
// Copyright (c) 2022 TOSHIBA CORPORATION
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

#pragma once

#include <string>
#include <vector>
#include "influxdb_export.h"

namespace influxdb
{
    /// \brief Represents a series row
    class INFLUXDB_EXPORT InfluxDBRow
    {
    public:
        /// Row tuple
        std::vector<std::string> tuple;
    };

    /// \brief Represents a series
    class INFLUXDB_EXPORT InfluxDBSeries
    {
    public:
        /// Serie name
        std::string name;
        /// tag names list
        std::vector<std::string> tagKeys;
        /// tag value list
        std::vector<std::string> tagValues;
        /// column name list
        std::vector<std::string> columnNames;
        /// row list
        std::vector<InfluxDBRow> rows;
    };

    /// \brief Represents a query result
    class INFLUXDB_EXPORT InfluxDBTable
    {
    public:
        InfluxDBTable()
        {
            series = std::vector<InfluxDBSeries>();
            statementId = -1;
        }
        /// statement id
        int     statementId;
        /// series
        std::vector<InfluxDBSeries> series;
        /// statement error message (default is {})
        std::string error;
    };
} /* !influxdb */
