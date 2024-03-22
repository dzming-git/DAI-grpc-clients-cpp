/*****************************************************************************
*  Copyright © 2023 - 2023 dzming.                                           *
*                                                                            *
*  @file     target_detection_client.h                                       *
*  @brief                                                                    *
*  @author   dzming                                                          *
*  @email    dzm_work@163.com                                                *
*                                                                            *
*----------------------------------------------------------------------------*
*  Remark  :                                                                 *
*****************************************************************************/

#ifndef _TARGET_DETECTION_CLIENT_H_
#define _TARGET_DETECTION_CLIENT_H_

#include <string>
#include <memory>
#include <vector>

class TargetDetectionClient {
public:
    TargetDetectionClient();
    ~TargetDetectionClient();
    struct Result {
        std::string label;
        double confidence;
        double x1;
        double y1;
        double x2;
        double y2;
    };

    bool setAddress(std::string ip, int port);
    bool setTaskId(int64_t taskId);
    bool getMappingTable();
    bool getResultByImageId(int64_t imageId, std::vector<TargetDetectionClient::Result>& results);
    bool loadModel(int64_t taskId);
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};



#endif /* _TARGET_DETECTION_CLIENT_H_ */