# Lyrah_Image_Filters
# C BMP Image Filter 🎨

A command-line image processing tool written in C that applies various visual filters to 24-bit uncompressed BMP images. 

This project demonstrates low-level file processing, dynamic memory management, and the practical application of 2D convolution matrices (kernels) to manipulate pixel data byte-by-byte.

## Features

This program parses BMP headers and modifies the pixel array to apply the following filters:
* **Grayscale:** Averages RGB values to convert the image to black and white.
* **Invert:** Reverses the RGB values to create a negative effect.
* **Reflect:** Horizontally mirrors the image.
* **Blur:** Applies a 3x3 box blur kernel (applied 15 times for a pronounced effect) to soften the image.
* **Edge Detection:** Uses a 3x3 convolution matrix to highlight visual boundaries and create a dark, outlined effect.
* **Sharpen:** Enhances edges and details using a custom sharpen kernel.

## Visual Examples

| Original | Edge Detection | Reflect |
| :---: | :---: | :---: |
| <img src="butterfly.bmp" width="250"/> | <img src="butterflyblur.bmp" width="250"/> | <img src="butterflygrayscale.bmp" width="250"/> |