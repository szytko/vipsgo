package vips

/*
#cgo pkg-config: vips
#cgo CXXFLAGS: -std=c++11
#cgo CFLAGS: -I../c/include
#cgo LDFLAGS: -L../c -lvips_wrapper -Wl,-rpath,../c
#cgo pkg-config: vips-cpp
#include "vips_wrapper.h"
#include <stdlib.h> // For free

// Forward declarations for functions that return pointers,
// so Go can understand their signatures when called from C.
// These are not strictly necessary for simple cases but good practice.
extern VImageHandle load_image(const char* input_path);
extern VImageHandle load_image_from_bytes(const unsigned char* data, size_t size);
extern void free_vimage_handle(VImageHandle handle);
extern ImageBuffer encode_to_jpeg(VImageHandle handle, ImageEncodeJPEGOptions options);
extern ImageBuffer encode_to_png(VImageHandle handle, ImageEncodePNGOptions options);
extern void free_image_buffer(ImageBuffer buffer);
*/
import "C"
import (
	"errors"
	"fmt"
	"runtime"
	"unsafe"
)

// ImageStatus represents the status codes returned by image SDK functions.
type ImageStatus C.ImageStatus

const (
	Success                 ImageStatus = C.SUCCESS
	VipsInitFailure         ImageStatus = C.VIPS_INIT_FAILURE
	VipsInvalidHandle       ImageStatus = C.VIPS_INVALID_HANDLE
	VipsError               ImageStatus = C.VIPS_ERROR
	ImageInvalidDimensions  ImageStatus = C.IMAGE_INVALID_DIMENSIONS
	ImageInvalidPosition    ImageStatus = C.IMAGE_INVALID_POSITION
	ImageInvalidBounds      ImageStatus = C.IMAGE_INVALID_BOUNDS
	MemoryAllocationFailure ImageStatus = C.MEMORY_ALLOCATION_FAILURE
	UnknownError            ImageStatus = C.UNKNOWN_ERROR
)

// Error converts an ImageStatus to a Go error.
func (s ImageStatus) Error() error {
	if s == Success {
		return nil
	}
	switch s {
	case VipsInitFailure:
		return errors.New("VIPS initialization failed")
	case VipsInvalidHandle:
		return errors.New("invalid VIPS image handle")
	case VipsError:
		return errors.New("VIPS internal error")
	case ImageInvalidDimensions:
		return errors.New("invalid image dimensions")
	case ImageInvalidPosition:
		return errors.New("invalid image position")
	case ImageInvalidBounds:
		return errors.New("image operation out of bounds")
	case MemoryAllocationFailure:
		return errors.New("memory allocation failed")
	case UnknownError:
		return errors.New("an unknown error occurred")
	default:
		return fmt.Errorf("unknown image status code: %d", s)
	}
}

// Image represents a loaded image, managed by the C library.
type Image struct {
	handle C.VImageHandle
}

// ImageResizeOptions defines options for resizing an image.
type ImageResizeOptions struct {
	Width          int
	Height         int
	MaintainAspect bool
}

// ImageCropOptions defines options for cropping an image.
type ImageCropOptions struct {
	X      int
	Y      int
	Width  int
	Height int
}

// ImageRotateOptions defines options for rotating an image.
type ImageRotateOptions struct {
	Angle float64 // Angle in degrees
}

// ImageWatermarkOptions defines options for watermarking an image.
type ImageWatermarkOptions struct {
	X       int
	Y       int
	Opacity float64 // 0.0 to 1.0
}

// ImageOpacityOptions defines options for changing image opacity.
type ImageOpacityOptions struct {
	Opacity float64 // 0.0 to 1.0
}

// ImageMeta contains metadata extracted from an image.
type ImageMeta struct {
	Width      int
	Height     int
	Channels   int
	Format     string
	Colorspace string
	DensityX   float64
	DensityY   float64
	FileSize   int // This will be 0 as per C implementation, usually after encoding
}

// ImageEncodeJPEGOptions defines options for JPEG encoding.
type ImageEncodeJPEGOptions struct {
	Quality   int  // 1-100
	Interlace bool // Progressive JPEG
}

// ImageEncodePNGOptions defines options for PNG encoding.
type ImageEncodePNGOptions struct {
	Compression int  // 0-9
	Interlace   bool // Adam7 interlacing
}

// Init initializes the VIPS library. Must be called once before any other operations.
func Init() error {
	status := ImageStatus(C.vips_wrapper_init())
	return status.Error()
}

// Cleanup cleans up VIPS resources. Should be called when processing is complete.
func Cleanup() {
	C.vips_wrapper_cleanup()
}

// LoadImage loads an image from the given file path.
// It returns an *Image and an error if the loading fails.
func LoadImage(inputPath string) (*Image, error) {
	cInputPath := C.CString(inputPath)
	defer C.free(unsafe.Pointer(cInputPath))

	handle := C.load_image(cInputPath)
	if handle == nil {
		return nil, errors.New("failed to load image: check logs for VIPS errors")
	}

	img := &Image{handle: handle}
	// Set a finalizer to ensure the C handle is freed when the Go Image object is garbage collected.
	runtime.SetFinalizer(img, func(i *Image) {
		C.free_vimage_handle(i.handle)
	})
	return img, nil
}

// LoadImageFromBytes loads an image from a byte slice.
// It returns an *Image and an error if the loading fails.
func LoadImageFromBytes(data []byte) (*Image, error) {
	if len(data) == 0 {
		return nil, errors.New("image data is empty")
	}

	// Convert Go slice to C pointer and size
	cData := (*C.uchar)(unsafe.Pointer(&data[0]))
	cSize := C.size_t(len(data))

	handle := C.load_image_from_bytes(cData, cSize)
	if handle == nil {
		return nil, errors.New("failed to load image from bytes: check logs for VIPS errors")
	}

	img := &Image{handle: handle}
	// Set a finalizer to ensure the C handle is freed when the Go Image object is garbage collected.
	runtime.SetFinalizer(img, func(i *Image) {
		C.free_vimage_handle(i.handle)
	})
	return img, nil
}

// Free explicitly frees the resources associated with the image.
// After calling Free, the Image object should not be used.
func (img *Image) Free() {
	if img.handle != nil {
		C.free_vimage_handle(img.handle)
		img.handle = nil               // Prevent double-free by finalizer
		runtime.SetFinalizer(img, nil) // Remove finalizer
	}
}

// Resize resizes the image according to the specified options.
func (img *Image) Resize(options *ImageResizeOptions) error {
	if img.handle == nil {
		return VipsInvalidHandle.Error()
	}

	cOptions := C.ImageResizeOptions{
		width:           C.int(options.Width),
		height:          C.int(options.Height),
		maintain_aspect: C.int(0), // C.int(bool) is 0 for false, 1 for true
	}
	if options.MaintainAspect {
		cOptions.maintain_aspect = C.int(1)
	}

	status := ImageStatus(C.resize_image(img.handle, cOptions))
	return status.Error()
}

// Crop crops the image to the specified rectangle.
func (img *Image) Crop(options *ImageCropOptions) error {
	if img.handle == nil {
		return VipsInvalidHandle.Error()
	}

	cOptions := C.ImageCropOptions{
		x:      C.int(options.X),
		y:      C.int(options.Y),
		width:  C.int(options.Width),
		height: C.int(options.Height),
	}

	status := ImageStatus(C.crop_image(img.handle, cOptions))
	return status.Error()
}

// Rotate rotates the image by the specified angle in degrees.
func (img *Image) Rotate(options *ImageRotateOptions) error {
	if img.handle == nil {
		return VipsInvalidHandle.Error()
	}

	cOptions := C.ImageRotateOptions{
		angle: C.double(options.Angle),
	}

	status := ImageStatus(C.rotate_image(img.handle, cOptions))
	return status.Error()
}

// Watermark applies a watermark image to the base image.
func (baseImg *Image) Watermark(watermarkImg *Image, options *ImageWatermarkOptions) error {
	if baseImg.handle == nil || watermarkImg.handle == nil {
		return VipsInvalidHandle.Error()
	}

	cOptions := C.ImageWatermarkOptions{
		x:       C.int(options.X),
		y:       C.int(options.Y),
		opacity: C.double(options.Opacity),
	}

	status := ImageStatus(C.watermark_image(baseImg.handle, watermarkImg.handle, cOptions))
	return status.Error()
}

// ChangeOpacity changes the overall opacity of an image.
func (img *Image) ChangeOpacity(options *ImageOpacityOptions) error {
	if img.handle == nil {
		return VipsInvalidHandle.Error()
	}

	cOptions := C.ImageOpacityOptions{
		opacity: C.double(options.Opacity),
	}

	status := ImageStatus(C.change_image_opacity(img.handle, cOptions))
	return status.Error()
}

// ExtractMetadata extracts metadata from the image.
func (img *Image) ExtractMetadata() (ImageMeta, error) {
	if img.handle == nil {
		return ImageMeta{}, VipsInvalidHandle.Error()
	}

	cMeta := C.extract_metadata(img.handle)

	// Convert C strings to Go strings
	format := C.GoString(&cMeta.format[0])
	colorspace := C.GoString(&cMeta.colorspace[0])

	meta := ImageMeta{
		Width:      int(cMeta.width),
		Height:     int(cMeta.height),
		Channels:   int(cMeta.channels),
		Format:     format,
		Colorspace: colorspace,
		DensityX:   float64(cMeta.density_x),
		DensityY:   float64(cMeta.density_y),
		FileSize:   int(cMeta.file_size), // Will be 0
	}
	return meta, nil
}

// EncodeToJPEG encodes the image to JPEG format and returns the encoded data.
func (img *Image) EncodeToJPEG(options *ImageEncodeJPEGOptions) ([]byte, error) {
	if img.handle == nil {
		return nil, VipsInvalidHandle.Error()
	}

	cOptions := C.ImageEncodeJPEGOptions{
		quality:   C.int(options.Quality),
		interlace: C.int(0),
	}
	if options.Interlace {
		cOptions.interlace = C.int(1)
	}

	cBuffer := C.encode_to_jpeg(img.handle, cOptions)
	if cBuffer.data == nil {
		return nil, errors.New("failed to encode image to JPEG: check logs for VIPS errors")
	}
	defer C.free_image_buffer(cBuffer) // Ensure C-allocated buffer is freed

	// Convert C buffer to Go slice
	goBytes := C.GoBytes(unsafe.Pointer(cBuffer.data), C.int(cBuffer.size))
	return goBytes, nil
}

// EncodeToPNG encodes the image to PNG format and returns the encoded data.
func (img *Image) EncodeToPNG(options *ImageEncodePNGOptions) ([]byte, error) {
	if img.handle == nil {
		return nil, VipsInvalidHandle.Error()
	}

	cOptions := C.ImageEncodePNGOptions{
		compression: C.int(options.Compression),
		interlace:   C.int(0),
	}
	if options.Interlace {
		cOptions.interlace = C.int(1)
	}

	cBuffer := C.encode_to_png(img.handle, cOptions)
	if cBuffer.data == nil {
		return nil, errors.New("failed to encode image to PNG: check logs for VIPS errors")
	}
	defer C.free_image_buffer(cBuffer) // Ensure C-allocated buffer is freed

	// Convert C buffer to Go slice
	goBytes := C.GoBytes(unsafe.Pointer(cBuffer.data), C.int(cBuffer.size))
	return goBytes, nil
}
