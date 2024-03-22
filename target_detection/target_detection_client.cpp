#include "target_detection_client.h"
#include <grpc++/grpc++.h>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "target_detection.grpc.pb.h"
#include "target_detection.pb.h"

struct TargetDetectionClient::Impl {
    std::mutex stubMutex;
    std::mutex labelsMutex;
    targetDetection::Communicate::Stub* stub = nullptr;
    int64_t taskId = 0;
    std::vector<std::string> labels;
    std::atomic<bool> shouldStop{false};
};

TargetDetectionClient::TargetDetectionClient(): pImpl(new Impl()) {

}

TargetDetectionClient::~TargetDetectionClient() {
    pImpl->shouldStop.store(true);
    std::lock(pImpl->stubMutex, pImpl->labelsMutex); // 同时锁定两个互斥锁
    std::lock_guard<std::mutex> lk1(pImpl->stubMutex, std::adopt_lock);
    std::lock_guard<std::mutex> lk2(pImpl->labelsMutex, std::adopt_lock);
    if (pImpl->stub) {
        delete pImpl->stub;
        pImpl->stub = nullptr;
    }
}

bool TargetDetectionClient::setAddress(std::string ip, int port) {
    if (pImpl->shouldStop.load()) return false;
    // TODO 重置时未考虑线程安全
    std::shared_ptr<grpc::ChannelInterface> channel = grpc::CreateChannel(ip + ":" + std::to_string(port), grpc::InsecureChannelCredentials());
    std::unique_ptr<targetDetection::Communicate::Stub> stubTmp = targetDetection::Communicate::NewStub(channel);
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

bool TargetDetectionClient::setTaskId(int64_t taskId) {
    if (pImpl->shouldStop.load()) return false;
    pImpl->taskId = taskId;
    return true;
}

bool TargetDetectionClient::getMappingTable() {
    if (pImpl->shouldStop.load()) return false;
    if (nullptr == pImpl->stub) {
        return false;
    }
    targetDetection::GetResultMappingTableRequest getResultMappingTableRequest;
    targetDetection::GetResultMappingTableResponse getResultMappingTableResponse;
    grpc::ClientContext context;

    getResultMappingTableRequest.set_taskid(pImpl->taskId);
    grpc::Status status = pImpl->stub->getResultMappingTable(&context, getResultMappingTableRequest, &getResultMappingTableResponse);
    targetDetection::CustomResponse response = getResultMappingTableResponse.response();
    int32_t code = response.code();
    if (200 != code) {
        auto message = response.message();
        // TODO 以后改成日志
        std::cout << message << std::endl;
        return false;
    }
    int labelsCnt = getResultMappingTableResponse.labels_size();
    pImpl->labels.resize(labelsCnt);
    for (int i = 0; i < labelsCnt; ++i) {
        pImpl->labels[i] = getResultMappingTableResponse.labels(i);
    }
    return true;
}

bool TargetDetectionClient::getResultByImageId(int64_t imageId, std::vector<TargetDetectionClient::Result>& results) {
    if (pImpl->shouldStop.load()) return false;
    std::lock_guard<std::mutex> lock(pImpl->labelsMutex);
    if (nullptr == pImpl->stub) {
        return false;
    }
    if (pImpl->labels.empty()) {
        std::cout << "labels is empty" << std::endl;
        return false;
    }
    targetDetection::GetResultIndexByImageIdRequest getResultIndexByImageIdRequest;
    targetDetection::GetResultIndexByImageIdResponse getResultIndexByImageIdResponse;
    grpc::ClientContext context;

    getResultIndexByImageIdRequest.set_taskid(pImpl->taskId);
    getResultIndexByImageIdRequest.set_imageid(imageId);
    getResultIndexByImageIdRequest.set_wait(true);
    grpc::Status status = pImpl->stub->getResultIndexByImageId(&context, getResultIndexByImageIdRequest, &getResultIndexByImageIdResponse);
    targetDetection::CustomResponse response = getResultIndexByImageIdResponse.response();
    int32_t code = response.code();
    if (200 != code) {
        auto message = response.message();
        // TODO 以后改成日志
        std::cout << message << std::endl;
        return false;
    }
    int resultsCnt = getResultIndexByImageIdResponse.results_size();
    results.resize(resultsCnt);
    for (int i = 0; i < resultsCnt; ++i) {
        auto result = getResultIndexByImageIdResponse.results(i);
        int c = result.labelid();
        if (c >= pImpl->labels.size()) {
            std::cout << "Invalid label ID: " << c << std::endl;
            results[i].label = std::to_string(c);
        }
        else {
            results[i].label = pImpl->labels[c];
        }
        results[i].confidence = result.confidence();
        results[i].x1 = result.x1();
        results[i].y1 = result.y1();
        results[i].x2 = result.x2();
        results[i].y2 = result.y2();
    }
    return true;
}

bool TargetDetectionClient::loadModel(int64_t taskId) {
    if (pImpl->shouldStop.load()) return false;
    if (nullptr == pImpl->stub) {
        return false;
    }
    targetDetection::LoadModelRequest loadModelRequest;
    targetDetection::LoadModelResponse loadModelResponse;
    grpc::ClientContext context;

    loadModelRequest.set_taskid(taskId);
    grpc::Status status = pImpl->stub->loadModel(&context, loadModelRequest, &loadModelResponse);
    targetDetection::CustomResponse response = loadModelResponse.response();
    int32_t code = response.code();
    if (200 != code) {
        auto message = response.message();
        // TODO 以后改成日志
        std::cout << message << std::endl;
        return false;
    }
    return true;
}

// bool TargetDetectionClient::checkModelState(int64_t taskId, targetDetection::ModelState& modelState) {
//     if (pImpl->shouldStop.load()) return false;
//     if (nullptr == pImpl->stub) {
//         return false;
//     }
//     targetDetection::CheckModelStateRequest checkModelStateRequest;
//     targetDetection::CheckModelStateResponse checkModelStateResponse;
//     grpc::ClientContext context;

//     checkModelStateRequest.set_taskid(taskId);
//     grpc::Status status = pImpl->stub->checkModelState(&context, checkModelStateRequest, &checkModelStateResponse);
//     targetDetection::CustomResponse response = checkModelStateResponse.response();
//     int32_t code = response.code();
//     if (200 != code) {
//         auto message = response.message();
//         // TODO 以后改成日志
//         std::cout << message << std::endl;
//         return false;
//     }
//     modelState = checkModelStateResponse.modelstate();
//     return true;
// }