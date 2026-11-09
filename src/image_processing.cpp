
#include "image_processing.hpp"
#include <opencv2/imgproc.hpp>

cv::Mat applyMorphology(
    const cv::Mat &input,
    int operation,
    int radius,
    int shape)
{
    cv::Mat output;

    const int kernelSize = 2 * radius + 1;

    const int shapes[] = {
        cv::MORPH_RECT,
        cv::MORPH_CROSS,
        cv::MORPH_ELLIPSE};

    cv::Mat kernel = cv::getStructuringElement(
        shapes[shape],
        cv::Size(kernelSize, kernelSize));

    if (operation == 0)
    {
        cv::erode(input, output, kernel);
    }
    else if (operation == 1)
    {
        cv::dilate(input, output, kernel);
    }
    else
    {
        const int operations[] = {
            cv::MORPH_OPEN,
            cv::MORPH_CLOSE,
            cv::MORPH_TOPHAT,
            cv::MORPH_BLACKHAT};

        cv::morphologyEx(
            input,
            output,
            operations[operation - 2],
            kernel);
    }

    return output;
}

DetectionResult detectDefects(
    const cv::Mat &original,
    const cv::Mat &processed,
    int thresholdValue,
    int thresholdType,
    int minArea)
{
    const int type = thresholdType == 0
                         ? cv::THRESH_BINARY
                         : cv::THRESH_BINARY_INV;

    // Threshold the raw morphological response,
    // not the normalized preview.
    DetectionResult temp_result;
    cv::threshold(
        processed,
        temp_result.mask,
        thresholdValue,
        255,
        type);

    cv::Mat labels, stats, centroids;

    int count = cv::connectedComponentsWithStats(
        temp_result.mask, labels, stats, centroids, 8, CV_32S);

    temp_result.filteredMask = cv::Mat::zeros(
        temp_result.mask.size(),
        CV_8UC1);

    cv::cvtColor(
        original,
        temp_result.visualization,
        cv::COLOR_GRAY2BGR);

    for (int label = 1; label < count; ++label)
    {
        const int area =
            stats.at<int>(label, cv::CC_STAT_AREA);

        if (area < minArea)
            continue;
            
        temp_result.filteredMask.setTo(
            255,
            labels == label);

        const int x =
            stats.at<int>(label, cv::CC_STAT_LEFT);
        const int y =
            stats.at<int>(label, cv::CC_STAT_TOP);
        const int w =
            stats.at<int>(label, cv::CC_STAT_WIDTH);
        const int h =
            stats.at<int>(label, cv::CC_STAT_HEIGHT);

        cv::rectangle(
            temp_result.visualization,
            cv::Rect(x, y, w, h),
            cv::Scalar(0, 0, 255),
            2);
        ++temp_result.candidateCount;
    }
    return temp_result;
}