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
#include <deque>
#include <variant>
#include "influxdb_export.h"

namespace influxdb
{
    /// \brief Represents InfluxQL parameters
    class INFLUXDB_EXPORT InfluxDBParams
    {
    public:
        /// add a param to param list
        InfluxDBParams&& addParam(std::string name, std::variant<bool, int, long long int, const char *, std::string, double> value);
        /// get list params in JSON value
        std::string toJSON() const;
        /// get param list size
        size_t size() const;

    private:
        /// params list
        std::deque<std::pair<std::string, std::variant<bool, int, long long int, const char *, std::string, double>>> params;
    };
}
