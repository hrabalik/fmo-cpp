#include "window.hpp"
#include "desktop-opencv.hpp"

#define TOSTR_INNER(x) #x
#define TOSTR(x) TOSTR_INNER(x)

namespace {
    constexpr fmo::Dims defaultSize = {1280, 720};
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
