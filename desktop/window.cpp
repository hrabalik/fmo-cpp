#include "window.hpp"
#include "desktop-opencv.hpp"
#include <chrono>
#include <fmo/assert.hpp>
#include <fmo/stats.hpp>

#define TOSTR_INNER(x) #x
#define TOSTR(x) TOSTR_INNER(x)

namespace {
    const char* const windowName = TOSTR(FMO_BINARY_NAME);
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

void Window::display(const fmo::Mat& image) {
    open(image.dims());
    cv::imshow(windowName, image.wrap());
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

    for (auto& pt : points) {
        mat.at<cv::Vec3b>({pt.x, pt.y}) = vec;
    }
}
