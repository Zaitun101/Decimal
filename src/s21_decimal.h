#ifndef SRC_S21_DECIMAL_H_
#define SRC_S21_DECIMAL_H_

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define s21_INF 1.0 / 0.0
#define s21_nan __builtin_nanf("0x7fc00000")

typedef struct {
  unsigned int bits[4];
} s21_decimal;

int get_bit(s21_decimal value, int bit_num);
int set_exp(int bits_3, int exp);
void print_dec(s21_decimal x);
void shift_dec_right(s21_decimal *value, int n);
void shift_dec_left(s21_decimal *value, int n);
int get_exp(int bits_3);
int zeroing_dec(s21_decimal *value);
void print_bit(int x);
int equal_scale(s21_decimal *value_1, s21_decimal *value_2);
int get_sign(s21_decimal value);
int s21_sub_logic(s21_decimal value_1, s21_decimal value_2,
                  s21_decimal *result);
int set_sign(s21_decimal *value, int x);
int s21_add_logic(s21_decimal value_1, s21_decimal value_2,
                  s21_decimal *result);
int multen(s21_decimal *value);
s21_decimal s21_div_logic(s21_decimal value_1, s21_decimal value_2,
                          s21_decimal *result);
// ARITHMETIC OPERATORS
int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);

// COMPARISON OPERATORS
int s21_is_less(s21_decimal value_1, s21_decimal value_2);
int s21_is_less_noexp(s21_decimal value_1, s21_decimal value_2);
int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_greater(s21_decimal value_1, s21_decimal value_2);
int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_greater_or_equal_noexp(s21_decimal value_1, s21_decimal value_2);
int s21_is_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2);

// CONVERTERS AND PARSERS
int s21_from_int_to_decimal(int src, s21_decimal *dst);
int s21_from_float_to_decimal(float src, s21_decimal *dst);
int s21_from_decimal_to_int(s21_decimal src, int *dst);
int s21_from_decimal_to_float(s21_decimal src, float *dst);

// OTHER FUNCTIONS
int s21_floor(s21_decimal value, s21_decimal *result);
int s21_round(s21_decimal value, s21_decimal *result);
int s21_truncate(s21_decimal value, s21_decimal *result);
int s21_negate(s21_decimal value, s21_decimal *result);

#endif  // SRC_S21_DECIMAL_H_
