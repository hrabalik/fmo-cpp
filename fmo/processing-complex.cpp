#include "image-util.hpp"
#include <fmo/processing.hpp>

namespace fmo {
    void deltaYUV420SP(const Mat& src1, const Mat& src2, Mat& dst) {
        if (src1.format() != Format::YUV420SP || src2.format() != Format::YUV420SP) {
            throw std::runtime_error("delta: inputs must be YUV420SP");
        }

        Dims dims = src1.dims();

        if (src2.dims() != dims) { throw std::runtime_error("delta: inputs must have same size"); }

        dst.resize(Format::GRAY, src1.dims());
        cv::Mat src1Mat = yuv420SPWrapGray(src1);
        cv::Mat src2Mat = yuv420SPWrapGray(src2);
        cv::Mat dstMat = dst.wrap();

        cv::absdiff(src1Mat, src2Mat, dstMat);
        cv::threshold(dstMat, dstMat, 19, 0xFF, cv::THRESH_BINARY);
    }

    void decimate(const Mat& src, Mat& dst) {
        if (src.format() == Format::YUV420SP) {
            throw std::runtime_error("downscale: source cannot be YUV420SP");
        }

        Dims srcDims = src.dims();
        Dims dstDims = {srcDims.width / 2, srcDims.height / 2};

        if (dstDims.width == 0 || dstDims.height == 0) {
            throw std::runtime_error("downscale: source is too small");
        }

        dst.resize(src.format(), dstDims);
        cv::Mat srcMat = src.wrap();
        cv::Mat dstMat = dst.wrap();

        if (srcDims.width % 2 != 0) { srcMat.flags &= ~cv::Mat::CONTINUOUS_FLAG; }

        srcMat.cols &= ~1;
        srcMat.rows &= ~1;

        cv::resize(srcMat, dstMat, cv::Size(dstDims.width, dstDims.height), 0, 0, cv::INTER_AREA);
    }

    void pyramid(const Mat& src, std::vector<Image>& dst, size_t levels) {
        const Mat* prevMat = &src;
        dst.resize(levels);

        for (Mat& dstMat : dst) {
            decimate(*prevMat, dstMat);
            prevMat = &dstMat;
        }
    }
}
