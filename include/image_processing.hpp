#pragma once

#include <opencv2/core.hpp>

cv::Mat applyMorphology(
    const cv::Mat& input,
    int operation,
    int radius,
    int shape
);