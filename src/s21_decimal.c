#include "s21_decimal.h"

#include <limits.h>

// ADDITIONAL FUNCS
// ================================================================================
int equal_scale(s21_decimal *value_1, s21_decimal *value_2) {
  if (get_exp(value_1->bits[3]) != get_exp(value_2->bits[3])) {
    int err = 0;
    int diff = get_exp(value_1->bits[3]) - get_exp(value_2->bits[3]);
    s21_decimal *min;
    s21_decimal *max;
    int exp = get_exp(value_1->bits[3]);
    if (diff < 0) {
      diff *= -1;
      min = value_1;
      max = value_2;
      exp = get_exp(value_2->bits[3]);
    } else {
      min = value_2;
      max = value_1;
    }
    int diff1 = diff;
    int diff2 = diff;

    int sign = get_sign(*min);
    s21_decimal test = *min;
    int exp1 = get_exp(test.bits[3]);
    while (diff1) {
      multen(&test);
      diff1--;
    }
    while (diff2) {
      s21_div_logic(test, (s21_decimal){{10, 0, 0, 0}}, &test);
      diff2--;
    }
    test.bits[3] = set_exp(test.bits[3], exp1);
    set_sign(&test, sign);
    if (s21_is_equal(test, *min)) {
      while (diff && err == 0) {
        err += multen(min);
        diff--;
      }
      if (err == 0) min->bits[3] = set_exp(min->bits[3], exp);
    } else {
      while (diff && err == 0) {
        s21_div_logic(*max, (s21_decimal){{10, 0, 0, 0}}, max);
        diff--;
      }
    }
    set_sign(min, sign);
  }
  return 0;
}

int get_exp(int bits_3) {
  int exp = 0;
  for (int i = 23; i >= 16; i--) {
    int b1 = (bits_3 >> i) & 1;
    exp <<= 1;
    exp |= b1;
  }
  return exp;
}
int set_exp(int bits_3, int exp) {
  int minus = 0;
  if (bits_3 >> 31 & 1) minus = 1;
  bits_3 >>= 16;
  bits_3 = exp;
  bits_3 <<= 16;
  if (minus) bits_3 |= 0b10000000000000000000000000000000;
  return bits_3;
}

int set_sign(s21_decimal *value, int x) {
  if (x) {
    value->bits[3] |= 0b10000000000000000000000000000000;
  } else {
    value->bits[3] &= 0b01111111111111111111111111111111;
  }
  return 0;
}

int get_sign(s21_decimal value) {
  int sign = (value.bits[3] >> 31) & 1;
  if ((value.bits[0] | value.bits[1] | value.bits[2]) == 0) sign = 0;
  return sign;
}

int get_bit(const s21_decimal dst, int bit) {
  int res = 0;
  if (bit / 32 < 4) {
    unsigned int mask = 1u << (bit % 32);
    res = dst.bits[bit / 32] & mask;
  }
  return !!res;
}

int zeroing_dec(s21_decimal *value) {
  value->bits[0] = 0;
  value->bits[1] = 0;
  value->bits[2] = 0;
  return 0;
}

void shift_dec_left(s21_decimal *value, int n) {
  int t = 0, t1 = 0;
  for (int i = 0; i < n; i++) {
    if (value->bits[0] >> 31 & 1) t = 1;
    value->bits[0] <<= 1;
    if (value->bits[1] >> 31 & 1) t1 = 1;
    value->bits[1] <<= 1;
    if (t) value->bits[1] |= 1;
    value->bits[2] <<= 1;
    if (t1) value->bits[2] |= 1;
  }
}

int multen(s21_decimal *value) {
  int error = 0;
  error = s21_mul(*value, (s21_decimal){{10, 0, 0, 0}}, value);
  return error;
}

// ARITHMETIC
// ================================================================================
int s21_round(s21_decimal value, s21_decimal *result) {
  int sign = get_sign(value);
  *result = (s21_decimal){{0, 0, 0, 0}};
  int exp_orig = get_exp(value.bits[3]);
  s21_decimal check = {{0, 0, 0, 0}};
  s21_decimal value_div = {{10, 0, 0, 0}};
  s21_decimal check_n = {0};
  value.bits[3] = set_exp(value.bits[3], exp_orig - 1);
  s21_truncate(value, &value);
  s21_mod(value, value_div, &check_n);
  if (check_n.bits[0] > 4) check.bits[0] = 1;
  s21_div_logic(value, value_div, result);
  s21_add_logic(*result, check, result);
  set_sign(result, sign);
  return 0;
}

int s21_floor(s21_decimal value, s21_decimal *result) {
  s21_truncate(value, result);
  if (get_sign(value) == 1) {
    s21_add_logic(*result, (s21_decimal){{1, 0, 0, 0}}, result);
    set_sign(result, 1);
  }
  return 0;
}

int s21_negate(s21_decimal value, s21_decimal *result) {
  *result = value;
  ((value.bits[3] >> 31 & 1) == 1) ? set_sign(result, 0) : set_sign(result, 1);
  return 0;
}

int s21_truncate(s21_decimal value, s21_decimal *result) {
  int exp = get_exp(value.bits[3]);
  s21_decimal value_div = {{10, 0, 0, 0}};
  *result = value;
  while (exp != 0) {
    s21_div_logic(*result, value_div, result);
    exp--;
  }
  result->bits[3] = set_exp(result->bits[3], 0);
  return 0;
}

s21_decimal s21_div_logic(s21_decimal value_1, s21_decimal value_2,
                          s21_decimal *result) {
  s21_decimal mod = {0};
  zeroing_dec(result);
  for (int i = 95; i >= 0; i--) {
    shift_dec_left(&mod, 1);
    shift_dec_left(result, 1);
    mod.bits[0] |= get_bit(value_1, i);
    if (s21_is_greater_or_equal_noexp(mod, value_2)) {
      s21_sub_logic(mod, value_2, &mod);
      result->bits[0] |= 1;
    }
  }
  return mod;
}

int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  *result = (s21_decimal){{0, 0, 0, 0}};
  if (value_2.bits[0] == 0 && value_2.bits[1] == 0 && value_2.bits[2] == 0)
    return 3;
  equal_scale(&value_1, &value_2);
  *result = (s21_decimal)s21_div_logic(value_1, value_2, result);
  s21_decimal res = *result;
  int n = 0;
  while (res.bits[0] > 1) {
    s21_div_logic(res, (s21_decimal){{10, 0, 0, 0}}, &res);
    n++;
  }
  result->bits[3] = set_exp(result->bits[3], n);
  return 0;
}

int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  if (value_2.bits[0] == 0 && value_2.bits[1] == 0 && value_2.bits[2] == 0)
    return 3;
  int exp = 0;
  s21_decimal mod = s21_div_logic(value_1, value_2, result);
  s21_decimal res = {};
  while (mod.bits[0] != 0 && exp < 9) {
    multen(&mod);
    mod = s21_div_logic(mod, value_2, &res);
    exp++;
    multen(result);
    s21_add_logic(res, *result, result);
  }
  result->bits[3] = set_exp(result->bits[3], exp);
  return 0;
}

int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  int sign1 = get_sign(value_1);
  int sign2 = get_sign(value_2);
  int err = 0;
  *result = (s21_decimal){{0, 0, 0, 0}};
  s21_decimal res;
  for (int i = 0; i < 96; i++) {
    int b2 = get_bit(value_2, i);
    if (b2 == 1) {
      res = *result;
      zeroing_dec(result);
      err += s21_add_logic(res, value_1, result);
      if (value_1.bits[2] >= 0b10000000000000000000000000000000 && i != 0)
        err = 1;
    }
    shift_dec_left(&value_1, 1);
  }
  int b1 = get_exp(value_1.bits[3]);
  int b2 = get_exp(value_2.bits[3]);
  result->bits[3] = set_exp(result->bits[3], b1 + b2);
  if (sign1 + sign2 == 1) {
    if (s21_is_not_equal(*result, (s21_decimal){{0, 0, 0, 0}}))
      set_sign(result, 1);
    if (err == 1) err++;
  }
  if (err) *result = (s21_decimal){{0, 0, 0, 0}};
  return err;
}

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  *result = (s21_decimal){{0, 0, 0, 0}};
  int exp1 = get_exp(value_1.bits[3]);
  int exp2 = get_exp(value_2.bits[3]);
  int exp = exp1 > exp2 ? exp1 : exp2;
  equal_scale(&value_1, &value_2);
  int sign1 = get_sign(value_1);
  int sign2 = get_sign(value_2);
  int err = 0;
  if ((sign1 + sign2) == 0) {
    err = s21_add_logic(value_1, value_2, result);
    result->bits[3] = set_exp(result->bits[3], exp);
    while (err && exp) {
      s21_div_logic(value_2, (s21_decimal){{10, 0, 0, 0}}, &value_2);
      s21_div_logic(value_1, (s21_decimal){{10, 0, 0, 0}}, &value_1);
      exp--;
      err = s21_add_logic(value_1, value_2, result);
      result->bits[3] = set_exp(result->bits[3], exp);
    }
    return err;
  }
  if ((sign1 + sign2) == 2) {
    err = s21_add_logic(value_1, value_2, result);
    set_sign(result, 1);
    result->bits[3] = set_exp(result->bits[3], exp);
    while (err && exp) {
      s21_div_logic(value_2, (s21_decimal){{10, 0, 0, 0}}, &value_2);
      s21_div_logic(value_1, (s21_decimal){{10, 0, 0, 0}}, &value_1);
      exp--;
      err = s21_add_logic(value_1, value_2, result);
      result->bits[3] = set_exp(result->bits[3], exp);
    }
    if (err) err += 1;
    return err;
  }
  if (s21_is_less(value_1, value_2)) {
    s21_decimal res;
    s21_negate(value_1, &res);
    if (s21_is_less(res, value_2)) {
      s21_sub_logic(value_2, value_1, result);
    } else {
      s21_sub_logic(value_1, value_2, result);
      set_sign(result, 1);
    }
  } else {
    s21_decimal res;
    s21_negate(value_2, &res);
    if (s21_is_less(res, value_1)) {
      s21_sub_logic(value_1, value_2, result);
    } else {
      s21_sub_logic(value_2, value_1, result);
      set_sign(result, 1);
    }
  }
  result->bits[3] = set_exp(result->bits[3], exp);
  if (err) zeroing_dec(result);
  if (err && sign1 && sign2) err += 1;
  return err;
}

int s21_add_logic(s21_decimal value_1, s21_decimal value_2,
                  s21_decimal *result) {
  *result = (s21_decimal){{0, 0, 0, 0}};
  int k = 0, err = 0;
  for (int i = 0; i < 96; i++) {
    int b1 = get_bit(value_1, i);
    int b2 = get_bit(value_2, i);
    if (i == 95 && ((b1 && b2) || ((b1 || b2) && k))) err = 1;
    result->bits[i / 32 % 4] |=
        k ? (!(b1 ^ b2) ? 1 << (i) : 0 << i) : ((b1 ^ b2) ? 1 << (i) : 0 << i);
    k = (b1 & b2) ? (b1 & b2) : (k & (b1 | b2));
  }
  return err;
}

int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  *result = (s21_decimal){{0, 0, 0, 0}};
  int sign1 = get_sign(value_1);
  int sign2 = get_sign(value_2);
  int exp1 = get_exp(value_1.bits[3]);
  int exp2 = get_exp(value_2.bits[3]);
  int exp = exp1 > exp2 ? exp1 : exp2;
  equal_scale(&value_1, &value_2);
  int err = 0;
  if (s21_is_less(value_1, value_2)) {
    if (sign1) {
      if (!sign2) err = s21_add_logic(value_2, value_1, result);
      if (sign2) s21_sub_logic(value_1, value_2, result);
    } else {
      s21_sub_logic(value_2, value_1, result);
    }
    set_sign(result, 1);
  } else {
    if (sign2) {
      if (!sign1) err = s21_add_logic(value_2, value_1, result);
      if (sign1) s21_sub_logic(value_2, value_1, result);
    } else {
      s21_sub_logic(value_1, value_2, result);
    }
  }
  result->bits[3] = set_exp(result->bits[3], exp);
  return err == 0 ? 0 : 2;
}

int s21_sub_logic(s21_decimal value_1, s21_decimal value_2,
                  s21_decimal *result) {
  *result = (s21_decimal){0};
  int k = 0, k1 = 0;
  for (int i = 0; i < 95; i++) {
    int b1 = get_bit(value_1, i);
    int b2 = get_bit(value_2, i);
    if ((b1 == 0) && k) k1 = 1;
    if (b1 < b2) k = 1;
    if (b1 && k) {
      b1 = 0, k = 0, k1 = 0;
    }
    if (b1 < b2) k = 1;
    if (b1 < k) {
      if (k1) {
        b1 = 1;
      } else {
        b1 = 0;
      }
    }
    result->bits[i / 32 % 4] |= (b1 ^ b2) ? 1 << i : 0 << i;
  }
  return 0;
}

// COMPARISON
// ================================================================================

int s21_is_less(s21_decimal value_1, s21_decimal value_2) {
  equal_scale(&value_1, &value_2);
  int sign_1 = get_sign(value_1);
  int sign_2 = get_sign(value_2);
  int check = 0;
  for (int i = 95; i >= 0; i--) {
    int b1 = get_bit(value_1, i);
    int b2 = get_bit(value_2, i);
    if (b1 ^ b2) {
      if (b2) check = 1;
      break;
    }
  }
  if (sign_1 && !sign_2) check = 1;
  if (!sign_1 && sign_2) check = 0;
  if (sign_1 && sign_2) check = check == 1 ? 0 : 1;
  return check;
}
int s21_is_less_noexp(s21_decimal value_1, s21_decimal value_2) {
  int check = 0;
  for (int i = 95; i >= 0; i--) {
    int b1 = get_bit(value_1, i);
    int b2 = get_bit(value_2, i);
    if (b1 ^ b2) {
      if (b2) check = 1;
      break;
    }
  }
  return check;
}

int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2) {
  return s21_is_less(value_1, value_2) + s21_is_equal(value_1, value_2);
}

int s21_is_greater(s21_decimal value_1, s21_decimal value_2) {
  return s21_is_less_or_equal(value_1, value_2) ? 0 : 1;
}

int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2) {
  return s21_is_less(value_1, value_2) ? 0 : 1;
}

int s21_is_greater_or_equal_noexp(s21_decimal value_1, s21_decimal value_2) {
  return s21_is_less_noexp(value_1, value_2) ? 0 : 1;
}

int s21_is_equal(s21_decimal value_1, s21_decimal value_2) {
  equal_scale(&value_1, &value_2);
  int check = 1;
  for (int i = 95; i >= 0; i--) {
    int b1 = get_bit(value_1, i);
    int b2 = get_bit(value_2, i);
    if (b1 ^ b2) check = 0;
  }
  if (get_sign(value_2) != get_sign(value_2)) check = 0;
  return check;
}

int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2) {
  return s21_is_equal(value_1, value_2) ? 0 : 1;
}

// CONVERTERS
// ================================================================================

int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  *dst = (s21_decimal){{0, 0, 0, 0}};
  if (src < 0) {
    src = -src;
    set_sign(dst, 1);
  }
  dst->bits[0] = src;
  return 0;
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
  *dst = 0;
  int error = 0;
  int sign = get_sign(src);
  int exp = get_exp(src.bits[3]);
  while (exp) {
    s21_div_logic(src, (s21_decimal){{10, 0, 0, 0}}, &src);
    exp--;
  }
  if (src.bits[1] == 0 && src.bits[2] == 0 && src.bits[0] <= INT_MAX) {
    *dst = src.bits[0];
    if (sign == 1) *dst = -*dst;
  } else {
    error = 1;
  }
  return error;
}

int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  *dst = (s21_decimal){{0, 0, 0, 0}};
  int error = 0;
  if (src != s21_INF && isnormal(src)) {
    int n = 0;
    if (src < 0) {
      set_sign(dst, 1);
      src = -src;
    }
    while (src > 1) {
      src /= 10;
      n++;
    }
    src *= 10000000;
    dst->bits[0] = (int)roundf(src);
    dst->bits[3] = set_exp(dst->bits[3], 7 - n);
  } else {
    error = 1;
  }
  return error;
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  int res = 0;
  if (!dst || get_exp(src.bits[3]) > 28) {
    res = 1;
  } else {
    *dst = 0.0;
    int scale = get_exp(src.bits[3]);
    int sign = get_bit(src, 127);
    double tmp = *dst;
    unsigned long base = 1;
    for (int i = 0; i < 96; i++) {
      tmp += get_bit(src, i) * base;
      base = base * 2;
    }
    while (scale != 0) {
      tmp = tmp / 10;
      scale--;
    }
    if (sign) tmp *= -1;
    *dst = tmp;
  }
  return res;
}
