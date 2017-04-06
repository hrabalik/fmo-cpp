#include "image-util.hpp"
#include <fmo/processing.hpp>

namespace fmo {
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

    struct Median3Job : public cv::ParallelLoopBody {
        enum {
            BATCH = 1,
        };

        Median3Job(const Mat& src1, const Mat& src2, const Mat& src3, Mat& dst, int cols)
            : mSrc1(src1.data()),
              mSrc2(src2.data()),
              mSrc3(src3.data()),
              mDst(dst.data()),
              mSkipSrc1(int(src1.skip())),
              mSkipSrc2(int(src2.skip())),
              mSkipSrc3(int(src3.skip())),
              mSkipDst(int(dst.skip())),
              mCols(cols) {}

        virtual void operator()(const cv::Range& rows) const override {
            uint8_t* dstRow = mDst + (mSkipDst * rows.start);
            uint8_t* dstRowEnd = mDst + (mSkipDst * rows.end);
            const uint8_t* src1Row = mSrc1 + (mSkipSrc1 * rows.start);
            const uint8_t* src2Row = mSrc2 + (mSkipSrc2 * rows.start);
            const uint8_t* src3Row = mSrc3 + (mSkipSrc3 * rows.start);

            for (; dstRow < dstRowEnd; dstRow += mSkipDst, src1Row += mSkipSrc1,
                                       src2Row += mSkipSrc2, src3Row += mSkipSrc3) {
                uint8_t* dst = dstRow;
                const uint8_t* src1 = src1Row;
                const uint8_t* src2 = src2Row;
                const uint8_t* src3 = src3Row;

                for (int i = 0; i < mCols; i++) {
                    uint8_t t = std::max(src1[i], src2[i]);
                    uint8_t s = std::min(src1[i], src2[i]);
                    t = std::min(t, src3[i]);
                    dst[i] = std::max(s, t);
                }
            }
        }

    private:
        const uint8_t* const mSrc1;
        const uint8_t* const mSrc2;
        const uint8_t* const mSrc3;
        uint8_t* const mDst;
        const int mSkipSrc1;
        const int mSkipSrc2;
        const int mSkipSrc3;
        const int mSkipDst;
        const int mCols;
    };

    void median3(const Mat& src1, const Mat& src2, const Mat& src3, Mat& dst) {
        Format format = src1.format();
        Dims dims = src1.dims();
        int cols = dims.width * int(getPixelStep(format));

        if (format == Format::YUV420SP) {
            throw std::runtime_error("median3: source cannot be YUV420SP");
        }
        if (cols % Median3Job::BATCH != 0) {
            throw std::runtime_error("median3: source must have a width divisible by BATCH");
        }
        if (format != src2.format() || dims != src2.dims() || format != src3.format() ||
            dims != src3.dims()) {
            throw std::runtime_error("median3: format/dimensions mismatch of inputs");
        }

        dst.resize(format, dims);
        Median3Job job{src1, src2, src3, dst, cols};
        cv::parallel_for_(cv::Range{0, dims.height}, job, cv::getNumThreads());
    }
}
