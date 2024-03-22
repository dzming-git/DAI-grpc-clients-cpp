#include "image_harmony_client.h"
#include <grpc++/grpc++.h>
#include "image_harmony.grpc.pb.h"
#include "image_harmony.pb.h"
#include <mutex>

struct ImageHarmonyClient::Impl {
    std::mutex stubMutex;
    imageHarmony::Communicate::Stub* stub = nullptr;
    int64_t connectionId = 0;
    std::atomic<bool> shouldStop{false};
};

ImageHarmonyClient::ImageHarmonyClient(): pImpl(new Impl()) {

}

ImageHarmonyClient::~ImageHarmonyClient() {
    pImpl->shouldStop.store(true);
    std::lock_guard<std::mutex> lock(pImpl->stubMutex);
    if (pImpl->stub) {
        delete pImpl->stub;
        pImpl->stub = nullptr;
    }
}

bool ImageHarmonyClient::setAddress(std::string ip, int port) {
    if (pImpl->shouldStop.load()) return false;
    // TODO 重置时未考虑线程安全
    std::shared_ptr<grpc::ChannelInterface> channel = grpc::CreateChannel(ip + ":" + std::to_string(port), grpc::InsecureChannelCredentials());
    std::unique_ptr<imageHarmony::Communicate::Stub> stubTmp = imageHarmony::Communicate::NewStub(channel);
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

bool ImageHarmonyClient::connectImageLoader(int64_t loaderArgsHash) {
    if (pImpl->shouldStop.load()) return false;
    if (nullptr == pImpl->stub) {
        return false;
    }
    imageHarmony::ConnectImageLoaderRequest request;
    imageHarmony::ConnectImageLoaderResponse response;
    grpc::ClientContext context;
    request.set_loaderargshash(loaderArgsHash);
    grpc::Status status = pImpl->stub->connectImageLoader(&context, request, &response);
    imageHarmony::CustomResponse customresponse = response.response();
    int32_t code = customresponse.code();
    if (200 != code) {
        auto message = customresponse.message();
        // TODO 以后改成日志
        std::cout << message << std::endl;
        return false;
    }
    pImpl->connectionId = response.connectionid();
    if (0 == pImpl->connectionId) {
        return false;
    }
    return true;
}

bool ImageHarmonyClient::disconnectImageLoader() {
    if (pImpl->shouldStop.load()) return false;
    if (0 == pImpl->connectionId) {
        return true;
    }
    if (nullptr == pImpl->stub) {
        return false;
    }
    imageHarmony::DisconnectImageLoaderRequest request;
    imageHarmony::DisconnectImageLoaderResponse response;
    grpc::ClientContext context;
    request.set_connectionid(pImpl->connectionId);
    grpc::Status status = pImpl->stub->disconnectImageLoader(&context, request, &response);
    imageHarmony::CustomResponse customResponse = response.response();
    int32_t code = customResponse.code();
    if (200 != code) {
        auto message = customResponse.message();
        // TODO 以后改成日志
        std::cout << message << std::endl;
        return false;
    }
    return true;
}

bool ImageHarmonyClient::getImageByImageId(ImageHarmonyClient::ImageInfo imageInfo, int64_t& imageIdOutput, cv::Mat& imageOutput) {
    if (pImpl->shouldStop.load()) return false;
    if (nullptr == pImpl->stub) {
        return false;
    }
    imageHarmony::GetImageByImageIdRequest request;
    imageHarmony::GetImageByImageIdResponse response;
    grpc::ClientContext context;

    request.set_connectionid(pImpl->connectionId);
    request.mutable_imagerequest()->set_imageid(imageInfo.imageId);
    request.mutable_imagerequest()->set_format(imageInfo.format);
    request.mutable_imagerequest()->mutable_params()->Add(cv::IMWRITE_JPEG_QUALITY);
    request.mutable_imagerequest()->mutable_params()->Add(imageInfo.quality);
    request.mutable_imagerequest()->set_expectedw(imageInfo.width);
    request.mutable_imagerequest()->set_expectedh(imageInfo.height);
    grpc::Status status = pImpl->stub->getImageByImageId(&context, request, &response);
    
    if (!status.ok()) {
        std::cout << "Error: " << status.error_code() << ": " << status.error_message() << std::endl;
        return false;
    }
    
    auto customResponse = response.response();
    if (200 != customResponse.code()) {
        std::cout << customResponse.message() << std::endl;
        return false;
    }
    
    imageIdOutput = response.imageresponse().imageid();

    if (!imageIdOutput) {
        std::cout << "image ID is 0" << std::endl;
        return false;
    }

    std::string buffer = response.imageresponse().buffer();
    std::vector<uint8_t> vecBuffer(buffer.begin(), buffer.end());
    cv::Mat image = cv::imdecode(vecBuffer, cv::IMREAD_COLOR);
    
    imageOutput = image.clone();
    
    return true;
}

bool ImageHarmonyClient::getImageSize(ImageHarmonyClient::ImageInfo imageInfo, int64_t &imageIdOutput, int& width, int& height) {
    if (pImpl->shouldStop.load()) return false;
    if (nullptr == pImpl->stub) {
        return false;
    }
    imageHarmony::GetImageByImageIdRequest request;
    imageHarmony::GetImageByImageIdResponse response;
    grpc::ClientContext context;

    request.set_connectionid(pImpl->connectionId);
    request.mutable_imagerequest()->set_imageid(imageInfo.imageId);
    request.mutable_imagerequest()->set_noimagebuffer(true);
    request.mutable_imagerequest()->set_expectedw(imageInfo.width);
    request.mutable_imagerequest()->set_expectedh(imageInfo.height);
    grpc::Status status = pImpl->stub->getImageByImageId(&context, request, &response);
    
    if (!status.ok()) {
        std::cout << "Error: " << status.error_code() << ": " << status.error_message() << std::endl;
        return false;
    }
    
    auto customResponse = response.response();
    if (200 != customResponse.code()) {
        std::cout << customResponse.message() << std::endl;
        return false;
    }
    
    imageIdOutput = response.imageresponse().imageid();

    if (!imageIdOutput) {
        std::cout << "image ID is 0" << std::endl;
        return false;
    }

    width = response.imageresponse().width();
    height = response.imageresponse().height();

    return true;
}
