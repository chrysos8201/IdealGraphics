#pragma once
#include "Core/Core.h"
// IEEE 754 half-precision float (16-bit) 변환 함수들
uint16 float32_to_float16(float value);
float float16_to_float32(uint16 h);

// 두 개의 16비트 float를 하나의 32비트 값으로 병합
uint32 pack_float16_to_32(uint16 f1, uint16 f2);

// 32비트 값에서 두 개의 16비트 float 추출
void unpack_float32_to_16(uint32 packed, uint16& OutF1, uint16& OutF2);

float PackFloat16ToFloat32(float f1, float f2);