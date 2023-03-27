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

#include "InfluxDBParams.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace influxdb
{
    InfluxDBParams&& InfluxDBParams::addParam(std::string name, std::variant<bool, int, long long int, const char *, std::string, double> value)
    {
        if (name.empty())
        {
            return std::move(*this);
        }

        params.emplace_back(std::make_pair(name, value));
        return std::move(*this);
    }

    template <class... Fs> struct Overload : Fs... { using Fs::operator()...; };
    template <class... Fs> Overload(Fs...) -> Overload<Fs...>;

    std::string InfluxDBParams::toJSON() const
    {
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);

        writer.StartObject();
        for (const auto& param : params)
        {
            writer.Key(param.first.c_str());
            std::visit(
            Overload{
                    [&writer] (bool v) { writer.Bool(v); },
                    [&writer] (int v) { writer.Int(v); },
                    [&writer] (long long int v) { writer.Int64(v); },
                    [&writer] (const char * v) { writer.String(v); },
                    [&writer] (std::string v) { writer.String(v.c_str()); },
                    [&writer] (double v) { writer.Double(v); }
                },
                param.second
            );
        }
        writer.EndObject();

        return s.GetString();
    }

    size_t InfluxDBParams::size() const
    {
        return params.size();
    }
}
