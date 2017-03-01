#include "image-util.hpp"
#include <fmo/assert.hpp>
#include <fmo/processing.hpp>

namespace fmo {
    void save(const Mat& src, const std::string& filename) {
        cv::Mat srcMat = src.wrap();
        cv::imwrite(filename, srcMat);
    }

    void copy(const Mat& src, Mat& dst) {
        dst.resize(src.format(), src.dims());

        if (src.format() == Format::YUV420SP) {
            cv::Mat srcMat1 = yuv420SPWrapGray(src);
            cv::Mat srcMat2 = yuv420SPWrapUV(src);
            cv::Mat dstMat1 = yuv420SPWrapGray(dst);
            cv::Mat dstMat2 = yuv420SPWrapUV(dst);
            srcMat1.copyTo(dstMat1);
            srcMat2.copyTo(dstMat2);
        } else {
            cv::Mat srcMat = src.wrap();
            cv::Mat dstMat = dst.wrap();
            srcMat.copyTo(dstMat);
        }
    }

    void convert(const Mat& src, Mat& dst, Format format) {
        const auto srcFormat = src.format();
        const auto dstFormat = format;

        if (srcFormat == dstFormat) {
            // no format change -- just copy
            copy(src, dst);
            return;
        }

        if (&src == &dst) {
            if (srcFormat == dstFormat) {
                // same instance and no format change: no-op
                return;
            }

            if (srcFormat == Format::YUV420SP && dstFormat == Format::GRAY) {
                // same instance and converting YUV420SP to GRAY: easy case
                dst.resize(Format::GRAY, dst.dims());
                return;
            }

            // same instance: convert into a new, temporary Image, then move into dst
            Image temp;
            convert(src, temp, dstFormat);
            dst = std::move(temp);
            return;
        }

        enum { ERROR = -1 };
        int code = ERROR;

        dst.resize(dstFormat, src.dims()); // this is why we check for same instance
        cv::Mat srcMat = src.wrap();
        cv::Mat dstMat = dst.wrap();

        if (srcFormat == Format::BGR) {
            if (dstFormat == Format::GRAY) { code = cv::COLOR_BGR2GRAY; }
        } else if (srcFormat == Format::GRAY) {
            if (dstFormat == Format::BGR) { code = cv::COLOR_GRAY2BGR; }
        } else if (srcFormat == Format::YUV420SP) {
            if (dstFormat == Format::BGR) {
                code = cv::COLOR_YUV420sp2BGR;
            } else if (dstFormat == Format::GRAY) {
                code = cv::COLOR_YUV420sp2GRAY;
            }
        }

        if (code == ERROR) {
            throw std::runtime_error("convert: failed to perform color conversion");
        }

        cv::cvtColor(srcMat, dstMat, code);

        FMO_ASSERT(dstMat.data == dst.data(), "convert: dst buffer reallocated");
    }

    void less_than(const Mat& src, Mat& dst, uint8_t value) {
        if (src.format() != Format::GRAY) {
            throw std::runtime_error("less_than: input must be GRAY");
        }

        dst.resize(Format::GRAY, src.dims());
        cv::Mat srcMat = src.wrap();
        cv::Mat dstMat = dst.wrap();

        cv::threshold(srcMat, dstMat, value - 1, 0xFF, cv::THRESH_BINARY_INV);
        FMO_ASSERT(dstMat.data == dst.data(), "less_than: dst buffer reallocated");
    }

    void greater_than(const Mat& src, Mat& dst, uint8_t value) {
        if (src.format() != Format::GRAY) {
            throw std::runtime_error("less_than: input must be GRAY");
        }

        dst.resize(Format::GRAY, src.dims());
        cv::Mat srcMat = src.wrap();
        cv::Mat dstMat = dst.wrap();

        cv::threshold(srcMat, dstMat, value, 0xFF, cv::THRESH_BINARY);
        FMO_ASSERT(dstMat.data == dst.data(), "less_than: dst buffer reallocated");
    }

    void equal(const Mat& src, Mat& dst, uint8_t value) {
        if (src.format() != Format::GRAY) { throw std::runtime_error("equal: input must be GRAY"); }

        dst.resize(Format::GRAY, src.dims());
        cv::Mat srcMat = src.wrap();
        cv::Mat dstMat = dst.wrap();

        dstMat = srcMat == value;
        FMO_ASSERT(dstMat.data == dst.data(), "equal: dst buffer reallocated");
    }

    std::pair<uint8_t*, uint8_t*> min_max(Mat& src) {
        if (src.format() != Format::GRAY) {
            throw std::runtime_error("min_max: input must be GRAY");
        }

        cv::Mat srcMat = src.wrap();
        cv::Point min, max;
        cv::minMaxLoc(srcMat, nullptr, nullptr, &min, &max);
        return {&srcMat.at<uint8_t>(min), &srcMat.at<uint8_t>(max)};
    }

    void absdiff(const Mat& src1, const Mat& src2, Mat& dst) {
        if (src1.format() != src2.format()) {
            throw std::runtime_error("diff: inputs must have same format");
        }
        if (src1.dims() != src2.dims()) {
            throw std::runtime_error("diff: inputs must have same size");
        }

        dst.resize(src1.format(), src1.dims());
        cv::Mat src1Mat = src1.wrap();
        cv::Mat src2Mat = src2.wrap();
        cv::Mat dstMat = dst.wrap();

        cv::absdiff(src1Mat, src2Mat, dstMat);
        FMO_ASSERT(dstMat.data == dst.data(), "resize: dst buffer reallocated");
    }
}
