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

#include "Point.h"
#include "LineProtocol.h"
#include <chrono>
#include <memory>
#include <sstream>
#include <iomanip>
#include <stdio.h>

namespace influxdb
{

  namespace
  {
    /// Escape for tag keys, tag values and field keys
    std::string escapeKey(const std::string &key)
    {
      std::string ret = {};
      /// Set the maximum length equal to the length when all characters
      /// need escaping.
      ret.reserve(key.length() * 2);
      for (char c: key)
      {
        switch (c)
        {
          case ',':
          case '=':
          case ' ':
              ret += '\\';
          break;
        }
        ret += c;
      }
      return ret;
    }

    /// Escape for measurement name
    std::string escapeMeasurement(const std::string &key)
    {
      std::string ret = {};
      /// Set the maximum length equal to the length when all characters
      /// need escaping.
      ret.reserve(key.length() * 2);
      for (char c: key)
      {
        switch (c)
        {
          case ',':
          case ' ':
              ret += '\\';
          break;
        }
        ret += c;
      }
      return ret;
    }

    /// Escape for string field
    std::string escapeStringValue(const std::string &value) {
      std::string ret;
      /// Set the maximum length equal to the length when all characters
      /// need escaping. String field must be in double quotes.
      ret.reserve(value.length() * 2 + 2);
      ret += '"';
      for(char c: value)
      {
        switch (c)
        {
            case '\\':
            case '\"':
                ret += '\\';
                break;
        }
        ret += c;
      }
      ret += '"';
      return ret;
    }
  }

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

Point::Point(const std::string& measurement) :
  mMeasurement(measurement), mTimestamp(Point::getCurrentTimestamp()), mTags({}), mFields({})
{
}

Point&& Point::addField(std::string_view name, const std::variant<bool, int, long long int, const char *, std::string, double>& value)
{
  if (name.empty())
  {
    return std::move(*this);
  }

  mFields.emplace_back(std::make_pair(name, value));
  return std::move(*this);

}

Point&& Point::addTag(std::string_view key, std::string_view value)
{
  if (key.empty() || value.empty())
  {
    return std::move(*this);
  }

  mTags.emplace_back(std::make_pair(key,value));
  return std::move(*this);
}

Point&& Point::setTimestamp(std::chrono::time_point<std::chrono::system_clock> timestamp)
{
  mTimestamp = timestamp;
  return std::move(*this);
}

Point&& Point::setTimestamp(long long nanos)
{
  std::chrono::nanoseconds dur(nanos);
  std::chrono::time_point<std::chrono::system_clock> timestamp(dur);
  return setTimestamp(timestamp);
}

auto Point::getCurrentTimestamp() -> decltype(std::chrono::system_clock::now())
{
  return std::chrono::system_clock::now();
}

std::string Point::toLineProtocol() const
{
    LineProtocol formatter;
    return formatter.format(*this);
}

std::string Point::getName() const
{
  return escapeMeasurement(mMeasurement);
}

std::chrono::time_point<std::chrono::system_clock> Point::getTimestamp() const
{
  return mTimestamp;
}

std::string Point::getFields() const
{
  std::string fields = "";

  for (const auto& field : mFields)
  {
    if (!fields.empty())
    {
      fields += ",";
    }

    fields += escapeKey(field.first) + "=";
    std::visit(overloaded {
      [&fields](bool v) { fields += (v == true ? "true" : "false"); },
      [&fields](int v) { fields += std::to_string(v) + 'i'; },
      [&fields](long long int v) { fields += std::to_string(v) + 'i'; },
      [&fields](double v) {
          char s[128];
          sprintf(s, "%.*f", floatsPrecision, v);
          fields += std::string(s);
        },
      [&fields](const std::string& v) { fields += escapeStringValue(v); },
      [&fields](const char *v) { fields += escapeStringValue(std::string(v)); },
    }, field.second);
  }

  return fields;
}

std::string Point::getTags() const
{
    if (mTags.empty())
    {
        return "";
    }

    std::string tags;
    for (const auto& tag : mTags)
    {
        tags += ",";
        tags += escapeKey(tag.first);
        tags += "=";
        tags += escapeKey(tag.second);
    }

    return tags.substr(1, tags.size());
}

} // namespace influxdb
