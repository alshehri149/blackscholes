/* para.c
 *
 * Author:
 * Date  :
 *
 *  Description
 */

/* Standard C includes */
#include <stdlib.h>

/* Include common headers */
#include "common/macros.h"
#include "common/types.h"

/* Include application-specific headers */
#include "include/types.h"

#include <math.h>

#define inv_sqrt_2xPI 0.39894228040143270286

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
    float NofXd1 = CNDFPara(xD1);
    float NofXd2 = CNDFPara(xD2);
    float FutureValueX = strike * exp(-rate * time);
    float OptionPrice;

    if (otype == 0) {
        OptionPrice = sptprice * NofXd1 - FutureValueX * NofXd2;
    } else {
        OptionPrice = FutureValueX * (1.0 - NofXd2) - sptprice * (1.0 - NofXd1);
    }
    return OptionPrice;
}

void blackScholesHelperPara(float *sptprice, float *strike, float *rate,
                              float *volatility, float *otime, char *otype,
                              float *OptionPrice, size_t size) {
    for (int i = 0; i < size; i++) {
        char type = otype[i] == 'C' ? 0 : 1;
        OptionPrice[i] = blackScholesPara(sptprice[i], strike[i], rate[i],
                                            volatility[i], otime[i], type,
                                            0);
    }

}




/* Alternative Implementation */
void* impl_parallel(void* args)
{
    args_t *parsed_args = (args_t *) args;


    blackScholesHelperPara(
            (float *) parsed_args->sptPrice,
            (float *) parsed_args->strike,
            (float *) parsed_args->rate,
            (float *) parsed_args->volatility,
            (float *) parsed_args->otime,
            (char *) parsed_args->otype,
            (float *) parsed_args->output,
            parsed_args->num_stocks
    );


  return NULL;
}