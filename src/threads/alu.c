#include "alu.h"
#include "lib/stdint.h"

int float_to_int(Float a){
    return a / FRACTION;
}
Float _int_to_float(int a){
    return a * FRACTION;
}

Float multiply_float_int(Float float_number, int integer){
    return float_number * integer;
}
Float multiply_float_float(Float a, Float b){
    int64_t result = a;
    result = result * b / FRACTION;
    return (Float)result;
}

Float add_float_float(Float a, Float b){
    return a + b;
}
Float add_float_int(Float float_number, int integer){
    return float_number + _int_to_float(integer);
}
Float sub_int_float(int a, Float b){
    return _int_to_float(a) - b;
}

Float divide_int_int(int num, int divider){
    return num * FRACTION / divider;
}
Float divide_float_float(Float num, Float divider){
    int64_t result = num;
    result = result *FRACTION / divider;
    return (Float)result;
}

Float divide_float_int(Float num, int divider)
{
    return num / divider;
}