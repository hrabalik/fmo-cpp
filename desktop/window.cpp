#include "window.hpp"
#include "desktop-opencv.hpp"
#include <chrono>
#include <fmo/assert.hpp>
#include <fmo/pointset.hpp>
#include <fmo/stats.hpp>

#define TOSTR_INNER(x) #x
#define TOSTR(x) TOSTR_INNER(x)

namespace {
    const char* const windowName = TOSTR(FMO_BINARY_NAME);
    constexpr Color colorFp = {0xFF, 0x00, 0x00};
    constexpr Color colorFn = {0x00, 0x00, 0xFF};
    constexpr Color colorTp = {0x00, 0xFF, 0x00};
    cv::Vec3b toCv(Color c) { return {c.b, c.g, c.r}; }

    struct PutColor {
        cv::Vec3b color;
        cv::Mat mat;

        void operator()(fmo::Pos pt) { mat.at<cv::Vec3b>({pt.x, pt.y}) = color; }
    };
}

Window::~Window() { close(); }

void Window::open(fmo::Dims dims) {
    if (mOpen) return;
    mOpen = true;
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);

    if (dims.height > 800) {
        dims.width /= 2;
        dims.height /= 2;
    }

    cv::resizeWindow(windowName, dims.width, dims.height);
}

void Window::close() {
    if (!mOpen) return;
    mOpen = false;
    cv::destroyWindow(windowName);
}

void Window::display(fmo::Mat& image) {
    open(image.dims());

    // print text into the image
    cv::Mat mat = image.wrap();
    if (!mLines.empty()) {
        int pad = 10;
        int fontFace = cv::FONT_HERSHEY_COMPLEX;
        double fontScale = 1;
        int bgThick = 6;
        int fgThick = 2;
        cv::Vec3b bgColor = {0x00, 0x00, 0x00};
        cv::Vec3b fgColor = {mColor.b, mColor.g, mColor.r};
        int lineHeight;

        {
            int dummy;
            auto lineSize = cv::getTextSize("ABC", fontFace, fontScale, bgThick, &dummy);
            lineHeight = (4 * lineSize.height) / 3;
        }

        int y = pad;
        for (auto& line : mLines) {
            y += lineHeight;
            cv::Point origin = {pad, y};
            cv::putText(mat, line, origin, fontFace, fontScale, bgColor, bgThick);
            cv::putText(mat, line, origin, fontFace, fontScale, fgColor, fgThick);
        }
        mLines.clear();
    }

    // display the image
    cv::imshow(windowName, mat);
}

Command Window::getCommand(bool block) {
    int waitMs = 1;

    if (block) {
        waitMs = 0;
    } else {
        int64_t sinceLastKeyTime = fmo::nanoTime() - mLastKeyTime;
        int64_t waitNs = mFrameTimeNs - sinceLastKeyTime;
        if (waitNs > 1'000'000) { waitMs = int(waitNs / 1'000'000); }
    }

    int keyCode = cv::waitKey(waitMs);
    mLastKeyTime = fmo::nanoTime();
    return encodeKey(keyCode);
}

Command Window::encodeKey(int keyCode) {
    switch (keyCode) {
    case 27: // escape
        return Command::QUIT;
    case 13: // cr
    case 10: // lf
        return Command::STEP;
    case ' ':
        return Command::PAUSE;
    default:
        return Command::NONE;
    };
}

void drawPoints(const fmo::PointSet& points, fmo::Mat& target, Color color) {
    FMO_ASSERT(target.format() == fmo::Format::BGR, "bad format");
    cv::Mat mat = target.wrap();
    cv::Vec3b vec = {color.b, color.g, color.r};

    for (auto& pt : points) { mat.at<cv::Vec3b>({pt.x, pt.y}) = vec; }
}

void drawPointsGt(const fmo::PointSet& ps, const fmo::PointSet& gt, fmo::Mat& target) {
    FMO_ASSERT(target.format() == fmo::Format::BGR, "bad format");
    cv::Mat mat = target.wrap();
    PutColor c1{toCv(colorFp), mat};
    PutColor c2{toCv(colorFn), mat};
    PutColor c3{toCv(colorTp), mat};
    fmo::pointSetCompare(ps, gt, c1, c2, c3);
}
