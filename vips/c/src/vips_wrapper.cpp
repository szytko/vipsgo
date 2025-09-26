#include "vips_wrapper.h"
#include <vips/vips8>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <memory>
#include <stdexcept>

using namespace vips;

// Define a custom deleter for VImageHandle to ensure proper cleanup
struct VImageCleanup {
    void operator()(VImage* img) const {
        delete img;
    }
};

// Type alias for a smart pointer to VImage
using VImagePtr = std::unique_ptr<VImage, VImageCleanup>;

extern "C" {

/**
 * @brief Initializes the VIPS library. Must be called before any other VIPS operations.
 * @return SUCCESS on successful initialization, VIPS_INIT_FAILURE otherwise.
 */
int vips_wrapper_init() {
    // Initialize VIPS with the program name
    if (VIPS_INIT("vips_wrapper")) {
        // Log VIPS error and exit if initialization fails
        vips_error_exit(nullptr);
        return VIPS_INIT_FAILURE;
    }
    return SUCCESS;
}

/**
 * @brief Cleans up VIPS resources. Should be called when image processing is complete.
 */
void vips_wrapper_cleanup() {
    vips_shutdown();
}

/**
 * @brief Loads an image from a file and returns a VImage handle.
 *
 * @param input_path The path to the image file to load.
 * @return A VImageHandle on success, nullptr on failure. The caller is responsible for freeing
 *         the handle using `free_vimage_handle`.
 * @throws std::runtime_error On failure to load the image or other unexpected errors.
 */
VImageHandle load_image(const char* input_path) {
    if (!input_path || std::strlen(input_path) == 0) {
        std::cerr << "Error: Input path for image loading is null or empty." << std::endl;
        return nullptr;
    }

    try {
        // Create a new VImage instance from the file
        VImage* img = new VImage(VImage::new_from_file(input_path));
        return static_cast<VImageHandle>(img);
    } catch (const VError &e) {
        std::cerr << "VIPS Error during image loading: " << e.what() << std::endl;
        return nullptr;
    } catch (const std::bad_alloc &e) {
        std::cerr << "Memory allocation error during image loading: " << e.what() << std::endl;
        return nullptr;
    } catch (const std::exception &e) {
        std::cerr << "Standard exception during image loading: " << e.what() << std::endl;
        return nullptr;
    } catch (...) {
        std::cerr << "Unknown error occurred while loading image." << std::endl;
        return nullptr;
    }
}

/**
 * @brief Loads an image from a byte buffer and returns a VImage handle.
 *
 * @param data Pointer to the image data bytes.
 * @param size Size of the image data in bytes.
 * @return A VImageHandle on success, nullptr on failure. The caller is responsible for freeing
 *         the handle using `free_vimage_handle`.
 */
VImageHandle load_image_from_bytes(const unsigned char* data, size_t size) {
    if (!data || size == 0) {
        std::cerr << "Error: Image data is null or empty." << std::endl;
        return nullptr;
    }

    try {
        // Create a new VImage instance from the byte buffer
        VImage* img = new VImage(VImage::new_from_buffer(data, size, ""));
        return static_cast<VImageHandle>(img);
    } catch (const VError &e) {
        std::cerr << "VIPS Error during image loading from bytes: " << e.what() << std::endl;
        return nullptr;
    } catch (const std::bad_alloc &e) {
        std::cerr << "Memory allocation error during image loading from bytes: " << e.what() << std::endl;
        return nullptr;
    } catch (const std::exception &e) {
        std::cerr << "Standard exception during image loading from bytes: " << e.what() << std::endl;
        return nullptr;
    } catch (...) {
        std::cerr << "Unknown error occurred while loading image from bytes." << std::endl;
        return nullptr;
    }
}

/**
 * @brief Frees the memory associated with a VImageHandle.
 * @param handle The VImageHandle to free.
 */
void free_vimage_handle(VImageHandle handle) {
    if (handle) {
        VImagePtr img_ptr(static_cast<VImage*>(handle)); // Use smart pointer for deletion
        // img_ptr will be deleted automatically when it goes out of scope
    }
}

/**
 * @brief Frees the data pointer within an ImageBuffer.
 * This should be used to free buffers returned by encode_to_jpeg/png.
 * @param buffer The ImageBuffer whose data pointer needs to be freed.
 */
void free_image_buffer(ImageBuffer buffer) {
    if (buffer.data) {
        g_free(buffer.data);
        buffer.data = nullptr; // Clear pointer after freeing
        buffer.size = 0;
    }
}

/**
 * @brief Resizes an image according to the specified options.
 * This function modifies the VImage associated with the handle in-place.
 *
 * @param handle The VImageHandle to be resized.
 * @param options The resize options including dimensions and aspect ratio maintenance.
 * @return SUCCESS on success, or an appropriate error code on failure.
 */
ImageStatus resize_image(VImageHandle handle, ImageResizeOptions options) {
    if (!handle) {
        std::cerr << "Error: Invalid VImage handle for resize operation." << std::endl;
        return VIPS_INVALID_HANDLE;
    }
    if (options.width <= 0 && options.height <= 0) {
        std::cerr << "Error: Invalid dimensions provided for resize (width and/or height must be positive)." << std::endl;
        return IMAGE_INVALID_DIMENSIONS;
    }

    try {
        VImage* img = static_cast<VImage*>(handle);
        VImage resized_img;

        if (options.maintain_aspect) {
            double scale = 1.0;
            // Calculate scale factor while maintaining aspect ratio
            if (options.width > 0 && options.height > 0) {
                scale = std::min(static_cast<double>(options.width) / img->width(),
                                 static_cast<double>(options.height) / img->height());
            } else if (options.width > 0) {
                scale = static_cast<double>(options.width) / img->width();
            } else { // options.height > 0
                scale = static_cast<double>(options.height) / img->height();
            }
            resized_img = img->resize(scale, VImage::option()->set("kernel", VIPS_KERNEL_LANCZOS3));
        } else {
            // Calculate independent scales for width and height
            double scale_x = (options.width > 0) ? static_cast<double>(options.width) / img->width() : 1.0;
            double scale_y = (options.height > 0) ? static_cast<double>(options.height) / img->height() : 1.0;

            // If one dimension is not specified, use the scale of the other to maintain proportion if no aspect constraint
            if (options.width <= 0) {
                scale_x = scale_y;
            } else if (options.height <= 0) {
                scale_y = scale_x;
            }

            resized_img = img->resize(scale_x, VImage::option()
                ->set("kernel", VIPS_KERNEL_LANCZOS3)
                ->set("vscale", scale_y));
        }

        // Overwrite the original VImage object with the resized one
        *img = resized_img;
        return SUCCESS;
    } catch (const VError &e) {
        std::cerr << "VIPS Error during resize_image: " << e.what() << std::endl;
        return VIPS_ERROR;
    } catch (const std::bad_alloc &e) {
        std::cerr << "Memory allocation error during resize_image: " << e.what() << std::endl;
        return MEMORY_ALLOCATION_FAILURE;
    } catch (const std::exception &e) {
        std::cerr << "Standard exception during resize_image: " << e.what() << std::endl;
        return UNKNOWN_ERROR;
    } catch (...) {
        std::cerr << "Unknown error occurred during resize_image." << std::endl;
        return UNKNOWN_ERROR;
    }
}

/**
 * @brief Crops an image to the specified rectangle.
 * This function modifies the VImage associated with the handle in-place.
 *
 * @param handle The VImageHandle to be cropped.
 * @param options The crop options including x and y coordinates, width and height.
 * @return SUCCESS on success, or an appropriate error code on failure.
 */
ImageStatus crop_image(VImageHandle handle, ImageCropOptions options) {
    if (!handle) {
        std::cerr << "Error: Invalid VImage handle for crop operation." << std::endl;
        return VIPS_INVALID_HANDLE;
    }
    if (options.width <= 0 || options.height <= 0) {
        std::cerr << "Error: Invalid dimensions provided for crop (width and height must be positive)." << std::endl;
        return IMAGE_INVALID_DIMENSIONS;
    }
    if (options.x < 0 || options.y < 0) {
        std::cerr << "Error: Invalid position provided for crop (x and y must be non-negative)." << std::endl;
        return IMAGE_INVALID_POSITION;
    }

    try {
        VImage* img = static_cast<VImage*>(handle);

        // Validate crop bounds against image dimensions
        if (static_cast<long long>(options.x) + options.width > img->width() ||
            static_cast<long long>(options.y) + options.height > img->height()) {
            std::cerr << "Error: Crop area extends beyond image boundaries." << std::endl;
            return IMAGE_INVALID_BOUNDS;
        }

        VImage cropped_img = img->crop(options.x, options.y, options.width, options.height);

        // Overwrite the original VImage object with the cropped one
        *img = cropped_img;
        return SUCCESS;
    } catch (const VError &e) {
        std::cerr << "VIPS Error during crop_image: " << e.what() << std::endl;
        return VIPS_ERROR;
    } catch (const std::bad_alloc &e) {
        std::cerr << "Memory allocation error during crop_image: " << e.what() << std::endl;
        return MEMORY_ALLOCATION_FAILURE;
    } catch (const std::exception &e) {
        std::cerr << "Standard exception during crop_image: " << e.what() << std::endl;
        return UNKNOWN_ERROR;
    } catch (...) {
        std::cerr << "Unknown error occurred during crop_image." << std::endl;
        return UNKNOWN_ERROR;
    }
}

/**
 * @brief Rotates an image by the specified angle in degrees.
 * This function modifies the VImage associated with the handle in-place.
 *
 * @param handle The VImageHandle to be rotated.
 * @param options The rotation options including the angle.
 * @return SUCCESS on success, or an appropriate error code on failure.
 */
ImageStatus rotate_image(VImageHandle handle, ImageRotateOptions options) {
    if (!handle) {
        std::cerr << "Error: Invalid VImage handle for rotate operation." << std::endl;
        return VIPS_INVALID_HANDLE;
    }

    try {
        VImage* img = static_cast<VImage*>(handle);

        std::vector<double> background_color;
        // Determine background color based on image properties
        if (img->has_alpha()) {
            background_color = {0.0, 0.0, 0.0, 0.0}; // Transparent black
        } else if (img->bands() >= 3) {
            background_color = {255.0, 255.0, 255.0}; // White
        } else {
            background_color = {0.0}; // Black (grayscale)
        }

        VImage rotated_img = img->rotate(options.angle, VImage::option()
            ->set("background", background_color));

        // Overwrite the original VImage object with the rotated one
        *img = rotated_img;
        return SUCCESS;
    } catch (const VError &e) {
        std::cerr << "VIPS Error during rotate_image: " << e.what() << std::endl;
        return VIPS_ERROR;
    } catch (const std::bad_alloc &e) {
        std::cerr << "Memory allocation error during rotate_image: " << e.what() << std::endl;
        return MEMORY_ALLOCATION_FAILURE;
    } catch (const std::exception &e) {
        std::cerr << "Standard exception during rotate_image: " << e.what() << std::endl;
        return UNKNOWN_ERROR;
    } catch (...) {
        std::cerr << "Unknown error occurred during rotate_image." << std::endl;
        return UNKNOWN_ERROR;
    }
}

/**
 * @brief Applies a watermark image to a base image.
 * This function modifies the base VImage associated with the handle in-place.
 *
 * @param base_handle The VImageHandle of the base image.
 * @param watermark_handle The VImageHandle of the watermark image.
 * @param options Watermark options including position and opacity.
 * @return SUCCESS on success, or an appropriate error code on failure.
 */
ImageStatus watermark_image(VImageHandle base_handle, VImageHandle watermark_handle, ImageWatermarkOptions options) {
    if (!base_handle || !watermark_handle) {
        std::cerr << "Error: Invalid VImage handle(s) for watermark operation." << std::endl;
        return VIPS_INVALID_HANDLE;
    }

    // Ensure opacity is within valid range [0.0, 1.0]
    options.opacity = std::max(0.0, std::min(1.0, options.opacity));

    try {
        VImage* base_img = static_cast<VImage*>(base_handle);
        VImage* watermark_img = static_cast<VImage*>(watermark_handle);

        VImage watermark_copy = *watermark_img;

        // Ensure watermark has an alpha channel for blending, add one if missing
        if (!watermark_copy.has_alpha()) {
            watermark_copy = watermark_copy.bandjoin(255); // Add opaque alpha channel
        }

        // Apply opacity to the watermark's alpha channel if less than 1.0
        if (options.opacity < 1.0) {
            VImage alpha_channel = watermark_copy.extract_band(watermark_copy.bands() - 1);
            VImage scaled_alpha = alpha_channel * options.opacity;
            watermark_copy = watermark_copy.extract_band(0, VImage::option()->set("n", watermark_copy.bands() - 1))
                       .bandjoin(scaled_alpha);
        }

        // Composite the watermark onto the base image
        VImage result_img = base_img->composite2(watermark_copy, VIPS_BLEND_MODE_OVER,
            VImage::option()->set("x", options.x)->set("y", options.y));

        // Overwrite the original base VImage object with the watermarked one
        *base_img = result_img;
        return SUCCESS;
    } catch (const VError &e) {
        std::cerr << "VIPS Error during watermark_image: " << e.what() << std::endl;
        return VIPS_ERROR;
    } catch (const std::bad_alloc &e) {
        std::cerr << "Memory allocation error during watermark_image: " << e.what() << std::endl;
        return MEMORY_ALLOCATION_FAILURE;
    } catch (const std::exception &e) {
        std::cerr << "Standard exception during watermark_image: " << e.what() << std::endl;
        return UNKNOWN_ERROR;
    } catch (...) {
        std::cerr << "Unknown error occurred during watermark_image." << std::endl;
        return UNKNOWN_ERROR;
    }
}

/**
 * @brief Changes the overall opacity of an image.
 * This function modifies the VImage associated with the handle in-place.
 *
 * @param handle The VImageHandle of the image to modify.
 * @param options Opacity options including the desired opacity level.
 * @return SUCCESS on success, or an appropriate error code on failure.
 */
ImageStatus change_image_opacity(VImageHandle handle, ImageOpacityOptions options) {
    if (!handle) {
        std::cerr << "Error: Invalid VImage handle for opacity change operation." << std::endl;
        return VIPS_INVALID_HANDLE;
    }

    // Ensure opacity is within valid range [0.0, 1.0]
    options.opacity = std::max(0.0, std::min(1.0, options.opacity));

    try {
        VImage* img = static_cast<VImage*>(handle);
        VImage img_copy = *img; // Work on a copy to avoid partial modifications on error

        // Add an alpha channel if the image doesn't have one
        if (!img_copy.has_alpha()) {
            img_copy = img_copy.bandjoin(255); // Add an opaque alpha channel
        }

        // Extract and scale the alpha channel
        VImage alpha_channel = img_copy.extract_band(img_copy.bands() - 1);
        VImage scaled_alpha = alpha_channel * options.opacity;

        // Recombine the color bands with the new alpha channel
        VImage result_img = img_copy.extract_band(0, VImage::option()->set("n", img_copy.bands() - 1))
                       .bandjoin(scaled_alpha);

        // Overwrite the original VImage object with the modified one
        *img = result_img;
        return SUCCESS;
    } catch (const VError &e) {
        std::cerr << "VIPS Error during change_image_opacity: " << e.what() << std::endl;
        return VIPS_ERROR;
    } catch (const std::bad_alloc &e) {
        std::cerr << "Memory allocation error during change_image_opacity: " << e.what() << std::endl;
        return MEMORY_ALLOCATION_FAILURE;
    } catch (const std::exception &e) {
        std::cerr << "Standard exception during change_image_opacity: " << e.what() << std::endl;
        return UNKNOWN_ERROR;
    } catch (...) {
        std::cerr << "Unknown error occurred during change_image_opacity." << std::endl;
        return UNKNOWN_ERROR;
    }
}

/**
 * @brief Extracts metadata from a VImage handle.
 * @param handle The VImageHandle from which to extract metadata.
 * @return An ImageMeta struct containing the extracted metadata.
 */
ImageMeta extract_metadata(const VImageHandle handle) {
    ImageMeta meta = {0}; // Initialize all members to zero/null
    if (!handle) {
        std::cerr << "Warning: Invalid VImage handle provided for metadata extraction." << std::endl;
        return meta;
    }

    const VImage* img = static_cast<const VImage*>(handle);

    meta.width = img->width();
    meta.height = img->height();
    meta.channels = img->bands();

    // Safely retrieve and copy format string
    try {
        std::string format_str = img->get_string("vips-loader");
        std::strncpy(meta.format, format_str.c_str(), sizeof(meta.format) - 1);
        meta.format[sizeof(meta.format) - 1] = '\0';
    } catch (const VError &e) {
        std::cerr << "VIPS Error getting image format: " << e.what() << std::endl;
        std::strncpy(meta.format, "unknown", sizeof(meta.format) - 1);
        meta.format[sizeof(meta.format) - 1] = '\0';
    }

    // Safely retrieve and copy colorspace string
    try {
        VipsInterpretation interp = img->interpretation();
        const char* colorspace_name = vips_enum_nick(VIPS_TYPE_INTERPRETATION, interp);
        if (colorspace_name) {
            std::strncpy(meta.colorspace, colorspace_name, sizeof(meta.colorspace) - 1);
            meta.colorspace[sizeof(meta.colorspace) - 1] = '\0';
        } else {
            std::strncpy(meta.colorspace, "unknown", sizeof(meta.colorspace) - 1);
            meta.colorspace[sizeof(meta.colorspace) - 1] = '\0';
        }
    } catch (const VError &e) {
        std::cerr << "VIPS Error getting image colorspace: " << e.what() << std::endl;
        std::strncpy(meta.colorspace, "unknown", sizeof(meta.colorspace) - 1);
        meta.colorspace[sizeof(meta.colorspace) - 1] = '\0';
    }

    // Safely retrieve density (DPI)
    try {
        meta.density_x = img->xres();
        meta.density_y = img->yres();
    } catch (const VError &e) {
        std::cerr << "VIPS Error getting image density: " << e.what() << std::endl;
        meta.density_x = 72.0; // Default DPI
        meta.density_y = 72.0;
    }

    meta.file_size = 0; // This field is typically populated after encoding to a file/buffer

    return meta;
}

/**
 * @brief Encodes the image to JPEG format and returns the encoded data in a buffer.
 * The caller is responsible for freeing the `data` pointer of the returned `EncodedImage`.
 *
 * @param handle The VImageHandle of the image to encode.
 * @param options JPEG encoding options (quality, interlacing).
 * @return An EncodedImage struct containing the buffer and its size, or {nullptr, 0} on failure.
 */
ImageBuffer encode_to_jpeg(const VImageHandle handle, ImageEncodeJPEGOptions options) {
    if (!handle) {
        std::cerr << "Error: Invalid VImage handle for JPEG encoding." << std::endl;
        return ImageBuffer{nullptr, 0};
    }

    void* buf = nullptr;
    size_t buf_size = 0;
    const VImage* img = static_cast<const VImage*>(handle);

    try {
        VOption* option = VImage::option();
        if (options.quality > 0 && options.quality <= 100) {
            option->set("Q", options.quality);
        } else {
            option->set("Q", 75); // Default JPEG quality
        }
        option->set("interlace", options.interlace != 0);

        // Write the image to a buffer in JPEG format
        img->write_to_buffer(".jpg", &buf, &buf_size, option);

        // The buffer returned by write_to_buffer is managed by GLib, so we transfer ownership
        return ImageBuffer{static_cast<unsigned char*>(buf), buf_size};
    } catch (const VError &e) {
        std::cerr << "VIPS Error during JPEG encoding: " << e.what() << std::endl;
        if (buf) g_free(buf); // Ensure buffer is freed on error
        return ImageBuffer{nullptr, 0};
    } catch (const std::bad_alloc &e) {
        std::cerr << "Memory allocation error during JPEG encoding: " << e.what() << std::endl;
        if (buf) g_free(buf);
        return ImageBuffer{nullptr, 0};
    } catch (const std::exception &e) {
        std::cerr << "Standard exception during JPEG encoding: " << e.what() << std::endl;
        if (buf) g_free(buf);
        return ImageBuffer{nullptr, 0};
    } catch (...) {
        std::cerr << "Unknown error occurred during JPEG encoding." << std::endl;
        if (buf) g_free(buf);
        return ImageBuffer{nullptr, 0};
    }
}

/**
 * @brief Encodes the image to PNG format and returns the encoded data in a buffer.
 * The caller is responsible for freeing the `data` pointer of the returned `EncodedImage`.
 *
 * @param handle The VImageHandle of the image to encode.
 * @param options PNG encoding options (compression, interlacing).
 * @return An EncodedImage struct containing the buffer and its size, or {nullptr, 0} on failure.
 */
ImageBuffer encode_to_png(const VImageHandle handle, ImageEncodePNGOptions options) {
    if (!handle) {
        std::cerr << "Error: Invalid VImage handle for PNG encoding." << std::endl;
        return ImageBuffer{nullptr, 0};
    }

    void* buf = nullptr;
    size_t buf_size = 0;
    const VImage* img = static_cast<const VImage*>(handle);

    try {
        VOption* option = VImage::option();
        // Validate compression level, default to 6 if invalid
        if (options.compression < 0 || options.compression > 9) {
            options.compression = 6;
        }
        option->set("compression", options.compression);
        option->set("interlace", options.interlace != 0);

        // Write the image to a buffer in PNG format
        img->write_to_buffer(".png", &buf, &buf_size, option);

        // The buffer returned by write_to_buffer is managed by GLib, so we transfer ownership
        return ImageBuffer{static_cast<unsigned char*>(buf), buf_size};
    } catch (const VError &e) {
        std::cerr << "VIPS Error during PNG encoding: " << e.what() << std::endl;
        if (buf) g_free(buf); // Ensure buffer is freed on error
        return ImageBuffer{nullptr, 0};
    } catch (const std::bad_alloc &e) {
        std::cerr << "Memory allocation error during PNG encoding: " << e.what() << std::endl;
        if (buf) g_free(buf);
        return ImageBuffer{nullptr, 0};
    } catch (const std::exception &e) {
        std::cerr << "Standard exception during PNG encoding: " << e.what() << std::endl;
        if (buf) g_free(buf);
        return ImageBuffer{nullptr, 0};
    } catch (...) {
        std::cerr << "Unknown error occurred during PNG encoding." << std::endl;
        if (buf) g_free(buf);
        return ImageBuffer{nullptr, 0};
    }
}

} // extern "C"
