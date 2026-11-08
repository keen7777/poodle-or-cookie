cmake_minimum_required(VERSION 3.16)

project(poodle_or_cookie LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(defect_detector 
    src/main.cpp
    src/image_processing.cpp
    )

target_include_directories(
    defect_detector
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(
    defect_detector
    PRIVATE
    ${OpenCV_LIBS}
)