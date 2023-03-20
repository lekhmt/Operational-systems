#include "../headers/sin.h"
#include <math.h>

float SinIntegral(float A, float B, float e){

    printf("|trapezoid sin|\n");
    float dx = (B - A) / e;
    int steps = (B - A) / dx;
    float current = A;
    float result = 0;
    for (int i = 0; i < steps; ++i){
        result += dx * (sin(current) + sin(current + dx)) / 2;
        current += dx;
    }
    result += (B - current) * (sin(B) + sin(current)) / 2;
    return result;

}