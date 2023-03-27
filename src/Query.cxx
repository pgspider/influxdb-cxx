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

#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>
#include "Query.h"
#include "InfluxDBParams.h"

namespace influxdb::internal
{
    namespace
    {
        /// Serialize rapidjson::Value to std::string
        std::string jsonToString(const rapidjson::Value *jsonVal)
        {
            if (jsonVal->IsNull())
            {
                /// return empty string for NULL json
                return std::string();
            }
            else if (jsonVal->IsBool())
            {
                /// treat bool value as text
                if (jsonVal->GetBool())
                    return std::string("true");
                else
                    return std::string("false");
            }
            else if (jsonVal->IsString())
            {
                /// all numbers are parse as string
                return jsonVal->GetString();
            }
            else
            {
                /// serialize
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                jsonVal->Accept(writer);
                return buffer.GetString();
            }

        }

        /// parse InfluxQL JSON response
        std::vector<InfluxDBTable> parseJsonResponse(const std::string &response)
        {
            std::vector<InfluxDBTable> resultSet;
            rapidjson::Document responseDoc;

            /// Parse json string to RapidJSON document,
            /// all numbers are parsed as strings to avoid loss of precision.
            if (responseDoc.Parse<rapidjson::kParseNumbersAsStringsFlag>(response.c_str()).HasParseError())
                throw InfluxDBException("Query", "Parse json false: " + responseDoc.GetParseError());

            if (!responseDoc.IsObject())
                throw InfluxDBException("Query", "Unsupported json structure");

            if (!responseDoc.HasMember("results"))
                return resultSet;

            /// Results element must be a JSON array
            auto &results = responseDoc["results"];
            if (!results.IsArray())
                throw InfluxDBException("Query", "Unsupported json structure");

            /* Parser result */
            for (rapidjson::SizeType resultIdx = 0; resultIdx < results.Size(); resultIdx++)
            {
                InfluxDBTable statementResult;
                statementResult.series = {};
                auto &iResult = results[resultIdx];

                /// Elements of resutls array must be a JSON object
                if (!iResult.IsObject())
                    throw InfluxDBException("Query", "Unsupported json structure");

                /// Get error message if existed
                if (iResult.HasMember("error"))
                {
                    /// Error message existed, do not parse other element.
                    statementResult.error = jsonToString(&iResult["error"]);
                    resultSet.push_back(std::move(statementResult));
                    continue;
                }

                /// Get statement ID if existed
                if (iResult.HasMember("statement_id"))
                {
                    statementResult.statementId = std::stoi(jsonToString(&iResult["statement_id"]));
                }

                /// Get series if existed
                if (iResult.HasMember("series"))
                {
                    auto &series = iResult["series"];
                    /// series must be a JSON array
                    if (!series.IsArray())
                        throw InfluxDBException("Query", "Unsupported json structure");

                    for (rapidjson::SizeType seriesIdx = 0; seriesIdx < series.Size(); seriesIdx++)
                    {
                        InfluxDBSeries resultSeries;
                        auto &iSeries = series[seriesIdx];

                        if (!iSeries.IsObject())
                            throw InfluxDBException("Query", "Unsupported json structure");

                        /// Get name
                        if (iSeries.HasMember("name"))
                            resultSeries.name = jsonToString(&iSeries["name"]);

                        /// get tags
                        if (iSeries.HasMember("tags"))
                        {
                            if (const auto &tags = iSeries["tags"]; tags.IsObject())
                            {
                                if (!tags.IsNull())
                                {
                                    for (auto iTags = tags.MemberBegin(); iTags != tags.MemberEnd(); iTags++)
                                    {
                                        resultSeries.tagKeys.push_back(jsonToString(&iTags->name));
                                        resultSeries.tagValues.push_back(jsonToString(&iTags->value));
                                    }
                                }
                            }
                        }

                        /// Get column name
                        if (iSeries.HasMember("columns"))
                        {
                            auto &columns = iSeries["columns"];
                            std::vector<std::string> colnames;
                            for (rapidjson::SizeType columnIdx = 0; columnIdx != columns.Size(); ++columnIdx)
                            {
                                colnames.push_back(jsonToString(&columns[columnIdx]));
                            }
                            resultSeries.columnNames = std::move(colnames);
                        }

                        /// Get rows value
                        if (iSeries.HasMember("values"))
                        {
                            const auto &values = iSeries["values"];
                            /// values must be an JSON array
                            if (!values.IsArray())
                                throw InfluxDBException("Query", "Unsupported json structure");

                            for (rapidjson::SizeType valuesIdx = 0; valuesIdx < values.Size(); ++valuesIdx)
                            {
                                InfluxDBRow row;
                                auto &iValue = values[valuesIdx];
                                /// Get a tuple
                                for (rapidjson::SizeType columnIdx = 0; columnIdx != iValue.Size(); ++columnIdx)
                                {
                                    row.tuple.push_back(jsonToString(&iValue[columnIdx]));
                                }
                                resultSeries.rows.push_back(std::move(row));
                            }
                        }
                        statementResult.series.push_back(std::move(resultSeries));
                    }
                }
                resultSet.push_back(std::move(statementResult));
            }
            return resultSet;
        }
    }

    std::vector<InfluxDBTable> queryImpl(Transport* transport, const std::string& query, const InfluxDBParams &param)
    {
        const auto response = transport->query(query, param);
        return parseJsonResponse(response);
    }

    std::string parseErrorMessage(const std::string& buffer)
    {
        rapidjson::Document js;
        std::string errMsg = {};
        if (js.Parse(buffer.c_str()).HasParseError())
            throw InfluxDBException("Query", "Parse json error: " + std::string(rapidjson::GetParseError_En(js.GetParseError())));

        if (!js.IsObject())
            throw InfluxDBException("Query", "Unsupported json structure");

        /// error message for InfluxDB 1.x
        if (js.HasMember("error"))
        {
            errMsg += "ERROR: " + jsonToString(&js["error"]);
            return errMsg;
        }

        /// error message for InfluxDB 2.x
        if (js.HasMember("code"))
        {
            errMsg += "CODE: " + jsonToString(&js["code"]);
        }
        if (js.HasMember("message"))
        {
            if (errMsg.length() > 0)
                errMsg += ", ";
            errMsg += "MESSAGE: " + jsonToString(&js["message"]);
        }

        return errMsg;
    }
}
