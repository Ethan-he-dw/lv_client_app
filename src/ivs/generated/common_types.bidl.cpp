/**************************************************************************************************
 * @file        common_types.bidl.cpp
 * @brief       Auto-generated OmniBinder C++ definitions
 * @details     Generated from OmniBinder IDL for C++ serialization and proxy/stub implementation glue.
 *              Source IDL: common_types.bidl.bidl
 *              This file is auto-generated. DO NOT EDIT MANUALLY.
 *
 * Copyright (c) 2025 taoist.luo (https://github.com/TaoistLuo/OmniBinder)
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *************************************************************************************************/
#include "common_types.bidl.h"
#include <stdexcept>

namespace common {

bool Timestamp::serialize(omnibinder::Buffer& buf) const {
    if (!buf.writeInt64(seconds)) return false;
    if (!buf.writeInt32(nanos)) return false;
    return true;
}

bool Timestamp::deserialize(omnibinder::Buffer& buf) {
    if (!buf.tryReadInt64(seconds)) return false;
    if (!buf.tryReadInt32(nanos)) return false;
    return true;
}

bool Status::serialize(omnibinder::Buffer& buf) const {
    if (!buf.writeInt32(code)) return false;
    if (!buf.writeString(message)) return false;
    return true;
}

bool Status::deserialize(omnibinder::Buffer& buf) {
    if (!buf.tryReadInt32(code)) return false;
    if (!buf.tryReadString(message)) return false;
    return true;
}

} // namespace common
