#include <png.h>
#include <unistd.h>

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <pngwriter.hpp>
#include <print>
#include <randomint.hpp>
#include <stdexcept>
#include <string>
#include <vector>

extern "C" void pngErrorFunc(png_structp pngPtr, png_const_charp msg);

int main() {
  std::size_t w = 100;
  std::size_t h = 100;

  auto pngbuf = std::make_shared_for_overwrite<uint16_t[]>(w * h);

  int16_t ix{0};
  int16_t iy{0};
  auto c{0u};
  for (uint16_t i{0u}; i < w * h; i++) {
    ix = (i % w) - w / 2;
    iy = (i / h) - h / 2;
    c++;
    pngbuf[i] = static_cast<uint16_t>(std::sqrt(ix * ix + iy * iy) * 600);
  }

  PNGWriter::Writer<uint16_t> pw(pngErrorFunc);

  pw.create(pngbuf, w, h, PNGWriter::ImageMode::GREYSCALE);
  pw.write("test_x.png");

  w = 108;
  h = 108;
  auto rgbbuf = std::make_shared_for_overwrite<uint8_t[]>(w * h * 3);

  RandomInt::RandomUint8Generator rg;

  for (uint16_t i{0u}; i < w * h * 3; i++) {
    ix = i % 9;
    uint8_t c = ix == 0 || ix == 4 || ix == 8 ? 255 : 0;

    rgbbuf[i] = c;
  }

  PNGWriter::Writer<uint8_t> rgbpw(pngErrorFunc);

  rgbpw.create(rgbbuf, 108, 108, PNGWriter::ImageMode::RGB);
  rgbpw.write("test_rgb.png");

  return 0;
}

extern "C" void pngErrorFunc(png_structp pngPtr, png_const_charp msg) {
  throw PNGWriter::PngError(msg ? msg : "libpng error");
}
