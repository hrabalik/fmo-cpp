#include "window.hpp"
#include "desktop-opencv.hpp"
#include <chrono>
#include <fmo/assert.hpp>
#include <fmo/pointset.hpp>
#include <fmo/stats.hpp>
#include <thread>

#define TOSTR_INNER(x) #x
#define TOSTR(x) TOSTR_INNER(x)

namespace {
    const char* const windowName = "FMO";

    constexpr Colour colourFp = Colour::lightMagenta();
    constexpr Colour colourFn = Colour::lightRed();
    constexpr Colour colourTp = Colour::lightGreen();

    constexpr int downscaleThresh = 800;

    cv::Vec3b toCv(Colour c) { return {c.b, c.g, c.r}; }

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

    if (dims.height > downscaleThresh) {
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
    cv::Mat mat = image.wrap();
    printText(mat);
    cv::imshow(windowName, mat);
}

void Window::printText(cv::Mat& mat) {
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = mat.rows / 1000.;
    int thick = (mat.rows > downscaleThresh) ? 2 : 1;
    int lineWidth = 0;
    int lineHeight = 0;
    int baseline = 0;

    {
        auto lineSize = cv::getTextSize("ABC", fontFace, fontScale, thick, &baseline);
        lineHeight = lineSize.height;
    }

    int above = (9 * lineHeight / 14) + (lineHeight / 2);
    int below = 5 * lineHeight / 14;
    int pad = lineHeight / 2;
    cv::Scalar color(mColour.b, mColour.g, mColour.r);

    // render text in top left corner
    if (!mLines.empty()) {
        for (auto& line : mLines) {
            auto lineSize = cv::getTextSize(line, fontFace, fontScale, thick, &baseline);
            lineWidth = std::max(lineWidth, lineSize.width);
        }

        // darken the area for the text
        int xMax = 2 * pad + lineWidth;
        int yMax = 2 * pad + int(mLines.size()) * (above + below);
        cv::Rect rect{0, 0, xMax, yMax};
        mat(rect) = 0.3 * mat(rect);

        // render the text
        int y = pad;
        for (auto& line : mLines) {
            y += above;
            cv::Point origin = {pad, y};
            cv::putText(mat, line, origin, fontFace, fontScale, color, thick);
            y += below;
        }
        mLines.clear();
    }

    // render bottom line
    if (mBottomLine != "") {
        int helpRectHeight = (above + below) + 2 * pad;
        cv::Rect helpRect{0, mat.rows - helpRectHeight, mat.cols, helpRectHeight};
        mat(helpRect) = 0.3 * mat(helpRect);
        cv::Point helpOrigin{pad, mat.rows - pad - below};
        cv::putText(mat, mBottomLine, helpOrigin, fontFace, fontScale, color, thick);
    }
}

Command Window::getCommand(bool block) {
    int keyCode = cv::waitKey(block ? 0 : 1);
    int64_t sinceLastNs = fmo::nanoTime() - mLastNs;
    int64_t waitNs = mFrameNs - sinceLastNs - 1'000'000;

    if (waitNs > 0) {
        std::chrono::nanoseconds chronoNs{waitNs};
        std::this_thread::sleep_for(chronoNs);
    }

    mLastNs = fmo::nanoTime();
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
    case ',':
        return Command::JUMP_BACKWARD;
    case '.':
        return Command::JUMP_FORWARD;
    case 'r':
    case 'R':
        return Command::RECORD;
    case 'a':
    case 'A':
        return Command::AUTOMATIC_MODE;
    case 'm':
    case 'M':
        return Command::MANUAL_MODE;
    case 'e':
    case 'E':
        return Command::FORCED_EVENT;
    case '?':
    case 'h':
    case 'H':
        return Command::SHOW_HELP;
    case 's':
    case 'S':
        return Command::PLAY_SOUNDS;
    default:
        return Command::NONE;
    };
}

void drawPoints(const fmo::PointSet& points, fmo::Mat& target, Colour colour) {
    FMO_ASSERT(target.format() == fmo::Format::BGR, "bad format");
    cv::Mat mat = target.wrap();
    cv::Vec3b vec = {colour.b, colour.g, colour.r};

    for (auto& pt : points) { mat.at<cv::Vec3b>({pt.x, pt.y}) = vec; }
}

void drawPointsGt(const fmo::PointSet& ps, const fmo::PointSet& gt, fmo::Mat& target) {
    FMO_ASSERT(target.format() == fmo::Format::BGR, "bad format");
    cv::Mat mat = target.wrap();
    PutColor c1{toCv(colourFp), mat};
    PutColor c2{toCv(colourFn), mat};
    PutColor c3{toCv(colourTp), mat};
    fmo::pointSetCompare(ps, gt, c1, c2, c3);
}
