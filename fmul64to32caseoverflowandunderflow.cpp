#include <cstring>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <bit>
#include <type_traits>
#include <cmath>
using namespace std;


uint64_t maxu(uint64_t A, uint64_t B) {
  return A > B ? A : B;
}

uint64_t mul(uint64_t A, uint64_t B) {
  __uint128_t t0, t1, t2;
  t0 = A;
  t1 = B;
  t2 = (t0*t1) >> 64;
  return t2;
}



uint64_t mullow(uint64_t A, uint64_t B) {
  __uint128_t t0, t1, t2;
  t0 = A;
  t1 = B;
  t2 = t0 * t1;
  return (uint64_t)t2; // Return lower 64 bits
}



uint64_t nlz(uint64_t x) {
  uint64_t z = 0;

  if (x == 0) return 64;
  if (x <= 0x00000000FFFFFFFF) {
    z = z + 32;
    x = x << 32;
  }
  if (x <= 0x0000FFFFFFFFFFFF) {
    z = z + 16;
    x = x << 16;
  }
  if (x <= 0x00FFFFFFFFFFFFFF) {
    z = z + 8;
    x = x << 8;
  }
  if (x <= 0x0FFFFFFFFFFFFFFF) {
    z = z + 4;
    x = x << 4;
  }
  if (x <= 0x3FFFFFFFFFFFFFFF) {
    z = z + 2;
    x = x << 2;
  }
  if (x <= 0x7FFFFFFFFFFFFFFF) {
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

float fmul(double aa, double bb) {
  uint64_t aa_bits = std::bit_cast<uint64_t>(aa);
  uint64_t bb_bits = std::bit_cast<uint64_t>(bb); 

  uint64_t absx, ex;
  absx = aa_bits & 0x7FFFFFFFFFFFFFFF; // (2^63) - 1
  ex = absx >> 52; // exponent of x
  print_variable(ex, "ex");

  uint64_t absy, ey;
  absy = bb_bits & 0x7FFFFFFFFFFFFFFF;
  ey = absy >> 52; //exponent of y
  print_variable(ey, "ey");

  uint64_t nx, ny;
  nx = absx >= 0x10000000000000; // is normal bit for x, 2^53
  print_variable(nx, "nx");

  ny = absy >= 0x10000000000000;// is normal bit for y
  print_variable(ny, "ny");

  uint64_t mx, my, lambday, lambdax;
  // the max between the leading zeros of x and the bit width ofthe  exponent wise
  mx = maxu(nlz(absx), 11); // nlz(absx) = leading zeros of x and 8 is the bit width of the exponent bias
  print_variable(mx, "mx");

  lambdax = mx - 11;  //leading zeros of significand x
  print_variable(lambdax, "lambdax");

  // the max between the leading zeros of y and the bit width ofthe  exponent wise
  my = maxu(nlz(absy), 11);
  print_variable(my, "my");

  lambday = my - 11; //
  print_variable(lambday, "lambday");

  int32_t dm1;
  uint64_t mpx, mpy, highs,lows, b;
  uint64_t  g, hight, lowt, morlowt, c, m;
  mpx = (aa_bits << mx) | 0x8000000000000000; // normalize significand x
  mpy = (bb_bits << my) |  0x8000000000000000; // normalize significand y
  print_bits(mpx, "mpx");
  print_bits(mpy, "mpy");
  highs = mul(mpx, mpy); // upper 32-bit of the product of mantissas
  c = highs >= 0x8000000000000000; // c = 0 if m'xm'y in [1,2) else 1 if m'xm'y in [2,4)
  print_variable(c, "c");
  lows = mullow(mpx, mpy); // lower 32-bit of the product of mantissas

  // the product of two 32 bit is a 64 bit number. this 64 bit can be sttored in two 32 bit variable; hence the highs and lows variable
  print_bits(highs, "highs");
  print_bits(lows, "lows");

  lowt = (lows != 0);
 // m = (highs >> (39 + c)); // the significand 8 = 60 - 52
  print_bits(m, "m");
  //morlowt = m | lowt;

  g = (highs >> (38 + c)) & 1;
  hight = (highs << (55 - c)) != 0;
  print_variable(g, "guard bit");
  print_variable(hight, "sticky bit for highs");
  print_variable(lowt, "sticky bit for lows");
  // b denotes the bit used to round to nearest in RN(l) = (1.s1,..s23) (base 2)  + B^-23
  //b = g & (morlowt | hight);

  print_bits(b, "b");
  int32_t exint = std::bit_cast<uint64_t>(ex);
  int32_t eyint = std::bit_cast<uint64_t>(ey);
  int32_t cint = std::bit_cast<uint64_t>(c);
  dm1 = ((exint + eyint) - 1919) + cint; // biased exponent
  print_variable(dm1, "first dm1");
  if (dm1 >= 255) {
    dm1 = 255;
    
    m = 0;
    //m = (highs >> (39 + c));
    print_variable(m, "m when dm1=255");
    print_bits(m, "m bits when dm1=255");
    //morlowt = m | lowt;
    //b = g & (morlowt | hight);
  }
  else if (dm1<=0) {
    m = (highs >> (39 + c)) >> (1 - dm1);
    dm1 = 0;
    print_variable(m, "m when dm1 = 0");
    print_bits(m, "m bits when dm1 =0");
    
    morlowt = m | lowt;
    b = g & (morlowt | hight);
  }
  else  {
    m = (highs >> (39 + c));
    morlowt = m | lowt;
    b = g & (morlowt | hight);
  }
  print_variable(dm1, "dm1");
  print_bits(dm1, "dm1");

  uint32_t sr = (aa_bits ^ bb_bits) & 0x8000000000000000;
  print_variable(sr, "sign bit of result");

  uint32_t exp16 = sr | (dm1 << 23);
  print_bits(exp16, "exp16");
  //constexpr uint32_t FLOAT32_MANTISSA_MASK = 0b00000000011111111111111111111111;
 // uint32_t m2 = m & FLOAT32_MANTISSA_MASK;
 if (dm1 == 0) {
    uint32_t m2 = m;
    ///uint32_t m3 = m2 >> 1;
    ///uint32_t m4 = m3 >> 1;
    print_bits(m2, "mantissa mask");
    uint32_t result = (exp16 + m2) + b;
   
    print_bits(result, "result");
    float result32 = std::bit_cast<float>(result);
    print_variable(result32, "result32");
    return result32;
      
 } else {
   constexpr uint32_t FLOAT32_MANTISSA_MASK = 0b00000000011111111111111111111111;
  uint32_t m2 = m & FLOAT32_MANTISSA_MASK;
   print_bits(m2, "mantissa mask");
  uint32_t result = (exp16 + m2) + b;
  print_bits(result, "result");
  float result16 = std::bit_cast<float>(result);
  // std::memcpy(&result16, &result, sizeof(result16));  
  //result = *reinterpret_cast<_Float16*>(&result);
  print_variable(result16, "result16");
  return result16;
    
 }
}
 
 //print_bits(m3, "mantissa mask");
  //uint32_t result = (exp16 + m3) + b;
  //print_bits(result, "result");
  //float result16 = std::bit_cast<float>(result);
  // std::memcpy(&result16, &result, sizeof(result16));  
  //result = *reinterpret_cast<_Float16*>(&result);
  //print_variable(result16, "result16");
  //return result16;


int main() {
  //double x = 0x1.0p100;
  //double y = 0x1.5p100;
  //double x = 3.0;
  //double y = 5.0;
    
    //double x = 0x1.0p2;
    //double y = 0x1.0p-129;
    double x = 0x1.0p1;
    double y = 0x1.0p-131;
    print_variable(fmul(x, y), "fmul(x, y)");
    float number = x*y;
    print_variable(number, "number");
    
    return 0;
}
/*
  so 0| 1000 0010 | 1110 0000 0000 0000 0000 000 and 0 | 1001 0 | 1110 0000 00
 */
