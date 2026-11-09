#pragma once

#include <opencv2/core.hpp>

cv::Mat applyMorphology(
    const cv::Mat& input,
    int operation,
    int radius,
    int shape
);

struct DetectionResult {
    cv::Mat mask;
    cv::Mat visualization;
    int candidateCount = 0;
};

DetectionResult detectDefects(
    const cv::Mat& original,
    const cv::Mat& processed,
    int thresholdValue,
    int thresholdType,
    int minArea
);