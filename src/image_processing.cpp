
#include <opencv2/opencv.hpp>

cv::Mat applyMorphology(
    const cv::Mat& input,
    int operation,
    int radius,
    int shape)
{
    cv::Mat output;

    const int kernelSize = 2 * radius + 1;

    const int shapes[] = {
        cv::MORPH_RECT,
        cv::MORPH_CROSS,
        cv::MORPH_ELLIPSE
    };

    cv::Mat kernel = cv::getStructuringElement(
        shapes[shape],
        cv::Size(kernelSize, kernelSize)
    );

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
            cv::MORPH_BLACKHAT
        };

        cv::morphologyEx(
            input,
            output,
            operations[operation - 2],
            kernel
        );
    }

    return output;
}
