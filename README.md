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

OpenCV is **required** for matrix operations and open I/O.

### <ins>Lossless Compression (Huffman Encoding)
The lossless compressor 
