/*****************************************************************************
*  Copyright © 2023 - 2023 dzming.                                           *
*                                                                            *
*  @file     image_harmony_client.h                                          *
*  @brief                                                                    *
*  @author   dzming                                                          *
*  @email    dzm_work@163.com                                                *
*                                                                            *
*----------------------------------------------------------------------------*
*  Remark  :                                                                 *
*****************************************************************************/

#ifndef _IMAGE_HARMONY_CLIENT_H_
#define _IMAGE_HARMONY_CLIENT_H_

#include <string>
#include <memory>
#include <opencv2/opencv.hpp>

class ImageHarmonyClient {
public:
    ImageHarmonyClient();
    ~ImageHarmonyClient();
    struct ImageInfo {
        int64_t imageId = 0;
        int width = 0;
        int height = 0;
        std::string format = ".jpg";
        int quality = 90;
    };

    bool setAddress(std::string ip, int port);
    bool connectImageLoader(int64_t loaderArgsHash);
    bool disconnectImageLoader();
    bool getImageByImageId(ImageHarmonyClient::ImageInfo imageInfo, int64_t& imageIdOutput, cv::Mat& imageOutput);
    bool getImageSize(ImageHarmonyClient::ImageInfo imageInfo, int64_t &imageIdOutput, int& width, int& height);
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif /* _IMAGE_HARMONY_CLIENT_H_ */
