#include <cstring>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <bit>
#include <type_traits>

uint32_t maxu(uint32_t A, uint32_t B) {
  return A > B ? A : B;
}

uint32_t mul(uint32_t A, uint32_t B) {
  uint64_t t0, t1, t2;
  t0 = A;
  t1 = B;
  t2 = (t0*t1) >> 32;
  return t2;
}

uint32_t mullow(uint32_t A, uint32_t B) {
  uint64_t t0, t1, t2;
  t0 = A;
  t1 = B;
  t2 = (t0 * t1) & 0xFFFFFFFF; // Mask to extract lower 32 bits
  return t2;
}




uint32_t nlz(uint32_t x) {
  uint32_t z = 0;

  if (x == 0) return 32;
  if (x <= 0x0000FFFF) {
    z = z + 16;
    x = x << 16;
  }
  if (x <= 0x00FFFFFF) {
    z = z + 8;
    x = x << 8;
  }

  if (x <= 0x0FFFFFFF) {
    z = z + 4;
    x = x << 4;
  }
  if (x <= 0x3FFFFFFF) {
    z = z + 2;
    x = x << 2;
  }
  if (x <= 0x7FFFFFFF) {
    z = z + 1;
  }
  return z;
}

std::ostream& operator<<(std::ostream& os, _Float16 f) {
    return os << static_cast<float>(f);
}    
template <typename T>
std::enable_if_t<std::is_integral_v<T>, void> print_variable(T value, const char* name) {
  std::cout << std::setw(20) << name << " value is       : " << value << std::endl;
}

template <typename T>
std::enable_if_t<std::is_integral_v<T>, void> print_bits(T value, const char* name) {
  std::bitset<sizeof(T) * 8> b = value;
  std::cout << std::setw(20) << name << " bit pattern is : " << b << std::endl;
}

template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, void> print_variable(T value, const char* name) {
  using bit_t = std::bitset<sizeof(T) * 8>;
  bit_t b = std::bit_cast<_BitInt(sizeof(T)*8)>(value);
  std::cout << std::setw(20) << name << " value is       : " << value << std::endl;
  std::cout << std::setw(20) << name << " bit pattern is : " << b << std::endl;
}

void print_variable(_Float16 value, const char* name) {
  std::bitset<16> b = std::bit_cast<uint16_t>(value);
  std::cout << std::setw(20) << name << " value is       : " << value << std::endl;
  std::cout << std::setw(20) << name << " bit pattern is : " << b << std::endl;
}

_Float16 fmul(float aa, float bb) {
  uint32_t aa_bits = std::bit_cast<uint32_t>(aa);
  uint32_t bb_bits = std::bit_cast<uint32_t>(bb); 

  uint32_t absx, ex;
  absx = aa_bits & 0x7FFFFFFF;
  ex = absx >> 23; // exponent of x
  print_variable(ex, "ex");

  uint32_t absy, ey;
  absy = bb_bits & 0x7FFFFFFF;
  ey = absy >> 23; //exponent of y
  print_variable(ey, "ey");

  uint32_t nx, ny;
  nx = absx >= 0x800000; // is normal bit for x
  print_variable(nx, "nx");

  ny = absy >= 0x800000;// is normal bit for y
  print_variable(ny, "ny");

  uint32_t mx, my, lambday, lambdax;
  // the max between the leading zeros of x and the bit width ofthe  exponent wise
  mx = maxu(nlz(absx), 8); // nlz(absx) = leading zeros of x and 8 is the bit width of the exponent bias
  print_variable(mx, "mx");

  lambdax = mx - 8;  //leading zeros of significand x
  print_variable(lambdax, "lambdax");

  // the max between the leading zeros of y and the bit width ofthe  exponent wise
  my = maxu(nlz(absy), 8);
  print_variable(my, "my");

  lambday = my - 8; // PP
  print_variable(lambday, "lambday");

  uint16_t dm1;
  uint32_t mpx, mpy, highs,lows, b;
  uint32_t  g, hight, lowt, morlowt, c, m;
  mpx = (aa_bits << mx) | 0x80000000; // normalize significand x
  mpy = (bb_bits << my) |  0x80000000; // normalize significand y
  print_bits(mpx, "mpx");
  print_bits(mpy, "mpy");
  highs = mul(mpx, mpy); // upper 32-bit of the product of mantissas
  c = highs >= 0x80000000; // c = 0 if m'xm'y in [1,2) else 1 if m'xm'y in [2,4)
  print_variable(c, "c");
  lows = mullow(mpx, mpy); // lower 32-bit of the product of mantissas

  // the product of two 32 bit is a 64 bit number. this 64 bit can be sttored in two 32 bit variable; hence the highs and lows variable
  print_bits(highs, "highs");
  print_bits(lows, "lows");

  lowt = (lows != 0);
  m = (highs >> (20 + c)); // the significand
  print_bits(m, "m");
  morlowt = m | lowt;

  g = (highs >> (19 + c)) & 1;
  hight = (highs << (26 - c)) != 0;
  print_variable(g, "guard bit");
  print_variable(hight, "sticky bit for highs");
  print_variable(lowt, "sticky bit for lows");
  // b denotes the bit used to round to nearest in RN(l) = (1.s1,..s23) (base 2)  + B^-23
  b = g & (morlowt | hight);
  print_bits(b, "b");

  dm1 = ((ex + ey) - 239) + c; // biased exponent
  print_variable(dm1, "dm1");
  print_bits(dm1, "dm1");

  uint16_t sr = (aa_bits ^ bb_bits) & 0x80000000;
  print_variable(sr, "sign bit of result");

  uint16_t exp16 = sr | (dm1 << 10);
  print_bits(exp16, "exp16");

  uint16_t result = (exp16 + m) + b;
  print_bits(result, "result");

  _Float16 result16 = std::bit_cast<_Float16>(result);
  // std::memcpy(&result16, &result, sizeof(result16));  
  //result = *reinterpret_cast<_Float16*>(&result);
  print_variable(result16, "result16");
  return result16;
}

int main() {
    float x = 3.0;
    float y = 5.0;

    print_variable(fmul(x, y), "fmul(x, y)");
    _Float16 number = 15;
    print_variable(number, "number");

    uint32_t three_bits = std::bit_cast<uint32_t>(x);
    uint16_t truncated_bits = static_cast<uint16_t>(three_bits);

    return 0;
}
/*
  so 0| 1000 0010 | 1110 0000 0000 0000 0000 000 and 0 | 1001 0 | 1110 0000 00
 */
