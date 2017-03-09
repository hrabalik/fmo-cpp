#define _CRT_SECURE_NO_WARNINGS // using std::localtime is insecure
#include "video.hpp"
#include "desktop-opencv.hpp"
#include <algorithm>
#include <ctime>
#include <fmo/assert.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

// VideoInput

VideoInput::VideoInput(VideoInput&& rhs) = default;
VideoInput& VideoInput::operator=(VideoInput&&) = default;

VideoInput::VideoInput(std::unique_ptr<cv::VideoCapture>&& cap)
    : mMat(std::make_unique<cv::Mat>()), mCap(std::move(cap)) {
    if (!mCap->isOpened()) { throw std::runtime_error("failed to open video"); }

#if CV_MAJOR_VERSION == 2
    mFps = (float)mCap->get(CV_CAP_PROP_FPS);
    mDims.width = (int)mCap->get(CV_CAP_PROP_FRAME_WIDTH);
    mDims.height = (int)mCap->get(CV_CAP_PROP_FRAME_HEIGHT);
#elif CV_MAJOR_VERSION == 3
    mFps = (float)mCap->get(cv::CAP_PROP_FPS);
    mDims.width = (int)mCap->get(cv::CAP_PROP_FRAME_WIDTH);
    mDims.height = (int)mCap->get(cv::CAP_PROP_FRAME_HEIGHT);
#endif
}

std::unique_ptr<VideoInput> VideoInput::makeFromCamera(int camId) {
    try {
        return std::make_unique<VideoInput>(std::make_unique<cv::VideoCapture>(camId));
    } catch (std::exception& e) {
        std::cerr << "while opening camera ID " << camId << "\n";
        throw e;
    }
}

std::unique_ptr<VideoInput> VideoInput::makeFromFile(const std::string& filename) {
    try {
        return std::make_unique<VideoInput>(std::make_unique<cv::VideoCapture>(filename));
    } catch (std::exception& e) {
        std::cerr << "while opening file '" << filename << "'\n";
        throw e;
    }
}

fmo::Region VideoInput::receiveFrame() {
    *mCap >> *mMat;

    if (mMat->empty()) {
        return fmo::Region{fmo::Format::UNKNOWN, {0, 0}, {0, 0}, nullptr, nullptr, 0};
    }

    FMO_ASSERT(mMat->type() == CV_8UC3, "bad type");
    FMO_ASSERT(mMat->cols == mDims.width, "bad width");
    FMO_ASSERT(mMat->rows == mDims.height, "bad height");

    auto rowStep = size_t(3 * mDims.width);
    return fmo::Region{fmo::Format::BGR, {0, 0}, mDims, mMat->data, nullptr, rowStep};
}

// VideoOutput

VideoOutput::VideoOutput(VideoOutput&& rhs) = default;
VideoOutput& VideoOutput::operator=(VideoOutput&&) = default;

VideoOutput::VideoOutput(std::unique_ptr<cv::VideoWriter>&& writer, fmo::Dims dims)
    : mWriter(std::move(writer)), mDims(dims) {
    if (!mWriter->isOpened()) { throw std::runtime_error("failed to open file for recording"); }
}

std::unique_ptr<VideoOutput> VideoOutput::makeFile(const std::string& filename, fmo::Dims dims,
                                                   float fps) {
    int fourCC = CV_FOURCC('D', 'I', 'V', 'X');
    fps = std::max(15.f, fps);
    cv::Size size = {dims.width, dims.height};

    try {
        return std::make_unique<VideoOutput>(
            std::make_unique<cv::VideoWriter>(filename, fourCC, fps, size, true), dims);
    } catch (std::exception& e) {
        std::cerr << "while opening file '" << filename << "'\n";
        throw e;
    }
}

std::unique_ptr<VideoOutput> VideoOutput::makeInDirectory(const std::string& dir, fmo::Dims dims,
                                                          float fps) {
    time_t time = std::time(nullptr);
    std::tm* ltm = std::localtime(&time);

    std::ostringstream outFile;
    outFile << std::setfill('0');
    outFile << dir << '/' << (ltm->tm_year + 1900) << '-' << std::setw(2) << (ltm->tm_mon + 1)
            << '-' << std::setw(2) << (ltm->tm_mday) << '-' << std::setw(2) << (ltm->tm_hour)
            << std::setw(2) << (ltm->tm_min) << std::setw(2) << (ltm->tm_sec) << ".avi";

    return makeFile(outFile.str(), dims, fps);
}

void VideoOutput::sendFrame(const fmo::Mat& frame) {
    FMO_ASSERT(frame.format() == fmo::Format::BGR, "bad format");
    FMO_ASSERT(frame.dims().width == mDims.width, "bad width");
    FMO_ASSERT(frame.dims().height == mDims.height, "bad height");
    cv::Mat mat = frame.wrap();
    *mWriter << mat;
}
