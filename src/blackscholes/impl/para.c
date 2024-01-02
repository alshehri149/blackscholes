/* para.c
 *
 * Author:
 * Date  :
 *
 *  Description
 */

/* Standard C includes */
#include <stdlib.h>
#include <pthread.h>



/* Include common headers */
#include "common/macros.h"
#include "common/types.h"

/* Include application-specific headers */
#include "include/types.h"

#include <math.h>

#define inv_sqrt_2xPI 0.39894228040143270286
#define NUM_THREADS 4



float CNDFPara(float InputX) {
    int sign;
    if (InputX < 0.0) {
        InputX = -InputX;
        sign = 1;
    } else {
        sign = 0;
    }
    float xInput = InputX;
    float expValues = exp(-0.5f * InputX * InputX);
    float xNPrimeofX = expValues * inv_sqrt_2xPI;
    float xK2 = 0.2316419 * xInput;
    xK2 = 1.0 + xK2;
    xK2 = 1.0 / xK2;
    float xLocal_1 = xK2 * 0.319381530;
    float xLocal_2 = xK2 * xK2 * (-0.356563782);
    float xLocal_3 = xK2 * xK2 * xK2 * 1.781477937;
    xLocal_2 = xLocal_2 + xLocal_3;
    xLocal_3 = xK2 * xK2 * xK2 * xK2 * (-1.821255978);
    xLocal_2 = xLocal_2 + xLocal_3;
    xLocal_3 = xK2 * xK2 * xK2 * xK2 * xK2 * 1.330274429;
    xLocal_2 = xLocal_2 + xLocal_3;
    xLocal_1 = xLocal_2 + xLocal_1;
    float xLocal = xLocal_1 * xNPrimeofX;
    xLocal = 1.0 - xLocal;
    if (sign) {
        xLocal = 1.0 - xLocal;
    }
    return xLocal;
}


float blackScholesPara(float sptprice, float strike, float rate, float volatility, float time, int otype, float timet) {
    float xSqrtTime = sqrt(time);
    float logValues = log(sptprice / strike);
    float xD1 = (rate + 0.5 * volatility * volatility) * time + logValues;
    xD1 = xD1 / (volatility * xSqrtTime);
    float xD2 = xD1 - volatility * xSqrtTime;
    float NofXd1 = CNDFPara(xD1);  // Use the parallel version of CNDF
    float NofXd2 = CNDFPara(xD2);  // Use the parallel version of CNDF
    float FutureValueX = strike * exp(-rate * time);
    float OptionPrice;

    if (otype == 0) {
        OptionPrice = sptprice * NofXd1 - FutureValueX * NofXd2;
    } else {
        OptionPrice = FutureValueX * (1.0 - NofXd2) - sptprice * (1.0 - NofXd1);
    }
    return OptionPrice;
}


typedef struct {
    float *sptprice;
    float *strike;
    float *rate;
    float *volatility;
    float *otime;
    char *otype;
    float *OptionPrice;
    size_t size;
    size_t start;
    size_t end;
} ThreadData;


void blackScholesHelperPara(float *sptprice, float *strike, float *rate,
                            float *volatility, float *otime, char *otype,
                            float *OptionPrice, size_t size, size_t start, size_t end) {
    for (int i = start; i < end; i++) {
        char type = otype[i] == 'C' ? 0 : 1;
        OptionPrice[i] = blackScholesPara(sptprice[i], strike[i], rate[i],
                                          volatility[i], otime[i], type,
                                          0);
    }
    
}

void *thread_function(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    blackScholesHelperPara(data->sptprice, data->strike, data->rate, 
                           data->volatility, data->otime, data->otype, 
                           data->OptionPrice, data->size, data->start, data->end);
    return NULL;

}   



/* Alternative Implementation */
void* impl_parallel(void* args)
{

args_t *parsed_args = (args_t *)args;

    // Number of threads
    const int num_threads = 4;
    pthread_t threads[num_threads];
    ThreadData threadData[num_threads];

    // Calculate the size of each chunk
    size_t size = parsed_args->num_stocks;
    size_t chunkSize = size / num_threads;

    // Create and start threads
    for (int i = 0; i < num_threads; i++) {
        threadData[i] = (ThreadData){
            .sptprice = parsed_args->sptPrice,
            .strike = parsed_args->strike,
            .rate = parsed_args->rate,
            .volatility = parsed_args->volatility,
            .otime = parsed_args->otime,
            .otype = parsed_args->otype,
            .OptionPrice = parsed_args->output,
            .size = size,
            .start = i * chunkSize,
            .end = (i + 1) * chunkSize
        };
        if (i == num_threads - 1) {
            threadData[i].end = size; // Ensure the last thread covers all remaining items
        }
        pthread_create(&threads[i], NULL, thread_function, &threadData[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    return NULL;

}