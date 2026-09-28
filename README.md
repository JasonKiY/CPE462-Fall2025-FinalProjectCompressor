# Image Compression Techniques using OpenCV in C++ 

This repository contains two different C++ files, both employing different compression techniques to reduce file size. One uses DFT (Discrete Fourier Transform) compression to reduce file size dramatically at the cost of image quality and the other uses Huffman Encoding to preserve the image entirely at the cost of size reduction and file usability.

## Algorithms
### <ins>Lossy Compression (DFT)
The lossy compressor reduces file size at the cost of image quality using a Discrete Fourier Transform (DFT) algorithm. The process works as follows:
1. The image's colors are converted to the YCbCr color space for separate luminance (Y) and chrominance values (Cr, Cb).
2. These elements are given three separate channels (Y, Cr, Cb).
3. An fftshift, using a complex matrix, is applied to each individual channel for compression where the luminance (Y) is compressed at a higher intensity than the chrominance (Cr, Cb).
4. All channels (Y, Cr, Cb) are recombined and the output image is written to a file.

The level of compression is determined by the "keepFraction" variable where a lower value increases the severity of the compression resulting in a smaller file, but worse image. Results from testing a 4K Image:

| keepFraction | File Size | Reduction |
|---|---|---|
| Original | 920 KB | — |
| 0.9 | 728 KB | 20.9% |
| 0.5 | 338 KB | 63.3% |
| 0.3 | 259 KB | 71.8% |
| 0.01 | 130 KB | 85.9% |

OpenCV is **required** for matrix operations and open I/O.

### <ins>Lossless Compression (Huffman Encoding)
The lossless compressor reduces file size by converting the image data into Huffman codes which can be fully decoded back into the original image. The process works as follows:
**Compression:**
1. Image data is scanned for byte frequencies which are then used to build a Huffman tree.
2. Image data is encoded into bits with a frequency map saved as the header within the compressed file.
3. Padding is added to the end of the last byte to complete ensure byte alignment.
4. Tree is cleaned up to free memory, and the compressed file is written (.huf). 

**Decompression:**
1. The saved header is read to reconstruct the frequency map and rebuild the Huffman tree.
2. The padding byte is read to determine how many bits of the last byte to disregard.
3. The data is decoded through Huffman tree traversal where each leaf node character is output and formatted back to the original image.

Results from testing on a 4K image:
| File | Size |
|---|---|
| Original (p3wallpaper.jpg) | 920 KB |
| Compressed (compressed.huf) | 914 KB |
| Reduction | ~0.7% |

> **Note:** Lossless compression yields minimal size reduction on already-compressed formats like JPEG. Results would be more significant on raw/uncompressed image formats.

## How to Run

### Dependencies
- C++ compiler (MSVC or g++).=
- OpenCV (Lossy only) —> set up as in Visual Studio 2022 with Debug lib

### Lossy (CPE462Lossy.cpp)
1. Set up OpenCV in Visual Studio 2022 and build the solution in Debug mode.
2. Open a terminal and navigate to `...\repos\{Project Name}\x64\Debug`.
3. Place your input `.jpg` image in that folder.
4. Run `.\{ProjectName}` and enter your image name and keepFraction value (0.01–1.0).
5. Find the compressed output image in the same Debug folder.

### Lossless (CPE462Lossless.cpp)
1. Open a terminal and navigate to the directory containing the .cpp file.
2. Compile with: `g++ CPE462ProjectLossless.cpp -o compressor`.
3. Run `.\compressor` and choose compression or decompression.
4. Enter the full path to your input image (e.g. `"C:\images\photo.jpg"`).
5. Find the output `.huf` or decompressed image in the same directory.

# Course Context
Final Project for CPE-462 (Image Processing & Coding) at Stevens Institute of Technology for Fall 2025.
