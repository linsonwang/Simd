/*
* Simd Library Tests.
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
#include "Test/TestUtils.h"
#include "Test/TestPerformance.h"
#include "Test/Test.h"

namespace Test
{
	namespace
	{
		struct FuncBgra
		{
			typedef void (*FuncPtr)(uchar * dst, size_t stride, size_t width, size_t height, uchar blue, uchar green, uchar red, uchar alpha);

			FuncPtr func;
			std::string description;

			FuncBgra(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

			void Call(View & dst, uchar blue, uchar green, uchar red, uchar alpha) const
			{
				TEST_PERFORMANCE_TEST(description);
				func(dst.data, dst.stride, dst.width, dst.height, blue, green, red, alpha);
			}
		};
	}

#define ARGS_BGRA(width, height, function1, function2) \
	width, height, FuncBgra(function1, std::string(#function1)), FuncBgra(function2, std::string(#function2))

	bool FillBgraTest(int width, int height, const FuncBgra & f1, const FuncBgra & f2)
	{
		bool result = true;

        std::cout << "Test " << f1.description << " & " << f2.description
            << " [" << width << ", " << height << "]." << std::endl;

        uchar blue = Random(256);
        uchar green = Random(256);
        uchar red = Random(256);
        uchar alpha = Random(256);

		View d1(width, height, View::Bgra32, NULL, TEST_ALIGN(width));
		View d2(width, height, View::Bgra32, NULL, TEST_ALIGN(width));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(d1, blue, green, red, alpha));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(d2, blue, green, red, alpha));

		result = result && Compare(d1, d2, 0, true, 10);

		return result;
	}

	bool FillBgraTest()
	{
		bool result = true;

		result = result && FillBgraTest(ARGS_BGRA(W, H, Simd::Base::FillBgra, Simd::FillBgra));
        result = result && FillBgraTest(ARGS_BGRA(W + 1, H - 1, Simd::Base::FillBgra, Simd::FillBgra));
        result = result && FillBgraTest(ARGS_BGRA(W - 1, H + 1, Simd::Base::FillBgra, Simd::FillBgra));

#if defined(SIMD_SSE2_ENABLE) && defined(SIMD_AVX2_ENABLE)
        if(Simd::Sse2::Enable && Simd::Avx2::Enable)
        {
            result = result && FillBgraTest(ARGS_BGRA(W, H, Simd::Sse2::FillBgra, Simd::Avx2::FillBgra));
            result = result && FillBgraTest(ARGS_BGRA(W + 1, H - 1, Simd::Sse2::FillBgra, Simd::Avx2::FillBgra));
            result = result && FillBgraTest(ARGS_BGRA(W - 1, H + 1, Simd::Sse2::FillBgra, Simd::Avx2::FillBgra));
        }
#endif 

		return result;
	}
}