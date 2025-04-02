//
// Created by ekomov on 02.04.25.
//
#include "angband.h"
#include "graphics_common.h"


pixelCoordinates correctPixelCoordinates(int x, int y, int minX, int minY, int maxX, int maxY) {
    struct pixelCoordinates correctedPixelCoordinates;
    correctedPixelCoordinates.x = x;
    correctedPixelCoordinates.y = y;

    if (x < minX) correctedPixelCoordinates.x = minX;
    if (x > maxX) correctedPixelCoordinates.x = maxX;

    if (y < minY) correctedPixelCoordinates.y = minY;
    if (y > maxY) correctedPixelCoordinates.y = maxY;

    return correctedPixelCoordinates;
}

// Function to get the RGB values of a pixel in an XImage
RGB get_pixel_rgb(XImage *image, int x, int y) {
    RGB pixel_rgb = {0, 0, 0}; // Initialize to black

    // Check for out-of-bounds coordinates
    if (x < 0 || x >= image->width || y < 0 || y >= image->height) {
        fprintf(stderr, "Error: Pixel coordinates out of bounds.\n");
        return pixel_rgb; // Return black if out of bounds
    }

    int bytes_per_pixel = image->bits_per_pixel / 8;
    int offset = (y * image->bytes_per_line) + (x * bytes_per_pixel);

    if (bytes_per_pixel == 1) { // Grayscale - treat as equal R, G, and B
        unsigned char gray_value = image->data[offset];
        pixel_rgb.red = gray_value;
        pixel_rgb.green = gray_value;
        pixel_rgb.blue = gray_value;
    } else if (bytes_per_pixel == 3) { // RGB
        // Assuming RGB format (R, G, B)
        pixel_rgb.red = image->data[offset + 2];
        pixel_rgb.green = image->data[offset + 1];
        pixel_rgb.blue = image->data[offset + 0];
    } else if (bytes_per_pixel == 4) { // RGBA (or BGRA - might need adjustment)
        // Assuming ARGB (might need to swap R and B if it's BGRA)
        pixel_rgb.red = image->data[offset + 2];
        pixel_rgb.green = image->data[offset + 1];
        pixel_rgb.blue = image->data[offset + 0];

        // Note: You could also get the alpha value from image->data[offset + 3];
    } else {
        fprintf(stderr, "Error: Unsupported pixel depth.\n");
    }

    return pixel_rgb;
}

void set_pixel_color(XImage *image, int x, int y, unsigned long pixel_color) {
    int bytes_per_pixel = image->bits_per_pixel / 8; // Bytes per pixel (1 for grayscale, 3 or 4 for RGB/RGBA)
    int offset;

    // Boundary checks
    if (x < 0 || x >= image->width || y < 0 || y >= image->height) {
        fprintf(stderr, "Error: Pixel coordinates out of bounds.\n");
        return; // Or handle the error in a different way
    }

    // Calculate the offset into the image data
    offset = (y * image->bytes_per_line) + (x * bytes_per_pixel);  // Careful calculation

    // Set the pixel color based on the color depth
    if (bytes_per_pixel == 1) { // Grayscale
        // For grayscale, 'pixel_color' is usually a single byte (0-255)
        // However, Xlib uses unsigned long for colors.  If 'pixel_color'
        // is a byte, it's okay as it'll be properly cast.
        image->data[offset] = (char)pixel_color;  // Cast to char for byte-wise assignment

    } else if (bytes_per_pixel == 3) { // RGB
        // Assume RGB format (e.g., 0xRRGGBB)
        image->data[offset + 0] = (pixel_color >> 16) & 0xFF; // Red
        image->data[offset + 1] = (pixel_color >> 8) & 0xFF;  // Green
        image->data[offset + 2] = pixel_color & 0xFF;        // Blue

    } else if (bytes_per_pixel == 4) { // RGBA
        // Assume RGBA format (e.g., 0xAARRGGBB or ARGB - order depends on the system)
        //  This example assumes ARGB, but your system might be different.  Double-check.
        image->data[offset + 0] = (pixel_color >> 16) & 0xFF; // Red
        image->data[offset + 1] = (pixel_color >> 8) & 0xFF;  // Green
        image->data[offset + 2] = pixel_color & 0xFF;        // Blue
        image->data[offset + 3] = (pixel_color >> 24) & 0xFF; // Alpha

    } else {
        fprintf(stderr, "Error: Unsupported pixel depth.\n");
        return;  // Or handle the error
    }
}

unsigned long rgb_to_hex(uint8_t red, uint8_t green, uint8_t blue) {
    return ((unsigned long)red << 16) | ((unsigned long)green << 8) | blue;
}


float linear_interpolation(float ratio, int valueLeft, int valueRight)
{
    return (1 - ratio) * valueLeft + ratio * valueRight;
}

float bilinear_interpolation(float ratio_x, float ratio_y, int value_11, int value_12, int value_21, int value_22)
{
    float f1 = linear_interpolation(ratio_x, value_11, value_12);
    float f2 = linear_interpolation(ratio_x, value_21, value_22);

    return linear_interpolation(ratio_y, f1, f2);
}

float quadratic_interpolation(float x, float x0, float f0, float x1, float f1, float x2, float f2) {
    float L0, L1, L2;

    // Lagrange basis polynomials
    L0 = ((x - x1) * (x - x2)) / ((x0 - x1) * (x0 - x2));
    L1 = ((x - x0) * (x - x2)) / ((x1 - x0) * (x1 - x2));
    L2 = ((x - x0) * (x - x1)) / ((x2 - x0) * (x2 - x1));

    // Interpolated value
    return L0 * f0 + L1 * f1 + L2 * f2;
}

float bi_quadratic_interpolation(
        float x, float y,
        float x1, float x2,  float x3,
        float y1, float y2,  float y3,
        float f00, float f01, float f02,
        float f10, float f11, float f12,
        float f20, float f21, float f22
        ) {

    float F1, F2, F3;

    F1 = quadratic_interpolation(x, x1, f00, x2, f01, x3, f02);
    F2 = quadratic_interpolation(x, x1, f10, x2, f11, x3, f12);
    F3 = quadratic_interpolation(x, x1, f20, x2, f21, x3, f22);

    return quadratic_interpolation(y, y1, F1, y2, F2, y3, F3);
}

RGB pixel_bi_quadratic_interpolation(int x,  int y,
                                     RGB p00, RGB p01, RGB p02,
                                     RGB p10, RGB p11, RGB p12,
                                     RGB p20, RGB p21, RGB p22
                                     )
{
    RGB newColor = {0, 0, 0};

    return newColor;
}

RGB pixel_bilinear_interpolation(float fractionOfX, float fractionOfY, RGB p11, RGB p12, RGB p21, RGB p22)
{
    RGB newColor;
    newColor.blue = (int) bilinear_interpolation(fractionOfX, fractionOfY, p11.blue, p12.blue, p21.blue, p22.blue);
    newColor.green = (int) bilinear_interpolation(fractionOfX, fractionOfY, p11.green, p12.green, p21.green, p22.green);
    newColor.red = (int) bilinear_interpolation(fractionOfX, fractionOfY, p11.red, p12.red, p21.red, p22.red);

    return newColor;
}

RGB hex_to_rgb(unsigned long hex_color) {
    RGB rgb;
    rgb.red   = (hex_color >> 16) & 0xFF;
    rgb.green = (hex_color >> 8) & 0xFF;
    rgb.blue  = hex_color & 0xFF;

    return rgb;
}

bool isRGBColorsEqual(RGB rgb1, RGB rgb2)
{
    return (rgb1.red == rgb2.red) && (rgb1.green == rgb2.green) && (rgb1.blue == rgb2.blue);
}


RGB get_pixel_color_or_black_if_its_mask_color(RGB pixel_color)
{
    RGB fgColor;
    fgColor.red = 252;
    fgColor.green = 0;
    fgColor.blue = 251;
    // fgColor.red = 251; // 252
    // fgColor.green = 0;
    // fgColor.blue = 252; // 251

    RGB transparancyColor;
    transparancyColor.red = 29;
    transparancyColor.green = 33;
    transparancyColor.blue = 28;

    RGB bgColor;
    bgColor.red = 62;
    bgColor.green = 61;
    bgColor.blue = 0;

    RGB black;
    black.red = 0;
    black.green = 0;
    black.blue = 0;

    if (isRGBColorsEqual(pixel_color, bgColor)) pixel_color = black;

    if (isRGBColorsEqual(pixel_color, transparancyColor)) pixel_color = black;

    if (isRGBColorsEqual(pixel_color, fgColor)) pixel_color = black;

    return pixel_color;
}

RGB get_pixel_color_if_fg_mask_or_black(RGB pixel_color)
{
    RGB fgColor;
    fgColor.red = 252;
    fgColor.green = 0;
    fgColor.blue = 251;

    RGB black;
    black.red = 0;
    black.green = 0;
    black.blue = 0;

    if (! isRGBColorsEqual(pixel_color, fgColor)) pixel_color = black;

    return pixel_color;
}

RGB get_rgb_from_pixel(Display *display, unsigned long pixel) {
    RGB rgb = {0, 0, 0}; // Initialize to black
    XColor color;

    color.pixel = pixel;
    XQueryColor(display, DefaultColormapOfScreen(DefaultScreenOfDisplay(display)), &color);

    // XQueryColor returns values in the range 0-65535.  Scale to 0-255.
    rgb.red   = (uint8_t)(color.red   / 256);
    rgb.green = (uint8_t)(color.green / 256);
    rgb.blue  = (uint8_t)(color.blue  / 256);


    return rgb;
}