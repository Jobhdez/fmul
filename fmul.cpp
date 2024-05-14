#include <cstdint>
#include <iostream>
#include <bitset>
#include <bit>

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
void print_float16(_Float16 value, const char* name) {
  uint16_t bits = std::bit_cast<uint16_t>(value);
  std::bitset<16> b = bits;
  std::cout << name << " value is      : " << value << std::endl;
  std::cout << name << " bit pattern is: " << b << std::endl;
}


float fmul(float aa, float bb) {
  uint32_t aa_bits = std::bit_cast<uint32_t>(aa);
  uint32_t bb_bits = std::bit_cast<uint32_t>(bb); 

  uint32_t absx, ex;
  absx = aa_bits & 0x7FFFFFFF;
  ex = absx >> 23; // exponent of x

  std::cout << "exp_x     : " << ex << std::endl;
  uint32_t absy, ey;
  absy = bb_bits & 0x7FFFFFFF;
  ey = absy >> 23; //exponent of y

  std::cout << "exp_y     : " << ey << std::endl;
  uint32_t nx, ny;
  nx = absx >= 0x800000; // is normal bit for x
  ny = absy >= 0x800000;// is normal bit for y

  std::cout << "normal bit of x : " << nx << std::endl;
  std::cout << "normal bit of y : " << ny << std::endl;
  uint32_t mx, my, lambday, lambdax;
  mx = maxu(nlz(absx), 8); // nlz(absx) = leading zeros of x and 8 is the bit width of the exponent bias
  
  std::cout << "the max between the leading zeros of x and the bit width ofthe  exponent wise" << mx << std::endl;
  
  lambdax = mx - 8;  //leading zeros of significand x
  
  std::cout << "leading zeros of significand x: " << lambdax << std::endl;
  std::cout << "bit pattern of leading zeros of significand x: " << std::bitset<32>(lambdax) << std::endl;
  
  my = maxu(nlz(absy), 8);
  lambday = my - 8; // PP

    std::cout << "the max between the leading zeros of y and the bit width ofthe  exponent wise" << my << std::endl;
  std::cout << "leading zeros of significand y: " << lambday << std::endl;
  std::cout << "bit pattern of leading zeros of significand y: " << std::bitset<32>(lambday) << std::endl;
  
  uint32_t dm1;
  uint32_t mpx, mpy, highs, m, lows, c, morlowt;
  uint32_t g, hight, lowt, b;
  
  mpx = (aa_bits << mx) | 0x80000000; // normalize significand x
  mpy = (bb_bits << my) |  0x80000000; // normalize significand y
  
  std::cout << "normalized significand_x : " << std::bitset<32>(mpx) << std::endl;
  std::cout << "normalize significand_x value: " << mpx << std::endl;
  std::cout << "normalized significand_y : " << std::bitset<32>(mpy) << std::endl;
  
  std::cout << "normalize significand_y value: " << mpy << std::endl;
  
  highs = mul(mpx, mpy);
  c = highs >= 0x80000000; // c = 0 if m'xm'y in [1,2) else 1 if m'xm'y in [2,4)
  std::cout << "c: " << c << std::endl;
  std::cout << "bit pattern for c: " << std::bitset<32>(c) << std::endl;
  
  lows = mullow(mpx, mpy);

  // the product of two 32 bit is a 64 bit number. this 64 bit can be sttored in two 32 bit variable; hence the highs and lows variable
  std::cout << "upper half of the product of the mantissas :"<< std::bitset<32>(highs) << std::endl;
  std::cout << "upper half value of the product of the mantissas : " << highs << std::endl;

    std::cout << "lower half of the product of the mantissas :"<< std::bitset<32>(lows) << std::endl;
  std::cout << "lower half value of the product of the mantissas : " << lows << std::endl;
  
  lowt = (lows != 0);
  m = highs >> (7 + c);
  
  std::cout << "m, the significand: " << m << std::endl;
  std::cout << " the bit pattern for m: " << std::bitset<32>(m) <<std::endl;
  morlowt = m | lowt;

  g = (highs >> (6 + c)) & 1;
  hight = (highs << (26 - c)) != 0;
  std::cout << "guard bit for rounding : " << std::bitset<32>(g) << std::endl;
  std::cout << "guard bit value : " << g << std::endl;
  std::cout << "sticky bit for highs " << std::bitset<32>(hight) << std::endl;
  std::cout << "sticky bit for lows " << std::bitset<32>(lowt) << std::endl;
  // b denotes the bit used to round to nearest in RN(l) = (1.s1,..s23) (base 2)  + B^-23
  b = g & (morlowt | hight);
  std::cout << "bit b ie round to nearest: " << b << std::endl;
  std::cout << "bit pattern for b: " << std::bitset<32>(b) << std::endl;
  dm1 = (((ex - nx) + (ey - ny)) - ((mx + my) + 110)) + c; // biased exponent
  std::cout << "exponent of result: " << std::bitset<32>(dm1) << std::endl;
  std::cout << "value of exponent: " << dm1 << std::endl;
  uint32_t sr = (aa_bits ^ bb_bits) & 0x80000000;
  std::cout << "sign bit of result : " << sr << std::endl;
  uint32_t result = ((sr | (dm1 << 23)) + m) + b;
  std::cout << "product of mantissas: " << std::bitset<32>(result) << std::endl;

  return result;
}

int main() {
    float x = 3.0;
    float y = 5.0;

    std::cout << "result of fmul32 " << std::bitset<32>(fmul(x,y)) << std::endl;
    float result = 15.0;
    std::cout << "bits of 15f " << std::bitset<32>(result) << std::endl;

}
/*
  so 0| 1000 0010 | 1110 0000 0000 0000 0000 000 and 0 | 1001 0 | 1110 0000 00
 */
