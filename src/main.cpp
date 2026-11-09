
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
            operationLabel_, windowName_,
            nullptr, 5, onTrackbar, this);

        cv::createTrackbar(
            radiusLabel_, windowName_,
            nullptr, 20, onTrackbar, this);

        cv::createTrackbar(
            shapeLabel_, windowName_,
            nullptr, 2, onTrackbar, this);

        cv::createTrackbar(
            viewLabel_, windowName_,
            nullptr, 1, onTrackbar, this);

        cv::createTrackbar(
            thresholdLabel_, windowName_,
            nullptr, 255, onTrackbar, this);

        cv::createTrackbar(
            thresholdTypeLabel_, windowName_,
            nullptr, 1, onTrackbar, this);

        cv::createTrackbar(
            minAreaLabel_, windowName_,
            nullptr, 500, onTrackbar, this);

        cv::createTrackbar(
            displayLabel_, windowName_,
            nullptr, 1, onTrackbar, this);

        // Initialize trackbar positions
        cv::setTrackbarPos(operationLabel_, windowName_, operation_);
        cv::setTrackbarPos(radiusLabel_, windowName_, radius_);
        cv::setTrackbarPos(shapeLabel_, windowName_, shape_);
        cv::setTrackbarPos(viewLabel_, windowName_, viewMode_);
        cv::setTrackbarPos(thresholdLabel_, windowName_, thresholdValue_);
        cv::setTrackbarPos(thresholdTypeLabel_, windowName_, thresholdType_);
        cv::setTrackbarPos(minAreaLabel_, windowName_, minArea_);
        cv::setTrackbarPos(displayLabel_, windowName_, overlayMode_);

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

    static constexpr const char *operationLabel_ =
        "Operation: 0=Erosion 1=Dilation 2=Opening 3=Closing 4=Top-hat 5=Black-hat";

    static constexpr const char *radiusLabel_ = "Kernel Radius";

    static constexpr const char *shapeLabel_ =
        "Shape: 0=Rectangle 1=Cross 2=Ellipse";

    static constexpr const char *viewLabel_ =
        "View: 0=Process 1=Detect";

    static constexpr const char *thresholdLabel_ = "Threshold";

    static constexpr const char *thresholdTypeLabel_ =
        "Threshold Type: 0=Binary 1=Inverse";

    static constexpr const char *minAreaLabel_ = "Minimum Area";

    static constexpr const char *displayLabel_ =
        "Display: 0=Boxes 1=Overlay";

    cv::Mat original_;
    cv::Mat processed_;
    cv::Mat preview_;
    cv::Mat binary_;
    cv::Mat detectionOutput_;

    int viewMode_ = 0; // 0 Processing, 1 Detection
    int thresholdValue_ = 25;
    int thresholdType_ = 0; // 0 Binary, 1 Binary Inverse
    int minArea_ = 10;
    int overlayMode_ = 0;

    int operation_ = 5;
    int radius_ = 5;
    int shape_ = 2;

    static void onTrackbar(int, void *userdata)
    {
        auto *self = static_cast<MorphologyWorkbench *>(userdata);
        self->update();
    }

    void update()
    {

        operation_ = cv::getTrackbarPos(operationLabel_, windowName_);
        radius_ = cv::getTrackbarPos(radiusLabel_, windowName_);
        shape_ = cv::getTrackbarPos(shapeLabel_, windowName_);

        viewMode_ = cv::getTrackbarPos(viewLabel_, windowName_);
        thresholdValue_ = cv::getTrackbarPos(thresholdLabel_, windowName_);
        thresholdType_ = cv::getTrackbarPos(thresholdTypeLabel_, windowName_);
        minArea_ = cv::getTrackbarPos(minAreaLabel_, windowName_);
        overlayMode_ = cv::getTrackbarPos(displayLabel_, windowName_);

        processed_ = applyMorphology(
            original_,
            operation_,
            radius_,
            shape_);

        preview_ = processed_.clone();

        if (operation_ == 4 || operation_ == 5)
        {
            cv::normalize(
                processed_, preview_,
                0, 255, cv::NORM_MINMAX);
        }

        cv::Mat comparison;

        if (viewMode_ == 0)
        {
            cv::hconcat(original_, preview_, comparison);
        }

        else
        {
            DetectionResult current_result = detectDefects(
                original_,
                processed_,
                thresholdValue_,
                thresholdType_,
                minArea_);

            // Store detection results
            binary_ = current_result.mask;
            if (overlayMode_ == 0)
            {
                detectionOutput_ = current_result.visualization;
            }
            else
            {
                detectionOutput_ = createOverlay(
                    original_,
                    current_result.filteredMask);
            }

            // Draw candidate count
            std::string candidate_str =
                "Candidates: " +
                std::to_string(current_result.candidateCount);

            cv::putText(
                detectionOutput_,
                candidate_str,
                cv::Point(20, 40),
                cv::FONT_HERSHEY_SIMPLEX,
                0.8,
                cv::Scalar(0, 255, 0),
                2);

            // Convert original image to BGR for comparison
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
            "_kernel_radius_" + std::to_string(radius_) +
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

            const std::string displaySuffix =
                overlayMode_ == 0 ? "_boxes.png" : "_overlay.png";

            cv::imwrite(
                prefix + displaySuffix,
                detectionOutput_);
        }
    }

    cv::Mat createOverlay(
        const cv::Mat &original,
        const cv::Mat &mask)
    {
        cv::Mat base;
        cv::cvtColor(original, base, cv::COLOR_GRAY2BGR);
        cv::Mat redLayer(
            base.size(),
            base.type(),
            cv::Scalar(0, 0, 255));
        cv::Mat blended;

        cv::addWeighted(
            base,
            0.6,
            redLayer,
            0.4,
            0,
            blended);
        blended.copyTo(base, mask);
        return base;
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
