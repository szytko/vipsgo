# VipsGo - High-Performance Image Processing for Go

[![Go Reference](https://pkg.go.dev/badge/github.com/szytko/vipsgo.svg)](https://pkg.go.dev/github.com/szytko/vipsgo)
[![Go Report Card](https://goreportcard.com/badge/github.com/szytko/vipsgo)](https://goreportcard.com/report/github.com/szytko/vipsgo)

A high-performance Go wrapper for the VIPS image processing library, providing lightning-fast image operations with minimal memory usage.

## Features

- 🚀 **High Performance**: Built on VIPS for optimal speed and memory efficiency
- 🔗 **Method Chaining**: Efficient in-place operations without reloading
- 📁 **Multiple Input Sources**: Load from files or byte arrays
- 🎨 **Rich Operations**: Resize, crop, rotate, watermark, opacity control
- 📊 **Metadata Extraction**: Comprehensive image information
- 🔄 **Format Support**: JPEG, PNG, WebP, TIFF, GIF and more
- 💾 **Memory Safe**: Automatic resource cleanup with Go finalizers

## Installation

### Prerequisites

First, install the VIPS library:

**macOS (Homebrew):**
```bash
brew install vips
```

**Ubuntu/Debian:**
```bash
sudo apt-get install libvips-dev
```

**CentOS/RHEL:**
```bash
sudo yum install vips-devel
```

### Install VipsGo

```bash
go get github.com/szytko/vipsgo/vips
```

## Quick Start

```go
package main

import (
    "fmt"
    "log"
    
    "github.com/szytko/vipsgo/vips"
)

func main() {
    // Initialize VIPS
    if err := vips.Init(); err != nil {
        log.Fatal(err)
    }
    defer vips.Cleanup()
    
    // Load image
    img, err := vips.LoadImage("input.jpg")
    if err != nil {
        log.Fatal(err)
    }
    defer img.Free()
    
    // Chain operations
    err = img.Resize(&vips.ImageResizeOptions{
        Width:          800,
        Height:         600,
        MaintainAspect: true,
    })
    if err != nil {
        log.Fatal(err)
    }
    
    // Encode to JPEG
    data, err := img.EncodeToJPEG(&vips.ImageEncodeJPEGOptions{
        Quality: 85,
    })
    if err != nil {
        log.Fatal(err)
    }
    
    fmt.Printf("Processed image: %d bytes\n", len(data))
}
```

## API Reference

### Initialization

```go
// Initialize VIPS (required before any operations)
err := vips.Init()
defer vips.Cleanup()
```

### Loading Images

```go
// From file
img, err := vips.LoadImage("image.jpg")

// From bytes (NEW!)
imageBytes, _ := ioutil.ReadFile("image.jpg")
img, err := vips.LoadImageFromBytes(imageBytes)
```

### Image Operations

```go
// Resize
err = img.Resize(&vips.ImageResizeOptions{
    Width:          800,
    Height:         600,
    MaintainAspect: true,
})

// Crop
err = img.Crop(&vips.ImageCropOptions{
    X:      50,
    Y:      50,
    Width:  400,
    Height: 300,
})

// Rotate
err = img.Rotate(&vips.ImageRotateOptions{
    Angle: 45.0,
})

// Watermark
watermark, _ := vips.LoadImage("logo.png")
err = img.Watermark(watermark, &vips.ImageWatermarkOptions{
    X:       10,
    Y:       10,
    Opacity: 0.8,
})

// Change opacity
err = img.ChangeOpacity(&vips.ImageOpacityOptions{
    Opacity: 0.5,
})
```

### Encoding

```go
// JPEG
jpegData, err := img.EncodeToJPEG(&vips.ImageEncodeJPEGOptions{
    Quality:   85,
    Interlace: false,
})

// PNG
pngData, err := img.EncodeToPNG(&vips.ImageEncodePNGOptions{
    Compression: 6,
    Interlace:   false,
})
```

### Metadata

```go
meta, err := img.ExtractMetadata()
fmt.Printf("Image: %dx%d, %d channels, %s format\n", 
    meta.Width, meta.Height, meta.Channels, meta.Format)
```

## Advanced Usage

### Processing Pipeline

```go
// Efficient chaining without reloading
img, _ := vips.LoadImage("large_image.jpg")
defer img.Free()

// Chain multiple operations
img.Resize(&vips.ImageResizeOptions{Width: 1200, Height: 800, MaintainAspect: true})
img.Crop(&vips.ImageCropOptions{X: 100, Y: 100, Width: 1000, Height: 600})
img.Rotate(&vips.ImageRotateOptions{Angle: 15.0})

// Final encoding
result, _ := img.EncodeToJPEG(&vips.ImageEncodeJPEGOptions{Quality: 90})
```

### Loading from HTTP Response

```go
response, err := http.Get("https://example.com/image.jpg")
if err != nil {
    log.Fatal(err)
}
defer response.Body.Close()

imageBytes, err := ioutil.ReadAll(response.Body)
if err != nil {
    log.Fatal(err)
}

img, err := vips.LoadImageFromBytes(imageBytes)
if err != nil {
    log.Fatal(err)
}
defer img.Free()

// Process the image...
```

## Performance

VipsGo is built on VIPS, which is significantly faster than ImageMagick and other image processing libraries:

- **Memory Efficient**: Streaming processing with minimal memory usage
- **Multi-threaded**: Automatic parallelization of operations
- **Optimized**: Hand-tuned algorithms for maximum performance
- **Format Optimized**: Native support for modern formats like WebP

## Examples

Check out the [examples](./examples/) directory for complete working examples:

- [Basic Usage](./examples/main.go) - Complete image processing pipeline
- [Bytes Processing](./examples/test_bytes.go) - Working with byte arrays

## Building from Source

```bash
# Clone the repository
git clone https://github.com/szytko/vipsgo.git
cd vipsgo

# Build the C library
cd vips/c
make

# Run examples
cd ../../examples
go run main.go
```

## Requirements

- Go 1.21 or later
- VIPS 8.12 or later
- CGO enabled

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built on the excellent [VIPS](https://libvips.github.io/libvips/) library
- Inspired by the need for high-performance image processing in Go
