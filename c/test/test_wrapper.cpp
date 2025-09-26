#include "vips_wrapper.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <chrono>
#include <string>

using namespace std::chrono;

/**
 * @brief Saves encoded image data to a file
 * @param data Pointer to the image data
 * @param size Size of the data in bytes
 * @param filename Output filename
 * @return true on success, false on failure
 */
bool save_encoded_image(const unsigned char* data, size_t size, const std::string& filename) {
    if (!data || size == 0) {
        std::cerr << "Error: Invalid data or size for saving" << std::endl;
        return false;
    }
    
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot create file: " << filename << std::endl;
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(data), size);
    
    if (!file.good()) {
        std::cerr << "Error: Failed to write data to file: " << filename << std::endl;
        return false;
    }
    
    file.close();
    std::cout << "   Successfully saved to: " << filename << std::endl;
    return true;
}

/**
 * @brief Converts ImageStatus to human-readable string
 * @param code The ImageStatus to convert
 * @return String representation of the status code
 */
const char* status_to_string(ImageStatus code) {
    switch (code) {
        case SUCCESS: return "SUCCESS";
        case VIPS_ERROR: return "VIPS_ERROR";
        case VIPS_INIT_FAILURE: return "VIPS_INIT_FAILURE";
        case VIPS_INVALID_HANDLE: return "VIPS_INVALID_HANDLE";
        case MEMORY_ALLOCATION_FAILURE: return "MEMORY_ALLOCATION_FAILURE";
        case IMAGE_INVALID_PATH: return "IMAGE_INVALID_PATH";
        case IMAGE_LOAD_FAILURE: return "IMAGE_LOAD_FAILURE";
        case IMAGE_INVALID_FORMAT: return "IMAGE_INVALID_FORMAT";
        case IMAGE_INVALID_DIMENSIONS: return "IMAGE_INVALID_DIMENSIONS";
        case IMAGE_INVALID_POSITION: return "IMAGE_INVALID_POSITION";
        case IMAGE_INVALID_BOUNDS: return "IMAGE_INVALID_BOUNDS";
        case IMAGE_SAVE_FAILURE: return "IMAGE_SAVE_FAILURE";
        case UNKNOWN_ERROR:
        default: return "UNKNOWN_ERROR";
    }
}

/**
 * @brief Prints image metadata in a formatted way
 * @param meta The ImageMeta structure to print
 */
void print_metadata(const ImageMeta& meta) {
    std::cout << "   Image Metadata:" << std::endl;
    std::cout << "     Dimensions: " << meta.width << "x" << meta.height << std::endl;
    std::cout << "     Channels: " << meta.channels << std::endl;
    std::cout << "     Format: " << meta.format << std::endl;
    std::cout << "     Colorspace: " << meta.colorspace << std::endl;
    std::cout << "     Density: " << meta.density_x << "x" << meta.density_y << " DPI" << std::endl;
    std::cout << "     File size: " << meta.file_size << " bytes" << std::endl;
}

/**
 * @brief Tests basic image loading and metadata extraction
 * @param input_path Path to the input image
 * @return true on success, false on failure
 */
bool test_image_loading(const char* input_path) {
    std::cout << "\n=== Test 1: Image Loading and Metadata ===" << std::endl;
    
    VImageHandle vimg = load_image(input_path);
    if (!vimg) {
        std::cout << "   Failed to load image from: " << input_path << std::endl;
        return false;
    }
    
    std::cout << "   Image loaded successfully" << std::endl;
    
    ImageMeta meta = extract_metadata(vimg);
    print_metadata(meta);
    
    free_vimage_handle(vimg);
    return true;
}

/**
 * @brief Tests image resizing functionality
 * @param input_path Path to the input image
 * @return true on success, false on failure
 */
bool test_image_resize(const char* input_path) {
    std::cout << "\n=== Test 2: Image Resize ===" << std::endl;
    
    VImageHandle vimg = load_image(input_path);
    if (!vimg) {
        std::cout << "   Failed to load image" << std::endl;
        return false;
    }
    
    ImageMeta original_meta = extract_metadata(vimg);
    std::cout << "   Original size: " << original_meta.width << "x" << original_meta.height << std::endl;
    
    // Test resize with aspect ratio maintained
    ImageResizeOptions resize_opts = {1, 800, 600}; // maintain_aspect=1, width=800, height=600
    ImageStatus result = resize_image(vimg, resize_opts);
    
    if (result == SUCCESS) {
        ImageMeta new_meta = extract_metadata(vimg);
        std::cout << "   Resize successful: " << new_meta.width << "x" << new_meta.height << std::endl;
        
        // Encode and save
        ImageBuffer jpeg = encode_to_jpeg(vimg, ImageEncodeJPEGOptions{85, 0});
        if (jpeg.data && jpeg.size > 0) {
            save_encoded_image(jpeg.data, jpeg.size, "./test/test_resized.jpg");
            free(jpeg.data); // Free the allocated buffer
        }
    } else {
        std::cout << "   Resize failed: " << status_to_string(result) << std::endl;
        free_vimage_handle(vimg);
        return false;
    }
    
    free_vimage_handle(vimg);
    return true;
}

/**
 * @brief Tests image cropping functionality
 * @param input_path Path to the input image
 * @return true on success, false on failure
 */
bool test_image_crop(const char* input_path) {
    std::cout << "\n=== Test 3: Image Crop ===" << std::endl;
    
    VImageHandle vimg = load_image(input_path);
    if (!vimg) {
        std::cout << "   Failed to load image" << std::endl;
        return false;
    }
    
    ImageMeta original_meta = extract_metadata(vimg);
    std::cout << "   Original size: " << original_meta.width << "x" << original_meta.height << std::endl;
    
    // Test crop - take center portion
    int crop_width = std::min(1000, original_meta.width - 100);
    int crop_height = std::min(800, original_meta.height - 100);
    ImageCropOptions crop_opts = {50, 50, crop_width, crop_height};
    
    ImageStatus result = crop_image(vimg, crop_opts);
    
    if (result == SUCCESS) {
        ImageMeta new_meta = extract_metadata(vimg);
        std::cout << "   Crop successful: " << new_meta.width << "x" << new_meta.height << std::endl;
        
        // Encode and save
        ImageBuffer jpeg = encode_to_jpeg(vimg, ImageEncodeJPEGOptions{90, 0});
        if (jpeg.data && jpeg.size > 0) {
            save_encoded_image(jpeg.data, jpeg.size, "./test/test_cropped.jpg");
            free(jpeg.data);
        }
    } else {
        std::cout << "   Crop failed: " << status_to_string(result) << std::endl;
        free_vimage_handle(vimg);
        return false;
    }
    
    free_vimage_handle(vimg);
    return true;
}

/**
 * @brief Tests image rotation functionality
 * @param input_path Path to the input image
 * @return true on success, false on failure
 */
bool test_image_rotate(const char* input_path) {
    std::cout << "\n=== Test 4: Image Rotate ===" << std::endl;
    
    VImageHandle vimg = load_image(input_path);
    if (!vimg) {
        std::cout << "   Failed to load image" << std::endl;
        return false;
    }
    
    ImageMeta original_meta = extract_metadata(vimg);
    std::cout << "   Original size: " << original_meta.width << "x" << original_meta.height << std::endl;
    
    // Test rotation
    ImageRotateOptions rotate_opts = {15.0}; // 15 degrees
    ImageStatus result = rotate_image(vimg, rotate_opts);
    
    if (result == SUCCESS) {
        ImageMeta new_meta = extract_metadata(vimg);
        std::cout << "   Rotation successful: " << new_meta.width << "x" << new_meta.height << std::endl;
        
        // Encode and save
        ImageBuffer jpeg = encode_to_jpeg(vimg, ImageEncodeJPEGOptions{85, 0});
        if (jpeg.data && jpeg.size > 0) {
            save_encoded_image(jpeg.data, jpeg.size, "./test/test_rotated.jpg");
            free(jpeg.data);
        }
    } else {
        std::cout << "   Rotation failed: " << status_to_string(result) << std::endl;
        free_vimage_handle(vimg);
        return false;
    }
    
    free_vimage_handle(vimg);
    return true;
}

/**
 * @brief Tests chained image operations
 * @param input_path Path to the input image
 * @return true on success, false on failure
 */
bool test_chained_operations(const char* input_path) {
    std::cout << "\n=== Test 5: Chained Operations ===" << std::endl;
    
    VImageHandle vimg = load_image(input_path);
    if (!vimg) {
        std::cout << "   Failed to load image" << std::endl;
        return false;
    }
    
    ImageMeta original_meta = extract_metadata(vimg);
    std::cout << "   Starting with: " << original_meta.width << "x" << original_meta.height << std::endl;
    
    // Chain: Resize -> Crop -> Rotate
    
    // 1. Resize
    ImageResizeOptions resize_opts = {1, 2734, 1538};
    ImageStatus result = resize_image(vimg, resize_opts);
    if (result != SUCCESS) {
        std::cout << "   Chained resize failed: " << result << std::endl;
        free_vimage_handle(vimg);
        return false;
    }
    
    ImageMeta after_resize = extract_metadata(vimg);
    std::cout << "   After resize: " << after_resize.width << "x" << after_resize.height << std::endl;
    
    // 2. Crop
    ImageCropOptions crop_opts = {100, 100, 800, 600};
    result = crop_image(vimg, crop_opts);
    if (result != SUCCESS) {
        std::cout << "   Chained crop failed: " << result << std::endl;
        free_vimage_handle(vimg);
        return false;
    }
    
    ImageMeta after_crop = extract_metadata(vimg);
    std::cout << "   After crop: " << after_crop.width << "x" << after_crop.height << std::endl;
    
    // 3. Rotate
    ImageRotateOptions rotate_opts = {10.0};
    result = rotate_image(vimg, rotate_opts);
    if (result != SUCCESS) {
        std::cout << "   Chained rotate failed: " << result << std::endl;
        free_vimage_handle(vimg);
        return false;
    }
    
    ImageMeta final_meta = extract_metadata(vimg);
    std::cout << "   After rotation: " << final_meta.width << "x" << final_meta.height << std::endl;
    
    // Encode final result
    ImageBuffer jpeg = encode_to_jpeg(vimg, ImageEncodeJPEGOptions{95, 1});
    if (jpeg.data && jpeg.size > 0) {
        std::cout << "   Final encoding: " << jpeg.size << " bytes" << std::endl;
        save_encoded_image(jpeg.data, jpeg.size, "./test/test_chained_operations.jpg");
        free(jpeg.data);
    } else {
        std::cout << "   Final encoding failed" << std::endl;
        free_vimage_handle(vimg);
        return false;
    }
    
    free_vimage_handle(vimg);
    return true;
}

/**
 * @brief Tests PNG encoding functionality
 * @param input_path Path to the input image
 * @return true on success, false on failure
 */
bool test_png_encoding(const char* input_path) {
    std::cout << "\n=== Test 6: PNG Encoding ===" << std::endl;
    
    VImageHandle vimg = load_image(input_path);
    if (!vimg) {
        std::cout << "   Failed to load image" << std::endl;
        return false;
    }
    
    // Resize to smaller size for PNG test
    ImageResizeOptions resize_opts = {1, 400, 300};
    ImageStatus result = resize_image(vimg, resize_opts);
    if (result != SUCCESS) {
        std::cout << "   Resize for PNG test failed" << std::endl;
        free_vimage_handle(vimg);
        return false;
    }
    
    // Encode as PNG
    ImageBuffer png = encode_to_png(vimg, ImageEncodePNGOptions{6, 0});
    if (png.data && png.size > 0) {
        std::cout << "   PNG encoding successful: " << png.size << " bytes" << std::endl;
        save_encoded_image(png.data, png.size, "./test/test_output.png");
        free(png.data);
    } else {
        std::cout << "   PNG encoding failed" << std::endl;
        free_vimage_handle(vimg);
        return false;
    }
    
    free_vimage_handle(vimg);
    return true;
}

int main() {
    std::cout << "=== Image SDK Comprehensive Test Suite ===" << std::endl;
    
    // Initialize the SDK
    int init_result = vips_wrapper_init();
    if (init_result != SUCCESS) {
        std::cerr << "Failed to initialize Image SDK (code: " << init_result << ")" << std::endl;
        return 1;
    }
    std::cout << "Image SDK initialized successfully" << std::endl;
    
    const char* input_image = "./test/test.jpg";
    auto start_time = high_resolution_clock::now();
    
    // Run all tests
    bool all_tests_passed = true;
    
    all_tests_passed &= test_image_loading(input_image);
    all_tests_passed &= test_image_resize(input_image);
    all_tests_passed &= test_image_crop(input_image);
    all_tests_passed &= test_image_rotate(input_image);
    all_tests_passed &= test_chained_operations(input_image);
    all_tests_passed &= test_png_encoding(input_image);
    
    auto total_time = high_resolution_clock::now() - start_time;
    auto duration_ms = duration_cast<milliseconds>(total_time).count();
    
    std::cout << "\n=== Test Suite Summary ===" << std::endl;
    std::cout << "Total execution time: " << duration_ms << "ms" << std::endl;
    
    if (all_tests_passed) {
        std::cout << "All tests PASSED!" << std::endl;
    } else {
        std::cout << "Some tests FAILED!" << std::endl;
    }
    
    // Cleanup
    vips_wrapper_cleanup();
    std::cout << "Image SDK cleanup completed" << std::endl;
    
    return all_tests_passed ? 0 : 1;
}
