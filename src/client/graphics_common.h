//
// Created by ekomov on 02.04.25.
//

#ifndef SRC_GRAPHICS_COMMON_H
#define SRC_GRAPHICS_COMMON_H

#include <X11/Xlib.h>


typedef struct pixelCoordinates pixelCoordinates;
struct pixelCoordinates {
    int x;
    int y;
};

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} RGB;


pixelCoordinates correctPixelCoordinates(int x, int y, int minX, int minY, int maxX, int maxY);
RGB get_pixel_rgb(XImage *image, int x, int y);
void set_pixel_color(XImage *image, int x, int y, unsigned long pixel_color);
unsigned long rgb_to_hex(uint8_t red, uint8_t green, uint8_t blue);
float linear_interpolation(float ratio, int valueLeft, int valueRight);
float bilinear_interpolation(float ratio_x, float ratio_y, int value_11, int value_12, int value_21, int value_22);
float quadratic_interpolation(float x, float x0, float f0, float x1, float f1, float x2, float f2);

RGB pixel_bilinear_interpolation(float fractionOfX, float fractionOfY, RGB p11, RGB p12, RGB p21, RGB p22);
RGB hex_to_rgb(unsigned long hex_color);
bool isRGBColorsEqual(RGB rgb1, RGB rgb2);
RGB get_pixel_color_or_black_if_its_mask_color(RGB pixel_color);
RGB get_pixel_color_if_fg_mask_or_black(RGB pixel_color);
RGB get_rgb_from_pixel(Display *display, unsigned long pixel);

#endif //SRC_GRAPHICS_COMMON_H
