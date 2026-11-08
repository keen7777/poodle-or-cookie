
#include <opencv2/opencv.hpp>

#include <filesystem>
#include <iostream>
#include <string>

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
    cv::Mat detectionOutput_;

    int thresholdValue_ = 25;
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
        // Extract local dark structures
        cv::Mat blackhat;
        cv::Mat kernel = cv::getStructuringElement(
            cv::MORPH_ELLIPSE,
            cv::Size(2 * radius_ + 1, 2 * radius_ + 1));

        cv::morphologyEx(
            original_,
            blackhat,
            cv::MORPH_BLACKHAT,
            kernel);

        // Threshold the response
        cv::Mat binary;
        cv::threshold(
            blackhat,
            binary,
            thresholdValue_,
            255,
            cv::THRESH_BINARY);

        // Connected components
        cv::Mat labels, stats, centroids;

        int count = cv::connectedComponentsWithStats(
            binary, labels, stats, centroids, 8, CV_32S);

        cv::cvtColor(
            original_,
            detectionOutput_,
            cv::COLOR_GRAY2BGR);

        for (int label = 1; label < count; ++label)
        {
            int area = stats.at<int>(
                label, cv::CC_STAT_AREA);

            if (area < minArea_)
                continue;

            int x = stats.at<int>(label, cv::CC_STAT_LEFT);
            int y = stats.at<int>(label, cv::CC_STAT_TOP);
            int w = stats.at<int>(label, cv::CC_STAT_WIDTH);
            int h = stats.at<int>(label, cv::CC_STAT_HEIGHT);

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

        const int kernelSize = 2 * radius_ + 1;

        const int shapes[] = {
            cv::MORPH_RECT,
            cv::MORPH_CROSS,
            cv::MORPH_ELLIPSE};

        cv::Mat kernel = cv::getStructuringElement(
            shapes[shape_],
            cv::Size(kernelSize, kernelSize));

        if (operation_ == 0)
        {
            cv::erode(original_, processed_, kernel);
        }
        else if (operation_ == 1)
        {
            cv::dilate(original_, processed_, kernel);
        }
        else
        {
            const int operations[] = {
                cv::MORPH_OPEN,
                cv::MORPH_CLOSE,
                cv::MORPH_TOPHAT,
                cv::MORPH_BLACKHAT};

            cv::morphologyEx(
                original_,
                processed_,
                operations[operation_ - 2],
                kernel);
        }

        preview_ = processed_.clone();

        if (operation_ == 4 || operation_ == 5)
        {
            cv::normalize(
                processed_, preview_,
                0, 255, cv::NORM_MINMAX);
        }

        cv::Mat comparison;
        cv::hconcat(original_, preview_, comparison);

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
