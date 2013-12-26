/*
* Simd Library.
*
* Copyright (c) 2011-2013 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy 
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
* copies of the Software, and to permit persons to whom the Software is 
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in 
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "Simd/SimdMemory.h"
#include "Simd/SimdInit.h"
#include "Simd/SimdLoad.h"
#include "Simd/SimdStore.h"
#include "Simd/SimdConversion.h"
#include "Simd/SimdSse2.h"

namespace Simd
{
#ifdef SIMD_SSE2_ENABLE    
    namespace Sse2
    {
		template <bool align> SIMD_INLINE void AdjustedYuv16ToBgra(__m128i y16, __m128i u16, __m128i v16, 
			const __m128i & a_0, __m128i * bgra)
		{
			const __m128i b16 = AdjustedYuvToBlue16(y16, u16);
			const __m128i g16 = AdjustedYuvToGreen16(y16, u16, v16);
			const __m128i r16 = AdjustedYuvToRed16(y16, v16);
			const __m128i bg8 = _mm_or_si128(b16, _mm_slli_si128(g16, 1));
			const __m128i ra8 = _mm_or_si128(r16, a_0);
			Store<align>(bgra + 0, _mm_unpacklo_epi16(bg8, ra8));
			Store<align>(bgra + 1, _mm_unpackhi_epi16(bg8, ra8));
		}

		template <bool align> SIMD_INLINE void Yuv16ToBgra(__m128i y16, __m128i u16, __m128i v16, 
			const __m128i & a_0, __m128i * bgra)
		{
			AdjustedYuv16ToBgra<align>(AdjustY16(y16), AdjustUV16(u16), AdjustUV16(v16), a_0, bgra);
		}

		template <bool align> SIMD_INLINE void Yuv8ToBgra(__m128i y8, __m128i u8, __m128i v8, const __m128i & a_0, __m128i * bgra)
		{
			Yuv16ToBgra<align>(_mm_unpacklo_epi8(y8, K_ZERO), _mm_unpacklo_epi8(u8, K_ZERO), 
				_mm_unpacklo_epi8(v8, K_ZERO), a_0, bgra + 0);
			Yuv16ToBgra<align>(_mm_unpackhi_epi8(y8, K_ZERO), _mm_unpackhi_epi8(u8, K_ZERO), 
				_mm_unpackhi_epi8(v8, K_ZERO), a_0, bgra + 2);
		}

		template <bool align> SIMD_INLINE void Yuv444pToBgra(const uint8_t * y, const uint8_t * u, 
			const uint8_t * v, const __m128i & a_0, uint8_t * bgra)
		{
			Yuv8ToBgra<align>(Load<align>((__m128i*)y), Load<align>((__m128i*)u), Load<align>((__m128i*)v), a_0, (__m128i*)bgra);
		}

		template <bool align> void Yuv444pToBgra(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride, 
			size_t width, size_t height, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
		{
			assert(width >= A);
			if(align)
			{
				assert(Aligned(y) && Aligned(yStride) && Aligned(u) &&  Aligned(uStride));
				assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
			}

			__m128i a_0 = _mm_slli_si128(_mm_set1_epi16(alpha), 1);
			size_t bodyWidth = AlignLo(width, A);
			size_t tail = width - bodyWidth;
			for(size_t row = 0; row < height; ++row)
			{
				for(size_t colYuv = 0, colBgra = 0; colYuv < bodyWidth; colYuv += A, colBgra += QA)
				{
					Yuv444pToBgra<align>(y + colYuv, u + colYuv, v + colYuv, a_0, bgra + colBgra);
				}
				if(tail)
				{
					size_t col = width - A;
					Yuv444pToBgra<false>(y + col, u + col, v + col, a_0, bgra + 4*col);
				}
				y += yStride;
				u += uStride;
				v += vStride;
				bgra += bgraStride;
			}
		}

		template <bool align> SIMD_INLINE void Yuv420pToBgra(const uint8_t * y, const __m128i & u, const __m128i & v, 
			const __m128i & a_0, uint8_t * bgra)
		{
			Yuv8ToBgra<align>(Load<align>((__m128i*)y + 0), _mm_unpacklo_epi8(u, u), _mm_unpacklo_epi8(v, v), a_0, (__m128i*)bgra + 0);
			Yuv8ToBgra<align>(Load<align>((__m128i*)y + 1), _mm_unpackhi_epi8(u, u), _mm_unpackhi_epi8(v, v), a_0, (__m128i*)bgra + 4);
		}

		template <bool align> void Yuv420pToBgra(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride, 
			size_t width, size_t height, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
		{
			assert((width%2 == 0) && (height%2 == 0) && (width >= DA) && (height >= 2));
			if(align)
			{
				assert(Aligned(y) && Aligned(yStride) && Aligned(u) &&  Aligned(uStride));
				assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
			}

			__m128i a_0 = _mm_slli_si128(_mm_set1_epi16(alpha), 1);
			size_t bodyWidth = AlignLo(width, DA);
			size_t tail = width - bodyWidth;
			for(size_t row = 0; row < height; row += 2)
			{
				for(size_t colUV = 0, colY = 0, colBgra = 0; colY < bodyWidth; colY += DA, colUV += A, colBgra += OA)
				{
					__m128i u_ = Load<align>((__m128i*)(u + colUV));
					__m128i v_ = Load<align>((__m128i*)(v + colUV));
					Yuv420pToBgra<align>(y + colY, u_, v_, a_0, bgra + colBgra);
					Yuv420pToBgra<align>(y + colY + yStride, u_, v_, a_0, bgra + colBgra + bgraStride);
				}
				if(tail)
				{
					size_t offset = width - DA;
					__m128i u_ = Load<false>((__m128i*)(u + offset/2));
					__m128i v_ = Load<false>((__m128i*)(v + offset/2));
					Yuv420pToBgra<false>(y + offset, u_, v_, a_0, bgra + 4*offset);
					Yuv420pToBgra<false>(y + offset + yStride, u_, v_, a_0, bgra + 4*offset + bgraStride);
				}
				y += 2*yStride;
				u += uStride;
				v += vStride;
				bgra += 2*bgraStride;
			}
		}

		void Yuv420pToBgra(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride, 
			size_t width, size_t height, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
		{
			if(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride) 
				&& Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
				Yuv420pToBgra<true>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
			else
				Yuv420pToBgra<false>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
		}

		void Yuv444pToBgra(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride, 
			size_t width, size_t height, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
		{
			if(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride) 
				&& Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
				Yuv444pToBgra<true>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
			else
				Yuv444pToBgra<false>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
		}
    }
#endif// SIMD_SSE2_ENABLE
}