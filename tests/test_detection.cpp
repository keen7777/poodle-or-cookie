
#include "image_processing.hpp"

#include <opencv2/imgproc.hpp>
#include <iostream>
#include <string>
#include <vector>

struct TestCase
{
    std::string name;
    int minArea;
    int expectedCandidates;
    int expectedMaskPixels;
};

cv::Mat createTestImage()
{
    cv::Mat image = cv::Mat::zeros(100, 100, CV_8UC1);

    // Small component: 100 pixels
    cv::rectangle(
        image, cv::Rect(10, 10, 5, 20),
        cv::Scalar(255), cv::FILLED);

    // Large component: 150 pixels
    cv::rectangle(
        image, cv::Rect(40, 40, 15, 10),
        cv::Scalar(255), cv::FILLED);

    return image;
}

int main()
{
    const cv::Mat image = createTestImage();

    const std::vector<TestCase> testCases = {
        {"Normal filtering", 101, 1, 150},
        {"Boundary condition", 100, 2, 250},
        {"Filter all", 151, 0, 0}
    };

    int failures = 0;

    for (const auto& test : testCases)
    {
        DetectionResult result = detectDefects(
            image, image, 127, 0, test.minArea);

        const int actualPixels =
            cv::countNonZero(result.filteredMask);

        const bool passed =
            result.candidateCount == test.expectedCandidates &&
            actualPixels == test.expectedMaskPixels;

        if (passed)
        {
            std::cout << "[PASS] " << test.name << '\n';
        }
        else
        {
            ++failures;

            std::cerr
                << "[FAIL] " << test.name << '\n'
                << "  Candidates: expected "
                << test.expectedCandidates
                << ", got " << result.candidateCount << '\n'
                << "  Mask pixels: expected "
                << test.expectedMaskPixels
                << ", got " << actualPixels << '\n';
        }
    }

    return failures == 0 ? 0 : 1;
}
