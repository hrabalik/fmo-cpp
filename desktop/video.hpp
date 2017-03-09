#ifndef FMO_DESKTOP_VIDEO_HPP
#define FMO_DESKTOP_VIDEO_HPP

#include <fmo/common.hpp>
#include <memory>
#include <string>

namespace cv {
    class VideoCapture;
    class VideoWriter;
}

struct VideoInput {
    VideoInput() = delete;
    VideoInput(const VideoInput&) = delete;
    VideoInput& operator=(const VideoInput&) = delete;
    VideoInput(VideoInput&&);
    VideoInput& operator=(VideoInput&&);

    static VideoInput makeFromCamera(int camId);
    static VideoInput makeFromFile(const std::string& filename);

    fmo::Dims dims() const { return mDims; }
    float fps() const { return mFps; }

private:
    VideoInput(std::unique_ptr<cv::VideoCapture>&& cap);

    // data
    std::unique_ptr<cv::VideoCapture> mCap;
    fmo::Dims mDims;
    float mFps;
};

struct VideoOutput {
    VideoOutput() = delete;
    VideoOutput(const VideoOutput&) = delete;
    VideoOutput& operator=(const VideoOutput&) = delete;
    VideoOutput(VideoOutput&&);
    VideoOutput& operator=(VideoOutput&&);

    static VideoOutput makeFile(const std::string& filename, fmo::Dims dims, float fps);
    static VideoOutput makeInDirectory(const std::string& dir, fmo::Dims dims, float fps);

private:
    VideoOutput(std::unique_ptr<cv::VideoWriter>&& writer);

    // data
    std::unique_ptr<cv::VideoWriter> mWriter;
};

#endif // FMO_DESKTOP_VIDEO_HPP
