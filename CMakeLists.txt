cmake_minimum_required(VERSION 3.10)

# 获取当前目录下的所有子目录
file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(child ${children})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
        # 检查每个子目录中是否存在CMakeLists.txt文件
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${child}/CMakeLists.txt")
            # 如果存在，则添加为子目录
            add_subdirectory(${child})
        endif()
    endif()
endforeach()
