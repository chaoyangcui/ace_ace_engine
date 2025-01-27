/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "base/log/log_wrapper.h"

#ifdef WINDOWS_PLATFORM
#include <cstdio>
#else
#include "securec.h"
#endif
#include <thread>

namespace OHOS::Ace {
namespace {

constexpr uint32_t MAX_BUFFER_SIZE = 512;
constexpr uint32_t MAX_TIME_SIZE = 32;
const char* const LOGLEVELNAME[] = { "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };

const char* LOG_TAGS[] = {
    "Ace",
    "Console",
};

} // namespace

// initial static member object
LogLevel LogWrapper::level_ = LogLevel::DEBUG;

const char* GetNameForLogLevel(LogLevel level)
{
    if (level <= LogLevel::FATAL) {
        return LOGLEVELNAME[static_cast<int>(level)];
    }
    return "UNKNOWN";
}

char LogWrapper::GetSeparatorCharacter()
{
    return '/';
}

std::string GetTimeStamp()
{
    time_t tt = time(nullptr);
    tm* t = localtime(&tt);
    char time[MAX_TIME_SIZE];

    if (sprintf_s(time, MAX_TIME_SIZE, " %02d/%02d %02d:%02d:%02d", t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min,
            t->tm_sec) < 0) {
        return std::string();
    }
    return std::string(time);
}

void LogWrapper::PrintLog(LogDomain domain, LogLevel level, const char* fmt, va_list args)
{
    std::string newFmt(fmt);
    StripFormatString("{public}", newFmt);
    StripFormatString("{private}", newFmt);

    char buf[MAX_BUFFER_SIZE];
    if (vsnprintf_s(buf, sizeof(buf), sizeof(buf) - 1, newFmt.c_str(), args) < 0) {
        return;
    }

    std::string newTimeFmt("%s %-6d [%-7s %7s]");
    char timeBuf[MAX_BUFFER_SIZE];
#ifdef WINDOWS_PLATFORM
    if (_snprintf_s(timeBuf, sizeof(timeBuf), sizeof(timeBuf) - 1, newTimeFmt.c_str(), GetTimeStamp().c_str(),
            std::this_thread::get_id(), LOG_TAGS[static_cast<uint32_t>(domain)], GetNameForLogLevel(level)) < 0) {
        return;
    }
#else
    if (snprintf_s(timeBuf, sizeof(timeBuf), sizeof(timeBuf) - 1, newTimeFmt.c_str(), GetTimeStamp().c_str(),
            std::this_thread::get_id(), LOG_TAGS[static_cast<uint32_t>(domain)], GetNameForLogLevel(level)) < 0) {
        return;
    }
#endif
    printf("%s  %s\r\n", timeBuf, buf);
    fflush(stdout);
}

} // namespace OHOS::Ace
