#include <algorithm>

class RgbColor {
public:
  unsigned char r;
  unsigned char g;
  unsigned char b;
  RgbColor(){};
  void setRgb(unsigned char r, unsigned char g, unsigned char b) {
    this->r = r;
    this->g = g;
    this->b = b;
  }
};

typedef struct HsbColor {
  double h;
  double s;
  double b;
} HsbColor;

static double k(double n, double h) { return int(n + h / 6) % 6; }
static double f(double n, double b, double s, double h) {
  return b * (1 - s * std::max(0.0d,
                               std::min(std::min(k(n, h), 1.0d), 4 - k(n, h))));
}

RgbColor HsbToRgb(HsbColor c) {
  c.s /= 100;
  c.b /= 100;
  RgbColor answer;
  answer.r = 255 * f(5, c.b, c.s, c.h);
  answer.g = 255 * f(3, c.b, c.s, c.h);
  answer.b = 255 * f(1, c.b, c.s, c.h);
  return answer;
}

typedef struct HsvColor {
  unsigned char h;
  unsigned char s;
  unsigned char v;
} HsvColor;

RgbColor HsvToRgb(HsvColor hsv) {
  RgbColor rgb;
  unsigned char region, remainder, p, q, t;

  if (hsv.s == 0) {
    rgb.r = hsv.v;
    rgb.g = hsv.v;
    rgb.b = hsv.v;
    return rgb;
  }

  region = hsv.h / 43;
  remainder = (hsv.h - (region * 43)) * 6;

  p = (hsv.v * (255 - hsv.s)) >> 8;
  q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
  t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

  switch (region) {
  case 0:
    rgb.r = hsv.v;
    rgb.g = t;
    rgb.b = p;
    break;
  case 1:
    rgb.r = q;
    rgb.g = hsv.v;
    rgb.b = p;
    break;
  case 2:
    rgb.r = p;
    rgb.g = hsv.v;
    rgb.b = t;
    break;
  case 3:
    rgb.r = p;
    rgb.g = q;
    rgb.b = hsv.v;
    break;
  case 4:
    rgb.r = t;
    rgb.g = p;
    rgb.b = hsv.v;
    break;
  default:
    rgb.r = hsv.v;
    rgb.g = p;
    rgb.b = q;
    break;
  }

  return rgb;
}

// HsvColor RgbToHsv(RgbColor rgb) {
//   HsvColor hsv;
//   unsigned char rgbMin, rgbMax;

//   rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b)
//                          : (rgb.g < rgb.b ? rgb.g : rgb.b);
//   rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b)
//                          : (rgb.g > rgb.b ? rgb.g : rgb.b);

//   hsv.v = rgbMax;
//   if (hsv.v == 0) {
//     hsv.h = 0;
//     hsv.s = 0;
//     return hsv;
//   }

//   hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
//   if (hsv.s == 0) {
//     hsv.h = 0;
//     return hsv;
//   }

//   if (rgbMax == rgb.r)
//     hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
//   else if (rgbMax == rgb.g)
//     hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
//   else
//     hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

//   return hsv;
// }