#ifndef FMO_DESKTOP_WINDOW_HPP
#define FMO_DESKTOP_WINDOW_HPP

#include <cstdint>
#include <fmo/common.hpp>

struct Color {
    uint8_t b, g, r;
};

struct Window {
    ~Window();
    void close();
    void display(const fmo::Mat& image);

private:
    void open(fmo::Dims dims);

    // data
    bool mOpen = false;
};

#endif // FMO_DESKTOP_WINDOW_HPP
