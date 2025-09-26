/**
 * @file vips_wrapper.h
 * @brief vips wrapper
 * 
 * A C/C++ image processing library built on VIPS for lightning-fast image operations.
 * Supports resize, crop, rotate, watermark, and format conversion with VImage handle
 * chaining for optimal performance.
 * 
 * @author Sławek Żytko
 * @version 1.0.0
 * 
 * @example Basic Usage:
 * @code
 * // Initialize SDK
 * if (vips_wrapper_init() != SUCCESS) {
 *     fprintf(stderr, "Failed to initialize SDK\n");
 *     return -1;
 * }
 * 
 * // Load image and chain operations
 * VImageHandle img = load_image("input.jpg");
 * if (img) {
 *     ImageResizeOptions resize_opts = {1, 800, 600};
 *     resize_image(img, resize_opts);
 *     
 *     ImageBuffer result = encode_to_jpeg(img, (ImageEncodeJPEGOptions){85, 0});
 *     // Save result.data to file...
 *     
 *     free(result.data);
 *     free_vimage_handle(img);
 * }
 * 
 * vips_wrapper_cleanup();
 * @endcode
 */

#ifndef IMAGE_SDK_H
#define IMAGE_SDK_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// CORE DATA STRUCTURES
//=============================================================================

/**
 * @brief Image metadata information
 * 
 * Contains comprehensive information about an image including dimensions,
 * color properties, and file characteristics.
 * 
 * @example
 * @code
 * VImageHandle img = load_image("photo.jpg");
 * ImageMeta meta = extract_metadata(img);
 * printf("Image: %dx%d, %d channels, %s format\n", 
 *        meta.width, meta.height, meta.channels, meta.format);
 * @endcode
 */
typedef struct {
    int width;              ///< Image width in pixels
    int height;             ///< Image height in pixels
    int channels;           ///< Number of color channels (1=grayscale, 3=RGB, 4=RGBA)
    char format[32];        ///< Original format (e.g., "jpegload", "pngload")
    char colorspace[32];    ///< Color space (e.g., "srgb", "cmyk")
    double density_x;       ///< Horizontal resolution in pixels per mm
    double density_y;       ///< Vertical resolution in pixels per mm
    long file_size;         ///< File size in bytes (0 if not from file)
} ImageMeta;

/**
 * @brief Options for image resizing operations
 * 
 * @example Resize maintaining aspect ratio:
 * @code
 * ImageResizeOptions opts = {1, 800, 600}; // maintain_aspect=1, max 800x600
 * ImageStatus result = resize_image(handle, opts);
 * @endcode
 * 
 * @example Resize to exact dimensions:
 * @code
 * ImageResizeOptions opts = {0, 1920, 1080}; // maintain_aspect=0, exact size
 * ImageStatus result = resize_image(handle, opts);
 * @endcode
 */
typedef struct {
    int maintain_aspect;    ///< 1 to maintain aspect ratio, 0 for exact dimensions
    int width;              ///< Target width in pixels (0 to auto-calculate)
    int height;             ///< Target height in pixels (0 to auto-calculate)
} ImageResizeOptions;

/**
 * @brief Options for image cropping operations
 * 
 * @example Crop center portion:
 * @code
 * ImageMeta meta = extract_metadata(handle);
 * ImageCropOptions opts = {
 *     (meta.width - 500) / 2,   // x: center horizontally
 *     (meta.height - 400) / 2,  // y: center vertically
 *     500,                      // width: 500px
 *     400                       // height: 400px
 * };
 * ImageStatus result = crop_image(handle, opts);
 * @endcode
 */
typedef struct {
    int x;                  ///< Left edge of crop rectangle (0-based)
    int y;                  ///< Top edge of crop rectangle (0-based)
    int width;              ///< Width of crop rectangle in pixels
    int height;             ///< Height of crop rectangle in pixels
} ImageCropOptions;

/**
 * @brief Options for watermark placement
 * 
 * @example Add semi-transparent watermark:
 * @code
 * VImageHandle base = load_image("photo.jpg");
 * VImageHandle watermark = load_image("logo.png");
 * ImageWatermarkOptions opts = {10, 10, 0.7}; // top-left, 70% opacity
 * ImageStatus result = watermark_image(base, watermark, opts);
 * @endcode
 */
typedef struct {
    int x;                  ///< Horizontal position of watermark
    int y;                  ///< Vertical position of watermark
    double opacity;         ///< Opacity level (0.0=transparent, 1.0=opaque)
} ImageWatermarkOptions;

/**
 * @brief Options for opacity adjustment
 * 
 * @example Make image 50% transparent:
 * @code
 * ImageOpacityOptions opts = {0.5}; // 50% opacity
 * ImageStatus result = change_image_opacity(handle, opts);
 * @endcode
 */
typedef struct {
    double opacity;         ///< Overall opacity (0.0=transparent, 1.0=opaque)
} ImageOpacityOptions;

/**
 * @brief Options for image rotation
 * 
 * @example Rotate 45 degrees clockwise:
 * @code
 * ImageRotateOptions opts = {45.0}; // 45 degrees
 * ImageStatus result = rotate_image(handle, opts);
 * @endcode
 * 
 * @example Rotate 90 degrees counter-clockwise:
 * @code
 * ImageRotateOptions opts = {-90.0}; // -90 degrees
 * ImageStatus result = rotate_image(handle, opts);
 * @endcode
 */
typedef struct {
    double angle;           ///< Rotation angle in degrees (positive=clockwise)
} ImageRotateOptions;

/**
 * @brief JPEG encoding options
 * 
 * @example High quality JPEG:
 * @code
 * ImageEncodeJPEGOptions opts = {95, 1}; // 95% quality, interlaced
 * ImageBuffer result = encode_to_jpeg(handle, opts);
 * @endcode
 * 
 * @example Web-optimized JPEG:
 * @code
 * ImageEncodeJPEGOptions opts = {75, 0}; // 75% quality, progressive
 * ImageBuffer result = encode_to_jpeg(handle, opts);
 * @endcode
 */
typedef struct {
    int quality;            ///< JPEG quality (1-100, higher=better quality)
    int interlace;          ///< 1 for progressive JPEG, 0 for baseline
} ImageEncodeJPEGOptions;

/**
 * @brief PNG encoding options
 * 
 * @example High compression PNG:
 * @code
 * ImageEncodePNGOptions opts = {9, 0}; // max compression, no interlace
 * ImageBuffer result = encode_to_png(handle, opts);
 * @endcode
 * 
 * @example Fast PNG encoding:
 * @code
 * ImageEncodePNGOptions opts = {1, 0}; // minimal compression, fast
 * ImageBuffer result = encode_to_png(handle, opts);
 * @endcode
 */
typedef struct {
    int compression;        ///< PNG compression level (0-9, higher=smaller file)
    int interlace;          ///< 1 for interlaced PNG, 0 for standard
} ImageEncodePNGOptions;

/**
 * @brief Encoded image data buffer
 * 
 * Contains the raw bytes of an encoded image. The caller is responsible
 * for freeing the data pointer using free().
 * 
 * @example Save encoded image to file:
 * @code
 * ImageBuffer buffer = encode_to_jpeg(handle, (ImageEncodeJPEGOptions){85, 0});
 * if (buffer.data && buffer.size > 0) {
 *     FILE* file = fopen("output.jpg", "wb");
 *     fwrite(buffer.data, 1, buffer.size, file);
 *     fclose(file);
 *     free(buffer.data); // Important: free the buffer
 * }
 * @endcode
 */
typedef struct {
    unsigned char* data;    ///< Pointer to encoded image data (caller must free)
    size_t size;            ///< Size of data in bytes
} ImageBuffer;

//=============================================================================
// STATUS CODES AND ERROR HANDLING
//=============================================================================

/**
 * @brief Status codes for image operations
 * 
 * All image processing functions return these status codes to indicate
 * success or the specific type of failure that occurred.
 * 
 * @example Error handling:
 * @code
 * ImageStatus status = resize_image(handle, opts);
 * switch (status) {
 *     case SUCCESS:
 *         printf("Resize successful\n");
 *         break;
 *     case VIPS_INVALID_HANDLE:
 *         fprintf(stderr, "Invalid image handle\n");
 *         break;
 *     case IMAGE_INVALID_DIMENSIONS:
 *         fprintf(stderr, "Invalid resize dimensions\n");
 *         break;
 *     default:
 *         fprintf(stderr, "Resize failed with code: %d\n", status);
 * }
 * @endcode
 */
typedef enum {
    UNKNOWN_ERROR = 0,              ///< Unknown or unspecified error
    SUCCESS = 1,                    ///< Operation completed successfully
    VIPS_ERROR = 2,                 ///< VIPS library error
    VIPS_INIT_FAILURE,              ///< Failed to initialize VIPS
    VIPS_INVALID_HANDLE,            ///< Invalid or null VImage handle
    MEMORY_ALLOCATION_FAILURE,      ///< Memory allocation failed
    IMAGE_INVALID_PATH,             ///< Invalid file path
    IMAGE_LOAD_FAILURE,             ///< Failed to load image from file
    IMAGE_INVALID_FORMAT,           ///< Unsupported image format
    IMAGE_INVALID_DIMENSIONS,       ///< Invalid width/height parameters
    IMAGE_INVALID_POSITION,         ///< Invalid x/y coordinates
    IMAGE_INVALID_BOUNDS,           ///< Operation exceeds image boundaries
    IMAGE_SAVE_FAILURE              ///< Failed to save image to file
} ImageStatus;

//=============================================================================
// CORE SDK FUNCTIONS
//=============================================================================

/**
 * @brief Initialize the Image SDK
 * 
 * Must be called before any other SDK functions. Initializes the underlying
 * VIPS library and sets up required resources.
 * 
 * @return SUCCESS on successful initialization, VIPS_INIT_FAILURE otherwise
 * 
 * @example
 * @code
 * if (vips_wrapper_init() != SUCCESS) {
 *     fprintf(stderr, "Failed to initialize Image SDK\n");
 *     return -1;
 * }
 * // ... use SDK functions ...
 * vips_wrapper_cleanup();
 * @endcode
 * 
 * @note This function is thread-safe and can be called multiple times.
 * @warning Always call vips_wrapper_cleanup() when done to free resources.
 */
int vips_wrapper_init();

/**
 * @brief Clean up SDK resources
 * 
 * Shuts down the VIPS library and frees all allocated resources.
 * Should be called when image processing is complete.
 * 
 * @example
 * @code
 * vips_wrapper_init();
 * // ... perform image operations ...
 * vips_wrapper_cleanup(); // Always cleanup when done
 * @endcode
 * 
 * @note After calling this function, vips_wrapper_init() must be called again
 *       before using any other SDK functions.
 */
void vips_wrapper_cleanup();

//=============================================================================
// VIMAGE HANDLE MANAGEMENT
//=============================================================================

/**
 * @brief Opaque handle to a VImage object
 * 
 * Represents a loaded image in memory that can be efficiently processed
 * through multiple operations without reloading from disk.
 */
typedef void* VImageHandle;

/**
 * @brief Load an image from file
 * 
 * Loads an image from the specified file path and returns a handle that
 * can be used for subsequent operations. The handle must be freed using
 * free_vimage_handle() when no longer needed.
 * 
 * @param input_path Path to the image file to load
 * @return VImageHandle on success, NULL on failure
 * 
 * @example Basic usage:
 * @code
 * VImageHandle img = load_image("photo.jpg");
 * if (img) {
 *     // Process image...
 *     free_vimage_handle(img);
 * } else {
 *     fprintf(stderr, "Failed to load image\n");
 * }
 * @endcode
 * 
 * @example Load and check metadata:
 * @code
 * VImageHandle img = load_image("input.png");
 * if (img) {
 *     ImageMeta meta = extract_metadata(img);
 *     printf("Loaded %dx%d image with %d channels\n", 
 *            meta.width, meta.height, meta.channels);
 *     free_vimage_handle(img);
 * }
 * @endcode
 * 
 * @note Supported formats: JPEG, PNG, TIFF, WebP, GIF, and many others
 * @warning Always check return value for NULL before using the handle
 */
VImageHandle load_image(const char* input_path);

/**
 * @brief Load an image from byte buffer
 * 
 * Loads an image from a byte buffer and returns a handle that can be used
 * for subsequent operations. The handle must be freed using free_vimage_handle()
 * when no longer needed.
 * 
 * @param data Pointer to the image data bytes
 * @param size Size of the image data in bytes
 * @return VImageHandle on success, NULL on failure
 * 
 * @example Load from byte array:
 * @code
 * unsigned char* image_data = ...; // Your image bytes
 * size_t data_size = ...; // Size of the data
 * VImageHandle img = load_image_from_bytes(image_data, data_size);
 * if (img) {
 *     // Process image...
 *     free_vimage_handle(img);
 * } else {
 *     fprintf(stderr, "Failed to load image from bytes\n");
 * }
 * @endcode
 * 
 * @example Load from network response:
 * @code
 * // After receiving image data from HTTP request
 * VImageHandle img = load_image_from_bytes(response_data, response_size);
 * if (img) {
 *     ImageMeta meta = extract_metadata(img);
 *     printf("Loaded %dx%d image from bytes\n", meta.width, meta.height);
 *     free_vimage_handle(img);
 * }
 * @endcode
 * 
 * @note Supports the same formats as load_image: JPEG, PNG, TIFF, WebP, GIF, etc.
 * @note The byte buffer is copied internally, so the original data can be freed after this call
 * @warning Always check return value for NULL before using the handle
 */
VImageHandle load_image_from_bytes(const unsigned char* data, size_t size);

/**
 * @brief Free a VImage handle
 * 
 * Releases the memory associated with a VImage handle. The handle becomes
 * invalid after this call and should not be used.
 * 
 * @param handle The VImageHandle to free (can be NULL)
 * 
 * @example
 * @code
 * VImageHandle img = load_image("photo.jpg");
 * if (img) {
 *     // ... process image ...
 *     free_vimage_handle(img); // Always free when done
 *     img = NULL; // Good practice to avoid accidental reuse
 * }
 * @endcode
 * 
 * @note It's safe to pass NULL to this function
 * @warning Do not use the handle after calling this function
 */
void free_vimage_handle(VImageHandle handle);

/**
 * @brief Free an ImageBuffer
 *
 * Releases the memory associated with an ImageBuffer. The buffer's data
 * becomes invalid after this call.
 *
 * @param buffer The ImageBuffer to free.
 *
 * @example
 * @code
 * ImageBuffer img_buffer = load_image_to_buffer("photo.png");
 * if (img_buffer.data) {
 *     // ... process image buffer ...
 *     free_image_buffer(img_buffer); // Always free when done
 *     img_buffer.data = nullptr; // Good practice to avoid accidental reuse
 *     img_buffer.size = 0;
 * }
 * @endcode
 *
 * @note This function checks if buffer.data is non-NULL before freeing.
 * @warning Do not use the buffer.data pointer after calling this function.
 */
void free_image_buffer(ImageBuffer buffer);

//=============================================================================
// IMAGE PROCESSING OPERATIONS
//=============================================================================

/**
 * @brief Resize an image
 * 
 * Resizes the image associated with the handle. The operation modifies
 * the image in-place for optimal performance.
 * 
 * @param handle VImageHandle of the image to resize
 * @param options Resize parameters (dimensions and aspect ratio settings)
 * @return SUCCESS on success, error code on failure
 * 
 * @example Resize maintaining aspect ratio:
 * @code
 * VImageHandle img = load_image("large_photo.jpg");
 * ImageResizeOptions opts = {1, 800, 600}; // fit within 800x600
 * ImageStatus result = resize_image(img, opts);
 * if (result == SUCCESS) {
 *     printf("Image resized successfully\n");
 * }
 * @endcode
 * 
 * @example Resize to exact dimensions:
 * @code
 * ImageResizeOptions opts = {0, 1920, 1080}; // exact 1920x1080
 * ImageStatus result = resize_image(img, opts);
 * @endcode
 * 
 * @note Uses high-quality Lanczos3 filtering for optimal results
 * @warning Ensure handle is valid before calling
 */
ImageStatus resize_image(VImageHandle handle, ImageResizeOptions options);

/**
 * @brief Crop an image to a rectangular region
 * 
 * Extracts a rectangular portion of the image. The operation modifies
 * the image in-place.
 * 
 * @param handle VImageHandle of the image to crop
 * @param options Crop parameters (position and dimensions)
 * @return SUCCESS on success, error code on failure
 * 
 * @example Crop center portion:
 * @code
 * VImageHandle img = load_image("photo.jpg");
 * ImageMeta meta = extract_metadata(img);
 * ImageCropOptions opts = {
 *     (meta.width - 500) / 2,   // center horizontally
 *     (meta.height - 400) / 2,  // center vertically
 *     500, 400                  // 500x400 crop
 * };
 * ImageStatus result = crop_image(img, opts);
 * @endcode
 * 
 * @example Crop top-left corner:
 * @code
 * ImageCropOptions opts = {0, 0, 300, 200}; // 300x200 from top-left
 * ImageStatus result = crop_image(img, opts);
 * @endcode
 * 
 * @note Crop coordinates are validated against image boundaries
 * @warning Crop rectangle must be within image bounds
 */
ImageStatus crop_image(VImageHandle handle, ImageCropOptions options);

/**
 * @brief Rotate an image by a specified angle
 * 
 * Rotates the image by the given angle in degrees. Positive angles rotate
 * clockwise, negative angles rotate counter-clockwise.
 * 
 * @param handle VImageHandle of the image to rotate
 * @param options Rotation parameters (angle in degrees)
 * @return SUCCESS on success, error code on failure
 * 
 * @example Rotate 90 degrees clockwise:
 * @code
 * ImageRotateOptions opts = {90.0};
 * ImageStatus result = rotate_image(img, opts);
 * @endcode
 * 
 * @example Rotate 45 degrees counter-clockwise:
 * @code
 * ImageRotateOptions opts = {-45.0};
 * ImageStatus result = rotate_image(img, opts);
 * @endcode
 * 
 * @note Background is filled with appropriate color (transparent for images with alpha)
 * @note Canvas size may increase to accommodate rotated image
 */
ImageStatus rotate_image(VImageHandle handle, ImageRotateOptions options);

/**
 * @brief Apply a watermark to an image
 * 
 * Composites a watermark image onto the base image at the specified position
 * with the given opacity level.
 * 
 * @param base_handle VImageHandle of the base image
 * @param watermark_handle VImageHandle of the watermark image
 * @param options Watermark parameters (position and opacity)
 * @return SUCCESS on success, error code on failure
 * 
 * @example Add logo watermark:
 * @code
 * VImageHandle photo = load_image("photo.jpg");
 * VImageHandle logo = load_image("logo.png");
 * ImageWatermarkOptions opts = {10, 10, 0.8}; // top-left, 80% opacity
 * ImageStatus result = watermark_image(photo, logo, opts);
 * free_vimage_handle(logo); // Can free watermark after use
 * @endcode
 * 
 * @example Center watermark:
 * @code
 * ImageMeta base_meta = extract_metadata(photo);
 * ImageMeta wm_meta = extract_metadata(watermark);
 * ImageWatermarkOptions opts = {
 *     (base_meta.width - wm_meta.width) / 2,   // center x
 *     (base_meta.height - wm_meta.height) / 2, // center y
 *     0.5  // 50% opacity
 * };
 * @endcode
 * 
 * @note Watermark is blended using alpha compositing
 * @note Watermark position can be negative (partial overlay)
 */
ImageStatus watermark_image(VImageHandle base_handle, VImageHandle watermark_handle, 
                           ImageWatermarkOptions options);

/**
 * @brief Change the overall opacity of an image
 * 
 * Adjusts the opacity of the entire image. Adds an alpha channel if the
 * image doesn't already have one.
 * 
 * @param handle VImageHandle of the image to modify
 * @param options Opacity parameters
 * @return SUCCESS on success, error code on failure
 * 
 * @example Make image 50% transparent:
 * @code
 * ImageOpacityOptions opts = {0.5}; // 50% opacity
 * ImageStatus result = change_image_opacity(img, opts);
 * @endcode
 * 
 * @example Make image fully opaque:
 * @code
 * ImageOpacityOptions opts = {1.0}; // 100% opacity
 * ImageStatus result = change_image_opacity(img, opts);
 * @endcode
 * 
 * @note Opacity values are clamped to [0.0, 1.0] range
 * @note Automatically adds alpha channel if needed
 */
ImageStatus change_image_opacity(VImageHandle handle, ImageOpacityOptions options);

//=============================================================================
// ENCODING AND METADATA FUNCTIONS
//=============================================================================

/**
 * @brief Encode image to JPEG format
 * 
 * Converts the image to JPEG format and returns the encoded data as bytes.
 * The caller is responsible for freeing the returned buffer.
 * 
 * @param handle VImageHandle of the image to encode
 * @param options JPEG encoding parameters (quality, interlacing)
 * @return ImageBuffer containing encoded data, or {NULL, 0} on failure
 * 
 * @example High quality JPEG:
 * @code
 * ImageEncodeJPEGOptions opts = {95, 0}; // 95% quality, baseline
 * ImageBuffer result = encode_to_jpeg(img, opts);
 * if (result.data) {
 *     // Save to file or send over network...
 *     free(result.data); // Important: free the buffer
 * }
 * @endcode
 * 
 * @example Web-optimized JPEG:
 * @code
 * ImageEncodeJPEGOptions opts = {75, 1}; // 75% quality, progressive
 * ImageBuffer result = encode_to_jpeg(img, opts);
 * @endcode
 * 
 * @note Quality range: 1-100 (higher = better quality, larger file)
 * @warning Always check result.data for NULL and free when done
 */
ImageBuffer encode_to_jpeg(const VImageHandle handle, ImageEncodeJPEGOptions options);

/**
 * @brief Encode image to PNG format
 * 
 * Converts the image to PNG format and returns the encoded data as bytes.
 * The caller is responsible for freeing the returned buffer.
 * 
 * @param handle VImageHandle of the image to encode
 * @param options PNG encoding parameters (compression, interlacing)
 * @return ImageBuffer containing encoded data, or {NULL, 0} on failure
 * 
 * @example High compression PNG:
 * @code
 * ImageEncodePNGOptions opts = {9, 0}; // max compression, standard
 * ImageBuffer result = encode_to_png(img, opts);
 * if (result.data) {
 *     FILE* file = fopen("output.png", "wb");
 *     fwrite(result.data, 1, result.size, file);
 *     fclose(file);
 *     free(result.data);
 * }
 * @endcode
 * 
 * @example Fast PNG encoding:
 * @code
 * ImageEncodePNGOptions opts = {1, 0}; // minimal compression, fast
 * ImageBuffer result = encode_to_png(img, opts);
 * @endcode
 * 
 * @note Compression range: 0-9 (higher = smaller file, slower encoding)
 * @note PNG preserves transparency and supports lossless compression
 * @warning Always check result.data for NULL and free when done
 */
ImageBuffer encode_to_png(const VImageHandle handle, ImageEncodePNGOptions options);

/**
 * @brief Extract metadata from an image
 * 
 * Retrieves comprehensive metadata information from the image including
 * dimensions, color properties, and format details.
 * 
 * @param handle VImageHandle of the image
 * @return ImageMeta structure containing metadata information
 * 
 * @example Display image information:
 * @code
 * VImageHandle img = load_image("photo.jpg");
 * ImageMeta meta = extract_metadata(img);
 * printf("Image: %dx%d pixels\n", meta.width, meta.height);
 * printf("Channels: %d\n", meta.channels);
 * printf("Format: %s\n", meta.format);
 * printf("Colorspace: %s\n", meta.colorspace);
 * printf("DPI: %.1fx%.1f\n", meta.density_x, meta.density_y);
 * @endcode
 * 
 * @example Check if image has transparency:
 * @code
 * ImageMeta meta = extract_metadata(img);
 * if (meta.channels == 4 || meta.channels == 2) {
 *     printf("Image has alpha channel\n");
 * }
 * @endcode
 * 
 * @note Returns zero-initialized structure if handle is invalid
 * @note file_size is typically 0 for images created in memory
 */
ImageMeta extract_metadata(const VImageHandle handle);

//=============================================================================
// USAGE EXAMPLES AND BEST PRACTICES
//=============================================================================

/**
 * @example Complete image processing pipeline:
 * @code
 * #include "vips_wrapper.h"
 * #include <stdio.h>
 * #include <stdlib.h>
 * 
 * int main() {
 *     // Initialize SDK
 *     if (vips_wrapper_init() != SUCCESS) {
 *         fprintf(stderr, "Failed to initialize SDK\n");
 *         return -1;
 *     }
 * 
 *     // Load and process image
 *     VImageHandle img = load_image("input.jpg");
 *     if (img) {
 *         // Chain operations for optimal performance
 *         ImageResizeOptions resize_opts = {1, 800, 600};
 *         resize_image(img, resize_opts);
 * 
 *         ImageCropOptions crop_opts = {50, 50, 700, 500};
 *         crop_image(img, crop_opts);
 * 
 *         ImageRotateOptions rotate_opts = {15.0};
 *         rotate_image(img, rotate_opts);
 * 
 *         // Encode and save
 *         ImageEncodeJPEGOptions jpeg_opts = {85, 0};
 *         ImageBuffer result = encode_to_jpeg(img, jpeg_opts);
 *         
 *         if (result.data) {
 *             FILE* file = fopen("output.jpg", "wb");
 *             fwrite(result.data, 1, result.size, file);
 *             fclose(file);
 *             free(result.data);
 *             printf("Processed image saved successfully\n");
 *         }
 * 
 *         free_vimage_handle(img);
 *     }
 * 
 *     vips_wrapper_cleanup();
 *     return 0;
 * }
 * @endcode
 */

#ifdef __cplusplus
}
#endif

#endif // IMAGE_SDK_H
