#include <platform.h>
#include <stdio.h>
#include <vector>
#include <xcore/hwtimer.h>
#include "xmath/xmath.h"


// reference 1 pole
// double maxiFilter::lopass(double input, double cutoff) {
// 	output=outputs[0] + cutoff*(input-outputs[0]);
// 	outputs[0]=output;
// 	return(output);
// }


// from bfp example in lib_xcore_math github
// Prints a BFP vector both in its mantissa-exponent form and as a floating point vector. Also prints headroom.
static void print_vector(
    const bfp_s32_t* vect,
    const char* name,
    const unsigned line_num)
{
  printf("=== From line #%u ===\n", line_num);

  // First, the raw mantissas
  printf("%s = [", name);
  for(unsigned int k = 0; k < vect->length; k++)
    printf("%ld, ", (long int)vect->data[k]);
  printf("] * 2**(%d)\n", vect->exp);

  // Next, the float equivalent
  printf("%s_float = [", name);
  for(unsigned int k = 0; k < vect->length; k++)
    printf("%0.07f, ", ldexp(vect->data[k], vect->exp));
  printf("]\n");

  // And the headroom
  printf("%s headroom: %u\n\n", name, vect->hr);
}

#define PRINT_VECT(X)     print_vector(&X, #X, __LINE__)
#define PRINT_SCALAR(X)   printf("%s = %0.07f\n\n", #X, ldexp((double) X.mant, X.exp))

void runTest(size_t nFilters, size_t nRuns) {
  std::vector<float> signals(nFilters);
  std::vector<float> outputs(nFilters);
  // std::vector<float> z(nFilters,0);  
  std::vector<float> outputs0(nRuns);


  printf("==================================\n\n");
  const float cutoff = 0.2;

  printf("IEE Test: %d runs, filterbank size: %d\n", nRuns, nFilters);
  
  auto siggen = [] (size_t i) {
    return i % 20 < 8 ? 1.f : -1.f;
  };

	hwtimer_t timer = hwtimer_alloc();
	auto now = hwtimer_get_time(timer);

  for(size_t i=0; i < nRuns; i++) {
    float sig = siggen(i);
    for(size_t n = 0; n < nFilters; n++) {
      signals[n] = sig;
    }
    for(size_t n = 0; n < nFilters; n++) {
      outputs[n] = outputs[n] + (cutoff * (signals[n] - outputs[n]));
      // z[n] = outputs[n];
    }
    // outputs0[i] = (outputs[0]);
  }
	auto t2 = hwtimer_get_time(timer);
	printf("start time: %lf\n", now * 1e-8);
	printf("end time: %lf\n", t2 * 1e-8);
	printf("total time: %lf s\n", (t2-now) * 1e-8);
  float unitTime = (t2-now) / nRuns * 1e-8;
	printf("unit time: %lf s\n", unitTime);
  printf("speed: %f Hz\n", 1.0 / unitTime);

  // for(size_t i=0; i < outputs0.size(); i++)
  //   printf("%f\n", outputs0[i]);



  //quick bfp test, multiple vector by scalar, print results
  double exp = -20;
	double mult = 1.0 / pow(2,exp);

  // int32_t testBuffer[10];
  // bfp_s32_t testBfp;
  // for(size_t i=0; i < 10; i++) {
  //   testBuffer[i] = (i+1) * mult;
  // }
  // bfp_s32_init(&testBfp, testBuffer, exp, 10, 1);
  // {
  //   float_s32_t alpha = f32_to_float_s32(2.5f);
  //   bfp_s32_scale(&testBfp, &testBfp, alpha);
  //   PRINT_VECT(testBfp);
  //   printf("\n\n");
  // }  

  //vector unit implementation
  bfp_s32_t x3Signals, x3Outputs, x3z, x3tmp, x3tmp2;

  // The bfp_s32_t type does not allocate its own buffer for the mantissa vector (nor does the initialization function).
  // Instead, it contains a pointer that points to the backing buffer.
  int32_t x3SignalsBuf[nFilters];
  int32_t x3OutputsBuf[nFilters];
  int32_t x3zBuf[nFilters];
  int32_t x3tmpBuf[nFilters];
  int32_t x3tmp2Buf[nFilters];

  bfp_s32_init(&x3tmp, x3tmpBuf, exp, nFilters, 0);
  bfp_s32_init(&x3tmp2, x3tmp2Buf, exp, nFilters, 0);

  //zero the z vector
  for(size_t n=0; n < nFilters; n++) {
    x3zBuf[n] = 0;
    x3OutputsBuf[0] = n;
  }
  bfp_s32_init(&x3z, x3zBuf, exp, nFilters, 1);
  bfp_s32_init(&x3Outputs, x3OutputsBuf, exp, nFilters, 1);


  float_s32_t alpha = f32_to_float_s32(cutoff);
  printf("\n\nXMOS Vector Unit Test: %d runs, filterbank size: %d\n", nRuns, nFilters);
	now = hwtimer_get_time(timer);

  //signals vector
  // for(size_t n=0; n < nFilters; n++) {
  //   x3SignalsBuf[n] = sig * mult;
  // }
  bfp_s32_init(&x3Signals, x3SignalsBuf, exp, nFilters, 0);

  for(size_t i=0; i < nRuns; i++) {
    float sig = siggen(i);
    bfp_s32_set(&x3Signals, sig * mult, exp);

    //signal - z
    bfp_s32_sub(&x3tmp, &x3Signals, &x3Outputs);
    //alpha * (signal-z)
    bfp_s32_scale(&x3tmp, &x3tmp, alpha);
    //z + above
    bfp_s32_add(&x3Outputs, &x3Outputs, &x3tmp);
    //z = outputs

    // PRINT_VECT(x3Outputs);
  }
	t2 = hwtimer_get_time(timer);
	printf("start time: %lf\n", now * 1e-8);
	printf("end time: %lf\n", t2 * 1e-8);
	printf("total time: %lf s\n", (t2-now) * 1e-8);
  unitTime = (t2-now) / nRuns * 1e-8;
	printf("unit time: %lf s\n", unitTime);
  printf("speed: %f Hz\n\n\n\n", 1.0 / unitTime);

  hwtimer_free(timer);

}

int main() {
  printf("Filter Bank Test\n");

  // const size_t nFilters = 28;
  // const size_t nRuns = 10000;

  for(auto &x: {4,8,16,24,28,32,40,64,128,256,512,1024,2048,4096,8192}) {
    runTest(x,10000);
  }

}
