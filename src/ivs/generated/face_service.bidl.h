/**************************************************************************************************
 * @file        face_service.bidl.h
 * @brief       Auto-generated OmniBinder C++ declarations
 * @details     Generated from OmniBinder IDL for C++ declarations and client/server bindings.
 *              Source IDL: face_service.bidl.bidl
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
#ifndef FACE_SERVICE_BIDL_H
#define FACE_SERVICE_BIDL_H

#include <omnibinder/omnibinder.h>
#include <omnibinder/proxy_base.h>
#include <string>
#include <vector>
#include <functional>

#include "common_types.bidl.h"

namespace mediaipc {

struct FaceDet {
    float score;
    float x1;
    float y1;
    float x2;
    float y2;
    std::vector<float> kps;

    FaceDet() : score(0), x1(0), y1(0), x2(0), y2(0) {}
    bool serialize(omnibinder::Buffer& buf) const;
    bool deserialize(omnibinder::Buffer& buf);
};

struct FaceCandidate {
    std::string name;
    float cosine;

    FaceCandidate() : cosine(0) {}
    bool serialize(omnibinder::Buffer& buf) const;
    bool deserialize(omnibinder::Buffer& buf);
};

struct FaceItem {
    FaceDet det;
    float yaw;
    float pitch;
    float roll;
    bool matched;
    std::string name;
    float cosine;
    std::vector<FaceCandidate> alts;

    FaceItem() : yaw(0), pitch(0), roll(0), matched(0), cosine(0) {}
    bool serialize(omnibinder::Buffer& buf) const;
    bool deserialize(omnibinder::Buffer& buf);
};

struct FacePerf {
    float fps_link;
    float exec_det_ms;
    float exec_mbf_ms;
    float decode_ms;
    float crop_copy_ms;

    FacePerf() : fps_link(0), exec_det_ms(0), exec_mbf_ms(0), decode_ms(0), crop_copy_ms(0) {}
    bool serialize(omnibinder::Buffer& buf) const;
    bool deserialize(omnibinder::Buffer& buf);
};

struct FaceResult {
    uint64_t seq;
    uint32_t buffer_id;
    uint32_t det_count;
    std::vector<FaceDet> dets;
    FaceItem best;
    int32_t best_index;
    float score_thr;
    float match_thr;
    FacePerf perf;
    uint64_t ts_us;

    static const uint32_t TOPIC_ID = 0x7fac31c5u;
    static const uint32_t TOPIC_IDL_HASH = 0x368f4259u;

    FaceResult() : seq(0), buffer_id(0), det_count(0), best_index(0), score_thr(0), match_thr(0), ts_us(0) {}
    bool serialize(omnibinder::Buffer& buf) const;
    bool deserialize(omnibinder::Buffer& buf);
};

class FaceServiceStub : public omnibinder::Service {
public:
    FaceServiceStub() : Service("FaceService") {}
    virtual ~FaceServiceStub() {}

    const omnibinder::InterfaceInfo& interfaceInfo() const override;

    virtual int32_t GetStatus() = 0;
    virtual int32_t GetFrameCount() = 0;
    int PublishFaceResult();
    int BroadcastFaceResult(const FaceResult& msg);

protected:
    int onInvoke(uint32_t method_id, const omnibinder::Buffer& request, omnibinder::Buffer& response) override;
};

class FaceServiceProxy : public omnibinder::ServiceProxyBase {
public:
    explicit FaceServiceProxy(omnibinder::OmniRuntime& runtime)
        : ServiceProxyBase(runtime, "FaceService") {}
    ~FaceServiceProxy() {}

    int GetStatus(int32_t& out);
    int GetFrameCount(int32_t& out);
    int SubscribeFaceResult(const std::function<void(const FaceResult&)>& callback);

    // Inherited from ServiceProxyBase: connect(), disconnect(), isConnected(),
    //   enableAutoReconnect(), startHeartbeat(), stopHeartbeat(), OnServiceDied()
};

} // namespace mediaipc

#endif // FACE_SERVICE_BIDL_H
