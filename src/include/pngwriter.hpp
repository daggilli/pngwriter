#pragma once
#include <png.h>
#include <unistd.h>

#include <bit>
#include <concepts>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <optional>
#include <print>
#include <randomint.hpp>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace PNGWriter {
  enum class ImageMode : int { GREYSCALE = PNG_COLOR_TYPE_GRAY, RGB = PNG_COLOR_TYPE_RGB };

  namespace fs = std::filesystem;

  extern "C" using PngErrorHandler = void (*)(png_structp, png_const_charp);

  struct PngError : std::runtime_error {
    using std::runtime_error::runtime_error;
  };

  template <std::unsigned_integral T>
  class Writer {
   public:
    explicit Writer(PngErrorHandler handler)
        : errorHandler{handler}, pngPtr{nullptr}, infoPtr{nullptr}, bufReady{false} {}
    virtual ~Writer() {
      if (pngPtr) {
        png_destroy_write_struct(&pngPtr, infoPtr ? &infoPtr : nullptr);
      }
    }
    void create(const std::shared_ptr<T[]> data, std::size_t width, std::size_t height,
                const ImageMode mode) {
      if (bufReady || outputBuf.size()) {
        outputBuf.clear();
        bufReady = false;
      }

      if (pngPtr) {
        png_destroy_write_struct(&pngPtr, infoPtr ? &infoPtr : nullptr);
      }

      pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, errorHandler, nullptr);
      infoPtr = png_create_info_struct(pngPtr);
      if (!infoPtr) {
        png_destroy_write_struct(&pngPtr, nullptr);
        throw PngError("png_create_info_struct failed");
      }

      png_set_write_fn(
          pngPtr, &outputBuf,
          [](png_structp png_ptr, png_bytep dataOut, png_size_t len) {
            auto buffer = reinterpret_cast<std::vector<uint8_t> *>(png_get_io_ptr(png_ptr));
            buffer->insert(buffer->end(), dataOut, dataOut + len);
          },
          nullptr);

      auto constexpr depth = 8 * sizeof(T);

      auto colourType = static_cast<typename std::underlying_type<ImageMode>::type>(mode);

      png_set_IHDR(pngPtr, infoPtr, width, height, depth, colourType, PNG_INTERLACE_NONE,
                   PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

      png_write_info(pngPtr, infoPtr);

      if constexpr (std::endian::native == std::endian::little) png_set_swap(pngPtr);

      auto channels = png_get_channels(pngPtr, infoPtr);

      std::vector<png_bytep> rows(height);
      auto idata = data.get();
      auto stride = width * channels;
      for (auto y{0u}; y < height; y++) {
        rows[y] = reinterpret_cast<png_bytep>(const_cast<T *>(idata));
        idata += stride;
      }

      png_write_image(pngPtr, rows.data());
      png_write_end(pngPtr, infoPtr);

      bufReady = true;
      png_destroy_write_struct(&pngPtr, &infoPtr);

      pngPtr = nullptr;
      infoPtr = nullptr;
    }

    void write(const std::string &filename) {
      if (!bufReady || outputBuf.size() == 0) {
        throw PngError("output buffer not ready");
      }
      auto filepath = fs::weakly_canonical(fs::absolute(filename));

      std::ofstream outputfile(filepath.string(), std::ofstream::binary);
      outputfile.write(std::bit_cast<char *>(outputBuf.data()), outputBuf.size());
    }

    const std::optional<std::vector<uint8_t>> imagedata() const {
      if (bufReady && outputBuf.size()) return outputBuf;
      return std::nullopt;
    }

   private:
    PngErrorHandler errorHandler;
    png_structp pngPtr;
    png_infop infoPtr;
    std::vector<uint8_t> outputBuf;
    bool bufReady;
  };
};  // namespace PNGWriter
