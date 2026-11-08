
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <string>

#include "image_processing.hpp"

class MorphologyWorkbench
{
public:
    explicit MorphologyWorkbench(const cv::Mat &image)
        : original_(image.clone()) {}

    void run()
    {
        cv::namedWindow(windowName_, cv::WINDOW_NORMAL);
        cv::resizeWindow(windowName_, 1200, 700);

        cv::createTrackbar(
            "Operation 0-5", windowName_,
            nullptr, 5, onTrackbar, this);

        cv::createTrackbar(
            "Kernel radius", windowName_,
            nullptr, 20, onTrackbar, this);

        cv::createTrackbar(
            "Shape 0-2", windowName_,
            nullptr, 2, onTrackbar, this);
        cv::createTrackbar(
            "View 0=Process 1=Detect",
            windowName_, nullptr, 1, onTrackbar, this);

        cv::createTrackbar(
            "Threshold",
            windowName_, nullptr, 255, onTrackbar, this);

        cv::createTrackbar(
            "Threshold type 0=Bin 1=Inv",
            windowName_, nullptr, 1, onTrackbar, this);

        cv::createTrackbar(
            "Minimum area",
            windowName_, nullptr, 500, onTrackbar, this);

        cv::setTrackbarPos("View 0=Process 1=Detect",
                           windowName_, viewMode_);
        cv::setTrackbarPos("Threshold",
                           windowName_, thresholdValue_);
        cv::setTrackbarPos("Threshold type 0=Bin 1=Inv",
                           windowName_, thresholdType_);
        cv::setTrackbarPos("Minimum area",
                           windowName_, minArea_);

        cv::setTrackbarPos("Operation 0-5", windowName_, operation_);
        cv::setTrackbarPos("Kernel radius", windowName_, radius_);
        cv::setTrackbarPos("Shape 0-2", windowName_, shape_);

        update();

        std::cout
            << "Operations:\n"
            << "  0 Erosion\n"
            << "  1 Dilation\n"
            << "  2 Opening\n"
            << "  3 Closing\n"
            << "  4 Top-hat\n"
            << "  5 Black-hat\n"
            << "Shapes: 0 Rectangle, 1 Cross, 2 Ellipse\n"
            << "Press S to save, Q or ESC to quit.\n";

        while (true)
        {
            int key = cv::waitKey(30);

            if (key == 'q' || key == 'Q' || key == 27)
                break;

            if (key == 's' || key == 'S')
                save();
        }

        cv::destroyAllWindows();
    }

private:
    static constexpr const char *windowName_ = "Poodle-or-Cookie";

    cv::Mat original_;
    cv::Mat processed_;
    cv::Mat preview_;
    cv::Mat binary_;
    cv::Mat detectionOutput_;

    int viewMode_ = 0; // 0 Processing, 1 Detection
    int thresholdValue_ = 25;
    int thresholdType_ = 0; // 0 Binary, 1 Binary Inverse
    int minArea_ = 10;

    int operation_ = 5;
    int radius_ = 5;
    int shape_ = 2;

    static void onTrackbar(int, void *userdata)
    {
        auto *self = static_cast<MorphologyWorkbench *>(userdata);
        self->update();
    }

    void detectDefects()
    {
        const int type = thresholdType_ == 0
                             ? cv::THRESH_BINARY
                             : cv::THRESH_BINARY_INV;

        // Threshold the raw morphological response,
        // not the normalized preview.
        cv::threshold(
            processed_,
            binary_,
            thresholdValue_,
            255,
            type);

        cv::Mat labels, stats, centroids;

        int count = cv::connectedComponentsWithStats(
            binary_, labels, stats, centroids, 8, CV_32S);

        cv::cvtColor(
            original_,
            detectionOutput_,
            cv::COLOR_GRAY2BGR);

        for (int label = 1; label < count; ++label)
        {
            const int area =
                stats.at<int>(label, cv::CC_STAT_AREA);

            if (area < minArea_)
                continue;

            const int x =
                stats.at<int>(label, cv::CC_STAT_LEFT);
            const int y =
                stats.at<int>(label, cv::CC_STAT_TOP);
            const int w =
                stats.at<int>(label, cv::CC_STAT_WIDTH);
            const int h =
                stats.at<int>(label, cv::CC_STAT_HEIGHT);

            cv::rectangle(
                detectionOutput_,
                cv::Rect(x, y, w, h),
                cv::Scalar(0, 0, 255),
                2);
        }
    }

    void update()
    {
        operation_ = cv::getTrackbarPos("Operation 0-5", windowName_);
        radius_ = cv::getTrackbarPos("Kernel radius", windowName_);
        shape_ = cv::getTrackbarPos("Shape 0-2", windowName_);

        processed_ = applyMorphology(
        original_,
        operation_,
        radius_,
        shape_
    );

        preview_ = processed_.clone();

        if (operation_ == 4 || operation_ == 5)
        {
            cv::normalize(
                processed_, preview_,
                0, 255, cv::NORM_MINMAX);
        }

        viewMode_ = cv::getTrackbarPos(
            "View 0=Process 1=Detect", windowName_);

        thresholdValue_ = cv::getTrackbarPos(
            "Threshold", windowName_);

        thresholdType_ = cv::getTrackbarPos(
            "Threshold type 0=Bin 1=Inv", windowName_);

        minArea_ = cv::getTrackbarPos(
            "Minimum area", windowName_);

        cv::Mat comparison;

        if (viewMode_ == 0)
        {
            cv::hconcat(original_, preview_, comparison);
        }
        else
        {
            detectDefects();

            cv::Mat originalColor;
            cv::cvtColor(
                original_,
                originalColor,
                cv::COLOR_GRAY2BGR);

            cv::hconcat(
                originalColor,
                detectionOutput_,
                comparison);
        }

        cv::imshow(windowName_, comparison);
    }

    void save()
    {
        std::filesystem::create_directories("results");

        const std::string prefix =
            "results/op_" + std::to_string(operation_) +
            "_kernel_" + std::to_string(2 * radius_ + 1) +
            "_shape_" + std::to_string(shape_);

        const bool savedRaw =
            cv::imwrite(prefix + "_raw.png", processed_);

        const bool savedPreview =
            cv::imwrite(prefix + "_preview.png", preview_);

        if (savedRaw && savedPreview)
            std::cout << "Saved: " << prefix << "\n";
        else
            std::cerr << "Failed to save results.\n";

        if (viewMode_ == 1)
        {
            cv::imwrite(
                prefix + "_mask.png",
                binary_);

            cv::imwrite(
                prefix + "_detections.png",
                detectionOutput_);
        }
    }
};

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: defect_detector <image_path>\n";
        return 1;
    }

    cv::Mat image = cv::imread(
        argv[1], cv::IMREAD_GRAYSCALE);

    if (image.empty())
    {
        std::cerr << "Failed to load image.\n";
        return 1;
    }

    MorphologyWorkbench workbench(image);
    workbench.run();

    return 0;
}
