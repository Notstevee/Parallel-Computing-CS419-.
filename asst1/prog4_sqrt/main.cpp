#include <stdio.h>
#include <algorithm>
#include <pthread.h>
#include <math.h>
#include <immintrin.h>

#include "CycleTimer.h"
#include "sqrt_ispc.h"

using namespace ispc;

extern void sqrtSerial(int N, float startGuess, float* values, float* output);

static void verifyResult(int N, float* result, float* gold) {
    for (int i=0; i<N; i++) {
        if (fabs(result[i] - gold[i]) > 1e-4) {
            printf("Error: [%d] Got %f expected %f\n", i, result[i], gold[i]);
        }
    }
}

// Not working
/* void sqrtAVX2(int N,
                float initialGuess,
                float values[],
                float output[])
{

    
    __m256i workMask;
    __m256 kThreshold = _mm256_set1_ps(0.00001f);
    __m256 one = _mm256_set1_ps(1.f);
    __m256 nzero = _mm256_set1_ps(-0.0f);
    __m256 x;
    __m256 guess;
    __m256 error;
    __m256 working;
    __m256 whilemask;
    for (int i=0; i<N; i+=8) {
        
       
        x = _mm256_loadu_ps(values+i);          // float x = values[i];
        guess = _mm256_set1_ps(initialGuess);    // float guess = initialGuess;

        working = _mm256_mul_ps(guess, guess);  // float error = fabs(guess * guess * x - 1.f);
        working = _mm256_mul_ps(working, x);
        working = _mm256_sub_ps(working, one);
        error = _mm256_andnot_ps(nzero, working);

        whilemask = _mm256_cmp_ps(error,kThreshold,_CMP_GT_OS);
        while (_mm256_movemask_ps(_mm256_cmp_ps(whilemask,_mm256_set1_ps(0.0f), _CMP_EQ_OQ))!=0xff) {
            
            working = _mm256_mul_ps(_mm256_mul_ps(_mm256_mul_ps(_mm256_set1_ps(3.f),x),_mm256_mul_ps(_mm256_mul_ps(x,guess),_mm256_mul_ps(guess,guess))),_mm256_set1_ps(0.5f));
            
            guess = _mm256_blendv_ps(working, guess, whilemask);

            working = _mm256_mul_ps(_mm256_mul_ps(guess, guess), x);
            working = _mm256_sub_ps(working, one);
            
            error = _mm256_andnot_ps(nzero, working);
            whilemask = _mm256_cmp_ps(error,kThreshold,_CMP_GT_OS);

        }
        _mm256_store_ps(output+i,_mm256_mul_ps(x,guess));

    }
} */

int main() {

    const unsigned int N = 20 * 1000 * 1000;
    const float initialGuess = 1.0f;

    float* values = new float[N];
    float* output = new float[N];
    float* gold = new float[N];

    for (unsigned int i=0; i<N; i++)
    {
        // TODO: CS149 students.  Attempt to change the values in the
        // array here to meet the instructions in the handout: we want
        // to you generate best and worse-case speedups
        
        // starter code populates array with random input values
        //values[i] = .001f + 2.998f * static_cast<float>(rand()) / RAND_MAX;

        //Worst speedup
        //values[i] = .001f + 0.999f ;

        //Best speedup
        values[i] = .001f + 2.998f ;


    }

    // generate a gold version to check results
    for (unsigned int i=0; i<N; i++)
        gold[i] = sqrt(values[i]);

    //
    // And run the serial implementation 3 times, again reporting the
    // minimum time.
    //
    double minSerial = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        sqrtSerial(N, initialGuess, values, output);
        double endTime = CycleTimer::currentSeconds();
        minSerial = std::min(minSerial, endTime - startTime);
    }

    printf("[sqrt serial]:\t\t[%.3f] ms\n", minSerial * 1000);

    verifyResult(N, output, gold);

    //
    // Compute the image using the ispc implementation; report the minimum
    // time of three runs.
    //
    double minISPC = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        sqrt_ispc(N, initialGuess, values, output);
        double endTime = CycleTimer::currentSeconds();
        minISPC = std::min(minISPC, endTime - startTime);
    }

    printf("[sqrt ispc]:\t\t[%.3f] ms\n", minISPC * 1000);

    verifyResult(N, output, gold);

    // Clear out the buffer
    for (unsigned int i = 0; i < N; ++i)
        output[i] = 0;

    //
    // Tasking version of the ISPC code
    //
    double minTaskISPC = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        sqrt_ispc_withtasks(N, initialGuess, values, output);
        double endTime = CycleTimer::currentSeconds();
        minTaskISPC = std::min(minTaskISPC, endTime - startTime);
    }

    printf("[sqrt task ispc]:\t[%.3f] ms\n", minTaskISPC * 1000);

    verifyResult(N, output, gold);


    // Clear out the buffer
    for (unsigned int i = 0; i < N; ++i)
        output[i] = 0;

    //
    // Tasking version of the ISPC code
    //
    double minAVX2 = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        sqrtAVX2(N, initialGuess, values, output);
        double endTime = CycleTimer::currentSeconds();
        minAVX2 = std::min(minAVX2, endTime - startTime);
    }

    printf("[sqrt AVX2]:\t[%.3f] ms\n", minAVX2 * 1000);

    verifyResult(N, output, gold);


    printf("\t\t\t\t(%.2fx speedup from ISPC)\n", minSerial/minISPC);
    printf("\t\t\t\t(%.2fx speedup from task ISPC)\n", minSerial/minTaskISPC);

    delete [] values;
    delete [] output;
    delete [] gold;

    return 0;
}
