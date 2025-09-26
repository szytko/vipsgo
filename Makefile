# VipsGo Makefile
# Builds the C library required for the Go wrapper

.PHONY: all clean build test install examples help

# Default target
all: build

# Build the C library
build:
	@echo "Building VIPS wrapper C library..."
	cd vips/c && cmake . && make
	@echo "✅ C library built successfully!"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	cd vips/c && make clean || true
	cd vips/c && rm -rf CMakeFiles CMakeCache.txt cmake_install.cmake Makefile || true
	@echo "✅ Clean completed!"

# Test the library
test: build
	@echo "Running C library tests..."
	cd vips/c && ./test_wrapper
	@echo "✅ C library tests passed!"

# Build and test examples
examples: build
	@echo "Building and running Go examples..."
	cd examples && go mod tidy
	cd examples && go run main.go
	@echo "✅ Examples completed successfully!"

# Install system dependencies (macOS)
install-deps-macos:
	@echo "Installing VIPS via Homebrew..."
	brew install vips
	@echo "✅ VIPS installed!"

# Install system dependencies (Ubuntu/Debian)
install-deps-ubuntu:
	@echo "Installing VIPS development libraries..."
	sudo apt-get update
	sudo apt-get install -y libvips-dev
	@echo "✅ VIPS development libraries installed!"

# Install system dependencies (CentOS/RHEL)
install-deps-centos:
	@echo "Installing VIPS development libraries..."
	sudo yum install -y vips-devel
	@echo "✅ VIPS development libraries installed!"

# Full setup for development
setup: install-deps-macos build test
	@echo "🎉 VipsGo setup completed successfully!"
	@echo ""
	@echo "You can now use VipsGo in your Go projects:"
	@echo "  go get github.com/szytko/vipsgo/vips"

# Help target
help:
	@echo "VipsGo Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  build              - Build the C library"
	@echo "  clean              - Clean build artifacts"
	@echo "  test               - Run C library tests"
	@echo "  examples           - Build and run Go examples"
	@echo "  install-deps-macos - Install VIPS via Homebrew (macOS)"
	@echo "  install-deps-ubuntu- Install VIPS dev libs (Ubuntu/Debian)"
	@echo "  install-deps-centos- Install VIPS dev libs (CentOS/RHEL)"
	@echo "  setup              - Full setup (macOS)"
	@echo "  help               - Show this help message"
	@echo ""
	@echo "Quick start:"
	@echo "  make setup         # Full setup on macOS"
	@echo "  make build         # Just build the library"
