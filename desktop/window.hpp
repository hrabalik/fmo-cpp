#ifndef FMO_DESKTOP_WINDOW_HPP
#define FMO_DESKTOP_WINDOW_HPP

#include <cstdint>
#include <fmo/common.hpp>
#include <fmo/pointset.hpp>

enum class Command {
    NONE,
    STEP,
    PAUSE,
    QUIT,
};

struct Color {
    uint8_t b, g, r;
};

/// Class of visualization and GUI-related procedures.
struct Window {
    ~Window();

    /// Closes the UI window (if open).
    void close();

    /// Sets the text color. In a given frame, all text will be rendered with the same color.
    void setTextColor(Color color) { mColor = color; }

    /// Adds text to be rendered when the next image is displayed.
    void print(const std::string& line) { mLines.push_back(line); }

    /// Renders the specified image on screen. The image may be modified by text rendering.
    void display(fmo::Mat& image);

    /// Sets the preferred frame duration in seconds.
    void setFrameTime(float sec) { mFrameTimeNs = int64_t(1e9f * sec); }

    /// Receives a command from the user. If the argument is false, blocks for some time between 1
    /// millisecond and the time set by the last call to setFrameTime(). If the argument is true,
    /// block indefinitely.
    Command getCommand(bool block);

private:
    void open(fmo::Dims dims);
    static Command encodeKey(int keyCode);

    // data
    int64_t mFrameTimeNs = 0;
    int64_t mLastKeyTime = 0;
    std::vector<std::string> mLines;
    Color mColor = {0x00, 0x90, 0x00};
    bool mOpen = false;
};

/// Visualize a given set of points painting it onto the target image with the specified color.
void drawPoints(const fmo::PointSet& points, fmo::Mat& target, Color color);

/// Visualize result point set in comparison with the ground truth point set.
void drawPointsGt(const fmo::PointSet& ps, const fmo::PointSet& gt, fmo::Mat& target);

#endif // FMO_DESKTOP_WINDOW_HPP
