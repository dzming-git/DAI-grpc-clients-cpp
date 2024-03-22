#include "behavior_recognition_client.h"
#include <opencv2/opencv.hpp>
#include <grpc++/grpc++.h>
#include "behavior_recognition.grpc.pb.h"
#include "behavior_recognition.pb.h"
#include <mutex>

struct BehaviorRecognitionClient::Impl {
    std::mutex stubMutex;
    behaviorRecognition::Communicate::Stub* stub = nullptr;
    int64_t taskId = 0;
    std::atomic<bool> shouldStop{false};
};

BehaviorRecognitionClient::BehaviorRecognitionClient(): pImpl(new Impl()) {

}

BehaviorRecognitionClient::~BehaviorRecognitionClient() {
    pImpl->shouldStop.store(true);
    std::lock_guard<std::mutex> lock(pImpl->stubMutex);
    if (pImpl->stub) {
        delete pImpl->stub;
        pImpl->stub = nullptr;
    }
}

bool BehaviorRecognitionClient::setAddress(std::string ip, int port) {
    if (pImpl->shouldStop.load()) return false;
    // TODO 重置时未考虑线程安全
    std::shared_ptr<grpc::ChannelInterface> channel = grpc::CreateChannel(ip + ":" + std::to_string(port), grpc::InsecureChannelCredentials());
    std::unique_ptr<behaviorRecognition::Communicate::Stub> stubTmp = behaviorRecognition::Communicate::NewStub(channel);
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

bool BehaviorRecognitionClient::setTaskId(int64_t taskId) {
    if (pImpl->shouldStop.load()) return false;
    pImpl->taskId = taskId;
    return true;
}

bool BehaviorRecognitionClient::informImageId(int64_t imageId) {
    if (pImpl->shouldStop.load()) return false;
    behaviorRecognition::InformImageIdRequest request;
    request.set_taskid(pImpl->taskId);
    request.set_imageid(imageId);

    behaviorRecognition::InformImageIdResponse response;
    grpc::ClientContext context;

    grpc::Status status = pImpl->stub->informImageId(&context, request, &response);

    if (status.ok() && response.response().code() == 200) {
        return true;
    } else {
        std::cerr << "InformImageId failed with code: " << response.response().code()
                  << ", message: " << response.response().message() << std::endl;
        return false;
    }
}

bool BehaviorRecognitionClient::getResultByImageId(int64_t imageId, std::vector<BehaviorRecognitionClient::Result>& results) {
    if (pImpl->shouldStop.load()) return false;
    behaviorRecognition::GetResultByImageIdRequest request;
    request.set_taskid(pImpl->taskId);
    request.set_imageid(imageId);

    behaviorRecognition::GetResultByImageIdResponse response;
    grpc::ClientContext context;

    grpc::Status status = pImpl->stub->getResultByImageId(&context, request, &response);

    if (status.ok() && response.response().code() == 200) {
        // 正确处理结果的代码...
        for (const auto& result_proto : response.results()) {
            Result result;
            for (const auto& label_info : result_proto.labelinfos()) {
                result.labelConfidencePairs.emplace_back(label_info.label(), label_info.confidence());
            }
            result.personId = result_proto.personid();
            result.x1 = result_proto.x1();
            result.y1 = result_proto.y1();
            result.x2 = result_proto.x2();
            result.y2 = result_proto.y2();
            results.push_back(result);
        }
        return true;
    } else {
        std::cerr << "GetResultByImageId failed with code: " << response.response().code()
                  << ", message: " << response.response().message() << std::endl;
        return false;
    }
}

bool BehaviorRecognitionClient::getLatestResult(std::vector<BehaviorRecognitionClient::Result>& results) {
    if (pImpl->shouldStop.load()) return false;
    behaviorRecognition::GetLatestResultRequest request;
    request.set_taskid(pImpl->taskId);

    behaviorRecognition::GetLatestResultResponse response;
    grpc::ClientContext context;

    grpc::Status status = pImpl->stub->getLatestResult(&context, request, &response);

    if (status.ok() && response.response().code() == 200) {
        // 正确处理结果的代码...
        for (const auto& result_proto : response.results()) {
            Result result;
            for (const auto& label_info : result_proto.labelinfos()) {
                result.labelConfidencePairs.emplace_back(label_info.label(), label_info.confidence());
            }
            result.personId = result_proto.personid();
            result.x1 = result_proto.x1();
            result.y1 = result_proto.y1();
            result.x2 = result_proto.x2();
            result.y2 = result_proto.y2();
            results.push_back(result);
        }
        return true;
    } else {
        std::cerr << "GetLatestResult failed with code: " << response.response().code()
                  << ", message: " << response.response().message() << std::endl;
        return false;
    }
}
