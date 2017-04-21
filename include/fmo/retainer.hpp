#ifndef FMO_RETAINER_HPP
#define FMO_RETAINER_HPP

#include <array>
#include <vector>

namespace fmo {
    /// A container for objects of type T, which must be default constructible and also must have a
    /// method named clear() which reverts the object to a state equivalent to that after default
    /// construction.
    ///
    /// The first Count elements are never destroyed, clear() is called on them instead.
    template <typename T, size_t Count>
    struct Retainer {
        Retainer() = default;
        Retainer(const Retainer&) = default;
        Retainer(Retainer&&) = default;
        Retainer& operator=(const Retainer&) = default;
        Retainer& operator=(Retainer&&) = default;

        void swap(Retainer& rhs) {
            std::swap(mSz, rhs.mSz);
            mArr.swap(rhs.mArr);
            mVec.swap(rhs.mVec);
        }
        friend void swap(Retainer& lhs, Retainer& rhs) { lhs.swap(rhs); }

        bool empty() const { return mSz == 0; }
        size_t size() const { return mSz; }

        void emplace_back() {
            if (++mSz > Count) { mVec.emplace_back(); }
        }

        void pop_back() {
            if (mSz-- > Count) {
                mVec.pop_back();
            } else {
                mArr[mSz].clear();
            }
        }

        T& back() {
            if (mSz > Count) return mVec.back();
            return mArr[mSz - 1];
        }

        const T& back() const {
            if (mSz > Count) return mVec.back();
            return mArr[mSz - 1];
        }

    private:
        std::array<T, Count> mArr;
        std::vector<T> mVec;
        size_t mSz = 0;
    };
}

#endif // FMO_RETAINER_HPP
