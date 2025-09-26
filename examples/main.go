package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	"time"

	"github.com/szytko/vipsgo/vips"
)

func testFile() {
	fmt.Println("=== Go Image SDK VImage Chaining Test ===")

	inputImage := "../c/test/test.jpg"

	start := time.Now()

	// Test VImage chaining approach
	fmt.Println("\n=== VImage Chaining Approach ===")

	// Load image once
	fmt.Println("1. Loading image...")
	vimg, err := vipsgo.LoadImage(inputImage)
	if err != nil {
		fmt.Printf("   Error loading image: %v\n", err)
		return
	}
	defer vimg.Free()
	fmt.Println("   Image loaded successfully")

	// Chain operations without reloading
	fmt.Println("2. Chaining operations: Resize -> Crop -> Rotate...")

	// Resize (modifies vimg in place)
	err = vimg.Resize(&vipsgo.ImageResizeOptions{
		MaintainAspect: true,
		Width:          800,
		Height:         600,
	})
	if err != nil {
		fmt.Printf("   Resize failed: %v\n", err)
		return
	}

	// Get metadata after resize
	meta, err := vimg.ExtractMetadata()
	if err != nil {
		fmt.Printf("   Failed to extract metadata: %v\n", err)
		return
	}
	fmt.Printf("   Resized to: %dx%d\n", meta.Width, meta.Height)

	// Crop (modifies vimg in place)
	err = vimg.Crop(&vipsgo.ImageCropOptions{50, 50, 400, 300})
	if err != nil {
		fmt.Printf("   Crop failed: %v\n", err)
		return
	}

	// Get metadata after crop
	meta, err = vimg.ExtractMetadata()
	if err != nil {
		fmt.Printf("   Failed to extract metadata: %v\n", err)
		return
	}
	fmt.Printf("   Cropped to: %dx%d\n", meta.Width, meta.Height)

	// Rotate (modifies vimg in place)
	err = vimg.Rotate(&vipsgo.ImageRotateOptions{Angle: 15.0})
	if err != nil {
		fmt.Printf("   Rotate failed: %v\n", err)
		return
	}

	// Get metadata after rotate
	meta, err = vimg.ExtractMetadata()
	if err != nil {
		fmt.Printf("   Failed to extract metadata: %v\n", err)
		return
	}
	fmt.Printf("   Rotated, new dimensions: %dx%d\n", meta.Width, meta.Height)

	// Convert to bytes only once at the end
	fmt.Println("3. Converting final result to bytes...")
	finalBytes, err := vimg.EncodeToJPEG(&vipsgo.ImageEncodeJPEGOptions{Quality: 85, Interlace: false})
	if err != nil {
		fmt.Printf("   JPEG encoding failed: %v\n", err)
		return
	}
	fmt.Printf("   Final image: %dx%d, %d bytes\n", meta.Width, meta.Height, len(finalBytes))

	// Performance comparison
	fmt.Println("\n=== Time result ===")
	fmt.Printf("	Total time: %v ms\n", time.Since(start).Milliseconds())

	// Print final metadata
	fmt.Println("\n=== Final Image Metadata (JSON) ===")
	jsonData, _ := json.MarshalIndent(meta, "", "  ")
	fmt.Println(string(jsonData))

	fmt.Println("\n=== Key Benefits of VImage Chaining ===")
	fmt.Println("Load image only once")
	fmt.Println("No intermediate file I/O")
	fmt.Println("Better memory efficiency")
	fmt.Println("Faster processing pipeline")
	fmt.Println("Cleaner, more readable code")

	// NEW FEATURE: Test LoadImageFromBytes
	fmt.Println("\n=== Testing NEW LoadImageFromBytes Feature ===")

	// Use the bytes we just encoded to test loading from bytes
	fmt.Println("Loading image from the encoded bytes...")
	vimgFromBytes, err := vipsgo.LoadImageFromBytes(finalBytes)
	if err != nil {
		fmt.Printf("   Error loading image from bytes: %v\n", err)
		return
	}
	defer vimgFromBytes.Free()
	fmt.Println("   Image loaded successfully from bytes!")

	// Get metadata from the bytes-loaded image
	metaFromBytes, err := vimgFromBytes.ExtractMetadata()
	if err != nil {
		fmt.Printf("   Failed to extract metadata from bytes-loaded image: %v\n", err)
		return
	}
	fmt.Printf("   Bytes-loaded image metadata: %dx%d, %d channels, %s format\n",
		metaFromBytes.Width, metaFromBytes.Height, metaFromBytes.Channels, metaFromBytes.Format)

	fmt.Println("\n=== LoadImageFromBytes Feature Test Completed Successfully! ===")
}

func testBytes() {
	fmt.Println("=== Testing LoadImageFromBytes ===")

	// Read image file into bytes
	imageBytes, err := os.ReadFile("../c/test/test.jpg")
	if err != nil {
		log.Fatalf("Failed to read image file: %v", err)
	}

	fmt.Printf("Read %d bytes from file\n", len(imageBytes))

	// Load image from bytes
	fmt.Println("Loading image from bytes...")
	vimg, err := vipsgo.LoadImageFromBytes(imageBytes)
	if err != nil {
		fmt.Printf("   Error loading image from bytes: %v\n", err)
		return
	}
	defer vimg.Free()
	fmt.Println("   Image loaded successfully from bytes")

	// Get metadata
	meta, err := vimg.ExtractMetadata()
	if err != nil {
		fmt.Printf("   Failed to extract metadata: %v\n", err)
		return
	}
	fmt.Printf("   Image metadata: %dx%d, %d channels, %s format\n",
		meta.Width, meta.Height, meta.Channels, meta.Format)

	// Test a simple operation
	fmt.Println("Testing resize operation...")
	err = vimg.Resize(&vipsgo.ImageResizeOptions{
		MaintainAspect: true,
		Width:          400,
		Height:         300,
	})
	if err != nil {
		fmt.Printf("   Resize failed: %v\n", err)
		return
	}

	// Get new metadata
	meta, err = vimg.ExtractMetadata()
	if err != nil {
		fmt.Printf("   Failed to extract metadata after resize: %v\n", err)
		return
	}
	fmt.Printf("   Resized to: %dx%d\n", meta.Width, meta.Height)

	// Encode back to bytes
	fmt.Println("Encoding back to JPEG bytes...")
	finalBytes, err := vimg.EncodeToJPEG(&vipsgo.ImageEncodeJPEGOptions{Quality: 85, Interlace: false})
	if err != nil {
		fmt.Printf("   JPEG encoding failed: %v\n", err)
		return
	}
	fmt.Printf("   Encoded to %d bytes\n", len(finalBytes))

	fmt.Println("\n=== LoadImageFromBytes Test Completed Successfully! ===")
}

func main() {
	// Initialize SDK
	err := vipsgo.Init()
	if err != nil {
		log.Fatalf("Failed to initialize SDK: %v", err)
	}
	defer vipsgo.Cleanup()
	testFile()
	testBytes()
}
