#include <cuda_runtime.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif
#include <npp.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// downloaded stb_image.h from https://github.com/nothings/stb/blob/master/stb_image.h
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// downloaded stb_image_write.h from https://github.com/nothings/stb/blob/master/stb_image_write.h
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Structure to hold image data
struct Image {
  int width;
  int height;
  int channels;
  std::vector<unsigned char> data;
  std::string name;
  std::string extension;
};

// Function to load an image from a file using stb_image
Image loadImage(const std::string& filepath) {
  Image img;
  int width, height, channels;
  // Force 3 channels (RGB) for consistency with the NPP function
  unsigned char* loaded_data = stbi_load(filepath.c_str(), &width, &height, &channels, 3);
  if (loaded_data == nullptr) {
    throw std::runtime_error("Cannot open or read image file: " + filepath + ". Reason: " + stbi_failure_reason());
  }

  img.width = width;
  img.height = height;
  img.channels = 3;
  img.data.assign(loaded_data, loaded_data + (width * height * 3));

  stbi_image_free(loaded_data);

  // Extract file name and extension
  size_t last_slash_idx = filepath.find_last_of("\\/");
  std::string filename_with_ext = (std::string::npos != last_slash_idx) ? filepath.substr(last_slash_idx + 1) : filepath;

  size_t dot_idx = filename_with_ext.rfind('.');
  if (std::string::npos != dot_idx) {
    img.name = filename_with_ext.substr(0, dot_idx);
    img.extension = filename_with_ext.substr(dot_idx);
  } else {
    img.name = filename_with_ext;
    img.extension = "";  // No extension
  }

  return img;
}

// Function to save an image to a file using stb_image_write
void saveImage(const std::string& filename, int width, int height, const unsigned char* data) {
  size_t dot_idx = filename.rfind('.');
  if (dot_idx == std::string::npos) {
    throw std::runtime_error("Output filename has no extension: " + filename);
  }
  std::string ext = filename.substr(dot_idx);
  int channels = 1;  // Grayscale

  int success = 0;
  if (ext == ".png") {
    success = stbi_write_png(filename.c_str(), width, height, channels, data, width * channels);
  } else if (ext == ".jpg" || ext == ".jpeg") {
    success = stbi_write_jpg(filename.c_str(), width, height, channels, data, 95);  // Quality 95
  } else if (ext == ".bmp") {
    success = stbi_write_bmp(filename.c_str(), width, height, channels, data);
  } else {
    std::cerr << "Warning: Unsupported output format '" << ext << "'. Saving as .png instead." << std::endl;
    std::string new_filename = filename.substr(0, dot_idx) + ".png";
    success = stbi_write_png(new_filename.c_str(), width, height, channels, data, width * channels);
  }

  if (!success) {
    throw std::runtime_error("Failed to save image: " + filename);
  }
}

int main() {
  const std::string input_dir = "input_images";
  const std::string output_dir = "output_images";

  std::vector<std::string> image_files;

// for windows
#ifdef _WIN32
  WIN32_FIND_DATAA find_data;
  HANDLE h_find = FindFirstFileA((input_dir + "\\*").c_str(), &find_data);
  if (h_find != INVALID_HANDLE_VALUE) {
    do {
      std::string filename = find_data.cFileName;
      if (filename == "." || filename == "..") {
        continue;
      }
      if (filename.length() > 4) {
        std::string ext = filename.substr(filename.length() - 4);
        if (ext == ".png" || ext == ".jpg") {
          image_files.push_back(input_dir + "\\" + filename);
        } else if (filename.length() > 5 && filename.substr(filename.length() - 5) == ".jpeg") {
          image_files.push_back(input_dir + "\\" + filename);
        }
      }
    } while (FindNextFileA(h_find, &find_data) != 0);
    FindClose(h_find);
  } else {
    std::cerr << "Error: Could not open input directory '" << input_dir << "'." << std::endl;
    std::cerr << "Please create it and place image files (png, jpg, jpeg) inside." << std::endl;
    return EXIT_FAILURE;
  }
// below runs if system is linux
#else
  DIR* dir;
  struct dirent* ent;
  // Find all image files in the input directory
  if ((dir = opendir(input_dir.c_str())) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      std::string filename = ent->d_name;
      if (filename.length() > 4) {
        std::string ext = filename.substr(filename.length() - 4);
        if (ext == ".png" || ext == ".jpg") {
          image_files.push_back(input_dir + "/" + filename);
        } else if (filename.length() > 5 && filename.substr(filename.length() - 5) == ".jpeg") {
          image_files.push_back(input_dir + "/" + filename);
        }
      }
    }
    closedir(dir);
  } else {
    std::cerr << "Error: Could not open input directory '" << input_dir << "'." << std::endl;
    std::cerr << "Please create it and place image files (png, jpg, jpeg) inside." << std::endl;
    return EXIT_FAILURE;
  }
#endif

  if (image_files.empty()) {
    std::cout << "No .png, .jpg, or .jpeg files found in " << input_dir << std::endl;
    return 0;
  }

  std::cout << "Found " << image_files.size() << " images to process." << std::endl;

  for (const auto& file_path : image_files) {
    try {
      std::cout << "Processing: " << file_path << std::endl;

      // 1. Load image on host
      Image host_rgb_image = loadImage(file_path);
      int width = host_rgb_image.width;
      int height = host_rgb_image.height;
      int rgb_data_size = width * height * 3 * sizeof(unsigned char);
      int gray_data_size = width * height * 1 * sizeof(unsigned char);

      // 2. Allocate memory on device
      Npp8u* device_rgb_ptr;
      Npp8u* device_gray_ptr;
      cudaMalloc((void**)&device_rgb_ptr, rgb_data_size);
      cudaMalloc((void**)&device_gray_ptr, gray_data_size);

      // 3. Copy image from host to device
      cudaMemcpy(device_rgb_ptr, host_rgb_image.data.data(), rgb_data_size, cudaMemcpyHostToDevice);

      // 4. Perform RGB to Grayscale conversion using NPP
      NppiSize roi_size = {width, height};
      int rgb_step = width * 3;
      int gray_step = width * 1;

      NppStatus status = nppiRGBToGray_8u_C3C1R(device_rgb_ptr, rgb_step, device_gray_ptr, gray_step, roi_size);
      if (status != NPP_SUCCESS) {
        std::cerr << "NPP Error: Failed to convert to grayscale for image " << file_path << std::endl;
        cudaFree(device_rgb_ptr);
        cudaFree(device_gray_ptr);
        continue;  // Skip to next image
      }

      // 5. Copy result from device to host
      std::vector<unsigned char> host_gray_data(width * height);
      cudaMemcpy(host_gray_data.data(), device_gray_ptr, gray_data_size, cudaMemcpyDeviceToHost);

      // 6. Save the grayscale image
#ifdef _WIN32  // for windows
      std::string output_filename = output_dir + "\\" + host_rgb_image.name + "_grayscale" + host_rgb_image.extension;
#else  // for linux
      std::string output_filename = output_dir + "/" + host_rgb_image.name + "_grayscale" + host_rgb_image.extension;
#endif
      saveImage(output_filename, width, height, host_gray_data.data());
      std::cout << "Saved to " << output_filename << std::endl;

      // 7. Free device memory
      cudaFree(device_rgb_ptr);
      cudaFree(device_gray_ptr);

    } catch (const std::exception& e) {
      std::cerr << "An error occurred while processing " << file_path << ": " << e.what() << std::endl;
    }
  }

  std::cout << "Batch processing complete." << std::endl;
  return 0;
}