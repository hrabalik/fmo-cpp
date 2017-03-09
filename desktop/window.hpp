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

/// Class of visualization and GUI-related procedures.
struct Window {
    ~Window();

    /// Closes the UI window (if open).
    void close();

    /// Renders the specified image on screen.
    void display(const fmo::Mat& image);

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
    bool mOpen = false;
};

struct Color {
    uint8_t b, g, r;
};

/// Visualize a given set of points painting it onto the target image with the specified color.
void drawPoints(const fmo::PointSet& points, fmo::Mat& target, Color color);

#endif // FMO_DESKTOP_WINDOW_HPP
