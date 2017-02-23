#ifndef __CMP_FT_COMMON_HPP__
#define __CMP_FT_COMMON_HPP__

#ifdef __cplusplus

#include "FASTex.hpp"
#include <opencv2/features2d/features2d.hpp>


namespace cmp
{

void makeOffsets(int pixel[34], int* corners, int* cornersOut, int row_stride, int patternSize, int pixelIndex[34], int pixelcheck[24], int pixelcheck16[16]);
void makeOffsetsC(int pixel[34], int pixelCounter[34], int corners[8], int rowStride, int patternSize, int pixelcheck[24], int pixelcheck16[16]);

template<int patternSize>
int cornerScore(const uchar* ptr, const int pixel[], int threshold);

template<typename _Tp>
static inline bool isMostSameAccessible(const uchar* ptr, int img_step, int xstep, int mostSameIdx, int threshold, long (*distFunction)(const _Tp&, const _Tp&))
{
	if( mostSameIdx > 15 )
		mostSameIdx -= 16;
	switch(mostSameIdx){
	case 0:
		if( distFunction(*ptr, ptr[img_step])  > threshold )
			return false;
		if( distFunction(*ptr, ptr[2*img_step])  > threshold )
			return false;
		break;
	case 1:
		if( distFunction(*ptr, ptr[img_step])  > threshold )
			return false;
		if( distFunction(*ptr, ptr[2*img_step + 1 * xstep])  > threshold )
			return false;
		break;
	case 15:
		 if( !(distFunction(*ptr, ptr[img_step])  <= threshold || distFunction(*ptr, ptr[img_step - 1 * xstep]) <= threshold)  )
			 return false;
		 if( distFunction(*ptr, ptr[2*img_step - 1 * xstep])  > threshold )
			 return false;
		 break;
	case 2:
		if( distFunction(*ptr, ptr[img_step + 1 * xstep])  > threshold )
			return false;
		break;
	case 3:
		if( !(distFunction(*ptr, ptr[1 * xstep])  <= threshold || distFunction(*ptr, ptr[1 * xstep + img_step])  <= threshold) )
			return false;
		if( distFunction(*ptr, ptr[img_step + 2 * xstep])  > threshold )
			return false;
		break;
	case 4:
		if( distFunction(*ptr, ptr[1])  > threshold )
			return false;
		if( distFunction(*ptr, ptr[2])  > threshold )
			return false;
		break;
	case 5:
		if( !(distFunction(*ptr, ptr[1])  <= threshold || distFunction(*ptr, ptr[1 -img_step])  <= threshold ))
			return false;
		if( distFunction(*ptr, ptr[-img_step + 2 * xstep])  > threshold )
			return false;
		break;
	case 6:
		if( distFunction(*ptr, ptr[-img_step + 1 * xstep])  > threshold )
			return false;
		break;
	case 7:
		if( !(distFunction(*ptr, ptr[-img_step])  <= threshold || distFunction(*ptr, ptr[-img_step + + 1 * xstep])  <= threshold) )
			return false;
		if( distFunction(*ptr, ptr[-2*img_step + 1 * xstep])  > threshold )
			return false;
		break;
	case 8:
		if( distFunction(*ptr, ptr[-img_step])  > threshold )
			return false;
		if( distFunction(*ptr, ptr[-2*img_step])  > threshold )
			return false;
		break;
	case 9:
		if( !( distFunction(*ptr, ptr[-img_step])  <= threshold || distFunction(*ptr, ptr[-img_step - 1 * xstep])  <= threshold) )
			return false;
		if( distFunction(*ptr, ptr[-2*img_step - 1 * xstep])  > threshold )
			return false;
		break;
	case 10:
		if( distFunction(*ptr,  ptr[-img_step - 1 * xstep])  > threshold )
			return false;
		break;
	case 11:
		if( !( distFunction(*ptr,  ptr[ -1 * xstep])  <= threshold || distFunction(*ptr,  ptr[ -1 * xstep -img_step])  <= threshold ) )
			return false;
		if( distFunction(*ptr,  ptr[ -img_step -2 * xstep])  > threshold )
			return false;
		break;
	case 12:
		if( distFunction(*ptr,  ptr[ -1 * xstep])  > threshold )
			return false;
		if( distFunction(*ptr,  ptr[ -2 * xstep])  > threshold )
			return false;
		break;
	case 13:
		if( !(distFunction(*ptr,  ptr[ -1 * xstep])  <= threshold || distFunction(*ptr,  ptr[ -1 * xstep + img_step]) <= threshold ) )
			return false;
		if( distFunction(*ptr,  ptr[ img_step -2 * xstep])  > threshold )
			return false;
		break;
	case 14:
		if( distFunction(*ptr,  ptr[img_step -1 * xstep])  > threshold )
			return false;
		break;
	}
	return true;
}

template<typename _Tp>
static inline bool isMostSameAccessible12(const uchar* ptr, int img_step, int xstep, int cn, int mostSameIdx, int threshold, long (*distFunction)(const _Tp&, const _Tp&))
{
	if( mostSameIdx > 11 )
		mostSameIdx -= 12;
	switch(mostSameIdx){
	case 0:
		if( !( distFunction(ptr[cn], ptr[img_step + cn])  <= threshold ) )
			return false;
		break;
	case 1:
		if( !( distFunction(ptr[cn], ptr[img_step + cn])  <= threshold
				|| distFunction(ptr[cn], ptr[img_step + xstep + cn])  <= threshold ) )
			return false;
		break;
	case 11:
		 if( !(distFunction(ptr[cn], ptr[img_step + cn])  <= threshold
				 || distFunction(ptr[cn], ptr[img_step -xstep + cn])  <= threshold) )
			 return false;
		 break;
	case 2:
		if( !(distFunction(ptr[cn], ptr[1 * xstep + cn])  <= threshold
				|| distFunction(ptr[cn], ptr[img_step + xstep + cn])  <= threshold ) )
			return false;
		break;
	case 3:
		if( !(distFunction(ptr[cn], ptr[xstep + cn])  <= threshold) )
			return false;
		break;
	case 4:
		if( !(distFunction(ptr[cn], ptr[1 * xstep + cn])  <= threshold
				|| distFunction(ptr[cn], ptr[-img_step + xstep + cn])  <= threshold ) )
			return false;
		break;
	case 5:
		if( !( distFunction(ptr[cn], ptr[-img_step + cn])  <= threshold
				|| distFunction(ptr[cn], ptr[-img_step + xstep + cn])  <= threshold ))
			return false;
		break;
	case 6:
		if( !(distFunction(ptr[cn], ptr[-img_step + cn])  <= threshold ) )
			return false;
		break;
	case 7:
		if( !(distFunction(ptr[cn], ptr[-img_step + cn])  <= threshold
				|| distFunction(ptr[cn], ptr[-img_step - xstep + cn])  <= threshold) )
			return false;
		break;
	case 8:
		if( !(distFunction(ptr[cn], ptr[-1*xstep + cn])  <= threshold
				|| distFunction(ptr[cn], ptr[-1*xstep - img_step + cn])  <= threshold ) )
			return false;
		break;
	case 9:
		if( !( distFunction(ptr[cn], ptr[-1*xstep + cn])  <= threshold ) )
			return false;
		break;
	case 10:
		if( !(distFunction(ptr[cn],  ptr[-1*xstep + cn])  <= threshold
				|| distFunction(ptr[cn],  ptr[-1*xstep + img_step + cn])  <= threshold) )
			return false;
		break;
	}
	return true;
}

}//namespace cmp

#endif
#endif //__CMP_FT_COMMON_HPP__
