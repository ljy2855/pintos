#ifndef THREADS_ALU_H
#define THREADS_ALU_H

#define FRACTION (1 << 14)
typedef int Float;

Float multiply_float_int(Float float_number, int integer);
Float multiply_float_float(Float a,Float b);

Float add_float_float(Float a,Float b);
Float add_float_int(Float float_number, int integer);
int sub_int_float(int a,Float b);


Float divide_int_int(int num, int divider);
Float divide_float_float(Float num,Float divider);
Float divide_float_int(Float num, int divider);

Float float_to_int(Float a);
Float _int_to_float(int a);

#endif