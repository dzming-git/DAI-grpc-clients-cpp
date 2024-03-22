#include "target_tracking_client.h"
#include <grpc++/grpc++.h>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "target_tracking.grpc.pb.h"
#include "target_tracking.pb.h"

struct TargetTrackingClient::Impl {
    std::mutex stubMutex;
    targetTracking::Communicate::Stub* stub = nullptr;
    int64_t taskId = 0;
    std::atomic<bool> shouldStop{false};
};

TargetTrackingClient::TargetTrackingClient(): pImpl(new Impl()) {

}

TargetTrackingClient::~TargetTrackingClient() {
    pImpl->shouldStop.store(true);
    std::lock_guard<std::mutex> lock(pImpl->stubMutex);
    if (pImpl->stub) {
        delete pImpl->stub;
        pImpl->stub = nullptr;
    }
}

bool TargetTrackingClient::setAddress(std::string ip, int port) {
    if (pImpl->shouldStop.load()) return false;
    // TODO 重置时未考虑线程安全
    std::shared_ptr<grpc::ChannelInterface> channel = grpc::CreateChannel(ip + ":" + std::to_string(port), grpc::InsecureChannelCredentials());
    std::unique_ptr<targetTracking::Communicate::Stub> stubTmp = targetTracking::Communicate::NewStub(channel);
    // 重置
    if (pImpl->stub) {
        delete pImpl->stub;
        pImpl->stub = nullptr;
    }
    // unique_ptr 转为 普通指针
    pImpl->stub = stubTmp.get();
    stubTmp.release();
    return true;
}

bool TargetTrackingClient::setTaskId(int64_t taskId) {
    if (pImpl->shouldStop.load()) return false;
    pImpl->taskId = taskId;
    return true;
}

bool TargetTrackingClient::getResultByImageId(int64_t imageId, std::vector<TargetTrackingClient::Result>& results) {
    if (pImpl->shouldStop.load()) return false;
    if (nullptr == pImpl->stub) {
        return false;
    }
    targetTracking::GetResultByImageIdRequest getResultByImageIdRequest;
    targetTracking::GetResultByImageIdResponse getResultByImageIdResponse;
    grpc::ClientContext context;

    getResultByImageIdRequest.set_taskid(pImpl->taskId);
    getResultByImageIdRequest.set_imageid(imageId);
    getResultByImageIdRequest.set_wait(true);
    getResultByImageIdRequest.set_onlythelatest(false);
    grpc::Status status = pImpl->stub->getResultByImageId(&context, getResultByImageIdRequest, &getResultByImageIdResponse);
    targetTracking::CustomResponse response = getResultByImageIdResponse.response();
    int32_t code = response.code();
    if (200 != code) {
        auto message = response.message();
        // TODO 以后改成日志
        std::cout << message << std::endl;
        return false;
    }
    int resultsCnt = getResultByImageIdResponse.results_size();
    results.resize(resultsCnt);
    for (int i = 0; i < resultsCnt; ++i) {
        auto result = getResultByImageIdResponse.results(i);
        int id = result.id();
        results[i].id = id;
        auto bboxs = result.bboxs();
        int bboxsCnt = result.bboxs_size();
        results[i].bboxs.resize(bboxsCnt);
        for (int j = 0; j < bboxsCnt; ++j) {
            results[i].bboxs[j].x1 = bboxs[j].x1();
            results[i].bboxs[j].y1 = bboxs[j].y1();
            results[i].bboxs[j].x2 = bboxs[j].x2();
            results[i].bboxs[j].y2 = bboxs[j].y2();
        }
    }
    return true;
}
