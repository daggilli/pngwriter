# PNGWRITER

### Minimal header-only library to make a dataset into a PNG image

#### Requirments

The `libpng` library and header files are needed. Your package manager has them if they're not already installed.

#### Usage

There's an example in `src/example/example.cpp`.

```cpp
#include <pngwriter.h>
```

Make a PNGWriter::Writer obhect

```cpp
PNGWriter::Writer<uint16_t> writer(pngErrorFunc);
```

The error function is of type `PNGWriter::PngErrorHanfdler` and has C linkage. It is called by the libpng library on error. It takes a pointer to the libpng write structure and a pointer to the info structure.

Fill a block of memory (as a `std::shared_ptr`) with the pixel data. This can be greyscale or RGB. If RGB, each pixel needs three elements.

```cpp
auto buffer = std::make_shared_for_overwrite<uint8_t[]>(width * height * 3);
// fill the buffer somehow...
```

Create the PNG image data and write it to a file.

```cpp
writer.create(buffer, width, height, PNGWriter::ImageMode::RGB);
writer.write("rgb.png");
```
