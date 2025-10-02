# CUDA NPP Image Batch Processing

This project is a demonstration of using the NVIDIA Performance Primitives (NPP) library with CUDA to perform common image processing tasks on a batch of images. This version supports standard image formats like JPG, PNG, and JPEG.

The program is designed to:

- Process all `.jpg`, `.jpeg`, and `.png` images found in a specified input directory.
- Convert them to grayscale.
- Save the resulting grayscale images to an output directory, preserving the original file format.

## Project Goal

The primary goal is to showcase a realistic image processing pipeline on a collection of data using NPP with CUDA. This example focuses on loading standard image formats, converting them to grayscale on the GPU, and saving them back.

## Dependencies

To build and run this project, you will need the following:

- **NVIDIA CUDA Toolkit**: This project relies on the CUDA platform. Make sure you have the CUDA Toolkit installed, which includes the NVIDIA CUDA Compiler (NVCC) and the necessary libraries. You can download it from the [NVIDIA Developer website](https://developer.nvidia.com/cuda-toolkit).
- **NVIDIA Performance Primitives (NPP)**: NPP is included with the CUDA Toolkit, so no separate installation is necessary.
- **stb Libraries**: This project uses `stb_image.h` for loading images and `stb_image_write.h` for saving them. These are single-header C/C++ libraries that need to be placed in the project directory.
  - Download `stb_image.h` from [here](https://github.com/nothings/stb/blob/master/stb_image.h).
  - Download `stb_image_write.h` from [here](https://github.com/nothings/stb/blob/master/stb_image_write.h).

## How It Works

This project integrates the popular `stb_image` and `stb_image_write` single-header libraries to handle image file I/O.

1.  **Image Loading**: The program scans the `input_images` directory for any files with the `.jpg`, `.jpeg`, or `.png` extension. It uses `stb_image` to load and decode each image into a raw RGB pixel buffer in memory.
2.  **Memory Allocation**: For each image, it allocates memory on the GPU (device) for both the input RGB image and the output grayscale image.
3.  **Data Transfer**: The raw RGB image data is copied from the host to the device.
4.  **Grayscale Conversion**: The `nppiRGBToGray_8u_C3C1R` function from the NPP library is called to perform the conversion on the GPU. This is a highly optimized function for this specific task.
5.  **Data Retrieval**: The resulting grayscale image data is copied back from the device to the host.
6.  **Image Saving**: The grayscale data is saved as a new image in the `output_images` directory using the `stb_image_write` library. The new file will have a `_grayscale` suffix and retain the original file format (e.g., `.jpg`, `.png`).
7.  **Memory Cleanup**: All allocated host and device memory is freed before the program exits.

## Building the Project

A `Makefile` is provided to simplify the compilation process.

1.  Open your terminal.
2.  Navigate to the project's root directory.
3.  Run the `make` command:
    ```bash
    make
    ```
    This will compile the `main.cpp` file using `nvcc` and create an executable named `batch_grayscale.exe`.

## Running the Program

1.  **Create Directories**: Before running, make sure you have `input_images` and `output_images` directories in the project folder.
    ```powershell
    mkdir input_images, output_images
    ```
2.  **Add Images**: Place one or more `.jpg`, `.jpeg`, or `.png` image files into the `input_images` directory.
3.  **Execute**: Run the compiled program from the terminal:
    ```powershell
    .\batch_grayscale.exe
    ```
4.  **Check Output**: The program will process each image and save the grayscale versions with a `_grayscale` suffix in the `output_images` directory, preserving the original file extension.

## Cleaning Up

To remove the compiled executable and any object files, you can use the `make clean` command:

```bash
make clean
```
