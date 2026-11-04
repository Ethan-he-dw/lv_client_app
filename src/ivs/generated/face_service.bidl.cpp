/**************************************************************************************************
 * @file        face_service.bidl.cpp
 * @brief       Auto-generated OmniBinder C++ definitions
 * @details     Generated from OmniBinder IDL for C++ serialization and proxy/stub implementation glue.
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
#include "face_service.bidl.h"
#include <stdexcept>

namespace mediaipc {

bool FaceDet::serialize(omnibinder::Buffer& buf) const {
    if (!buf.writeFloat32(score)) return false;
    if (!buf.writeFloat32(x1)) return false;
    if (!buf.writeFloat32(y1)) return false;
    if (!buf.writeFloat32(x2)) return false;
    if (!buf.writeFloat32(y2)) return false;
    if (!buf.writeUint32(static_cast<uint32_t>(kps.size()))) return false;
    for (size_t i0 = 0; i0 < kps.size(); ++i0) {
        if (!buf.writeFloat32(kps[i0])) return false;
    }
    return true;
}

bool FaceDet::deserialize(omnibinder::Buffer& buf) {
    if (!buf.tryReadFloat32(score)) return false;
    if (!buf.tryReadFloat32(x1)) return false;
    if (!buf.tryReadFloat32(y1)) return false;
    if (!buf.tryReadFloat32(x2)) return false;
    if (!buf.tryReadFloat32(y2)) return false;
    { uint32_t cnt0 = 0; if (!buf.tryReadUint32(cnt0)) return false; if (cnt0 > omnibinder::MAX_ARRAY_ELEMENTS) return false; if (static_cast<size_t>(cnt0) > buf.remaining() / 4u) return false; if (static_cast<size_t>(cnt0) > omnibinder::MAX_MESSAGE_SIZE / sizeof(float)) return false; try { kps.resize(cnt0); } catch (...) { return false; }
      for (uint32_t i0 = 0; i0 < cnt0; ++i0) {
        if (!buf.tryReadFloat32(kps[i0])) return false;
      }}
    return true;
}

bool FaceCandidate::serialize(omnibinder::Buffer& buf) const {
    if (!buf.writeString(name)) return false;
    if (!buf.writeFloat32(cosine)) return false;
    return true;
}

bool FaceCandidate::deserialize(omnibinder::Buffer& buf) {
    if (!buf.tryReadString(name)) return false;
    if (!buf.tryReadFloat32(cosine)) return false;
    return true;
}

bool FaceItem::serialize(omnibinder::Buffer& buf) const {
    if (!det.serialize(buf)) return false;
    if (!buf.writeFloat32(yaw)) return false;
    if (!buf.writeFloat32(pitch)) return false;
    if (!buf.writeFloat32(roll)) return false;
    if (!buf.writeBool(matched)) return false;
    if (!buf.writeString(name)) return false;
    if (!buf.writeFloat32(cosine)) return false;
    if (!buf.writeUint32(static_cast<uint32_t>(alts.size()))) return false;
    for (size_t i0 = 0; i0 < alts.size(); ++i0) {
        if (!alts[i0].serialize(buf)) return false;
    }
    return true;
}

bool FaceItem::deserialize(omnibinder::Buffer& buf) {
    if (!det.deserialize(buf)) return false;
    if (!buf.tryReadFloat32(yaw)) return false;
    if (!buf.tryReadFloat32(pitch)) return false;
    if (!buf.tryReadFloat32(roll)) return false;
    if (!buf.tryReadBool(matched)) return false;
    if (!buf.tryReadString(name)) return false;
    if (!buf.tryReadFloat32(cosine)) return false;
    { uint32_t cnt0 = 0; if (!buf.tryReadUint32(cnt0)) return false; if (cnt0 > omnibinder::MAX_ARRAY_ELEMENTS) return false; if (cnt0 > omnibinder::MAX_ZERO_WIRE_ARRAY_ELEMENTS) return false; if (static_cast<size_t>(cnt0) > omnibinder::MAX_MESSAGE_SIZE / sizeof(FaceCandidate)) return false; try { alts.resize(cnt0); } catch (...) { return false; }
      for (uint32_t i0 = 0; i0 < cnt0; ++i0) {
        if (!alts[i0].deserialize(buf)) return false;
      }}
    return true;
}

bool FacePerf::serialize(omnibinder::Buffer& buf) const {
    if (!buf.writeFloat32(fps_link)) return false;
    if (!buf.writeFloat32(exec_det_ms)) return false;
    if (!buf.writeFloat32(exec_mbf_ms)) return false;
    if (!buf.writeFloat32(decode_ms)) return false;
    if (!buf.writeFloat32(crop_copy_ms)) return false;
    return true;
}

bool FacePerf::deserialize(omnibinder::Buffer& buf) {
    if (!buf.tryReadFloat32(fps_link)) return false;
    if (!buf.tryReadFloat32(exec_det_ms)) return false;
    if (!buf.tryReadFloat32(exec_mbf_ms)) return false;
    if (!buf.tryReadFloat32(decode_ms)) return false;
    if (!buf.tryReadFloat32(crop_copy_ms)) return false;
    return true;
}

bool FaceResult::serialize(omnibinder::Buffer& buf) const {
    if (!buf.writeUint64(seq)) return false;
    if (!buf.writeUint32(buffer_id)) return false;
    if (!buf.writeUint32(det_count)) return false;
    if (!buf.writeUint32(static_cast<uint32_t>(dets.size()))) return false;
    for (size_t i0 = 0; i0 < dets.size(); ++i0) {
        if (!dets[i0].serialize(buf)) return false;
    }
    if (!best.serialize(buf)) return false;
    if (!buf.writeInt32(best_index)) return false;
    if (!buf.writeFloat32(score_thr)) return false;
    if (!buf.writeFloat32(match_thr)) return false;
    if (!perf.serialize(buf)) return false;
    if (!buf.writeUint64(ts_us)) return false;
    return true;
}

bool FaceResult::deserialize(omnibinder::Buffer& buf) {
    if (!buf.tryReadUint64(seq)) return false;
    if (!buf.tryReadUint32(buffer_id)) return false;
    if (!buf.tryReadUint32(det_count)) return false;
    { uint32_t cnt0 = 0; if (!buf.tryReadUint32(cnt0)) return false; if (cnt0 > omnibinder::MAX_ARRAY_ELEMENTS) return false; if (cnt0 > omnibinder::MAX_ZERO_WIRE_ARRAY_ELEMENTS) return false; if (static_cast<size_t>(cnt0) > omnibinder::MAX_MESSAGE_SIZE / sizeof(FaceDet)) return false; try { dets.resize(cnt0); } catch (...) { return false; }
      for (uint32_t i0 = 0; i0 < cnt0; ++i0) {
        if (!dets[i0].deserialize(buf)) return false;
      }}
    if (!best.deserialize(buf)) return false;
    if (!buf.tryReadInt32(best_index)) return false;
    if (!buf.tryReadFloat32(score_thr)) return false;
    if (!buf.tryReadFloat32(match_thr)) return false;
    if (!perf.deserialize(buf)) return false;
    if (!buf.tryReadUint64(ts_us)) return false;
    return true;
}

static const uint32_t s_FaceResult_topic_idl_hash = 0x368f4259u;

static omnibinder::InterfaceInfo s_FaceService_info;
static bool s_FaceService_info_init = false;

const omnibinder::InterfaceInfo& FaceServiceStub::interfaceInfo() const {
    if (!s_FaceService_info_init) {
        s_FaceService_info.interface_id = 0x7c3f240du;
        s_FaceService_info.name = "FaceService";
        { omnibinder::MethodInfo mi(0x2b1f41a1u, "GetStatus", "", "int32_t"); mi.idl_hash = 0xbea1a15bu; s_FaceService_info.methods.push_back(mi); }
        { omnibinder::MethodInfo mi(0xe5e52523u, "GetFrameCount", "", "int32_t"); mi.idl_hash = 0xb5a5d93du; s_FaceService_info.methods.push_back(mi); }
        s_FaceService_info_init = true;
    }
    return s_FaceService_info;
}

int FaceServiceStub::onInvoke(uint32_t method_id, const omnibinder::Buffer& request, omnibinder::Buffer& response) {
    omnibinder::Buffer req(request.data(), request.size());
    switch (method_id) {
    case 0x2b1f41a1u: {
        int32_t result = this->GetStatus();
        if (!response.writeInt32(result)) return static_cast<int>(omnibinder::ErrorCode::ERR_SERIALIZE);
        return 0;
    }
    case 0xe5e52523u: {
        int32_t result = this->GetFrameCount();
        if (!response.writeInt32(result)) return static_cast<int>(omnibinder::ErrorCode::ERR_SERIALIZE);
        return 0;
    }
    default: return static_cast<int>(omnibinder::ErrorCode::ERR_METHOD_NOT_FOUND);
    }
}

int FaceServiceStub::PublishFaceResult() {
    if (!runtime()) return static_cast<int>(omnibinder::ErrorCode::ERR_NOT_INITIALIZED);
    return runtime()->publishTopic(serviceName(), "FaceResult", s_FaceResult_topic_idl_hash);
}

int FaceServiceStub::BroadcastFaceResult(const FaceResult& msg) {
    omnibinder::Buffer buf;
    msg.serialize(buf);
    if (!runtime()) return static_cast<int>(omnibinder::ErrorCode::ERR_NOT_INITIALIZED);
    return runtime()->broadcast(0x7fac31c5u, buf);
}

int FaceServiceProxy::GetStatus(int32_t& out) {
    omnibinder::Buffer req, resp;
    int ret = runtime_.invoke("FaceService", 0x7c3f240du, 0x2b1f41a1u, 0xbea1a15bu, req, resp, 0);
    if (ret != 0) return ret;
    if (!resp.tryReadInt32(out)) return static_cast<int>(omnibinder::ErrorCode::ERR_DESERIALIZE);
    return 0;
}

int FaceServiceProxy::GetFrameCount(int32_t& out) {
    omnibinder::Buffer req, resp;
    int ret = runtime_.invoke("FaceService", 0x7c3f240du, 0xe5e52523u, 0xb5a5d93du, req, resp, 0);
    if (ret != 0) return ret;
    if (!resp.tryReadInt32(out)) return static_cast<int>(omnibinder::ErrorCode::ERR_DESERIALIZE);
    return 0;
}

int FaceServiceProxy::SubscribeFaceResult(const std::function<void(const FaceResult&)>& callback) {
    return runtime_.subscribeTopic("FaceResult", s_FaceResult_topic_idl_hash, [callback](uint32_t, const omnibinder::Buffer& data) {
        FaceResult msg;
        omnibinder::Buffer buf(data.data(), data.size());
        if (!msg.deserialize(buf)) return;
        callback(msg);
    }, nullptr);
}

} // namespace mediaipc
