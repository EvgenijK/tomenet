//
// Created by ekomov on 02.04.25.
//

#ifndef SRC_GRAPHICS_COMMON_H
#define SRC_GRAPHICS_COMMON_H

#include <X11/Xlib.h>

typedef struct coordinates coordinates;
struct coordinates {
    int x;
    int y;
};

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} color_rgb;

typedef struct
{
    coordinates coordinates;
    color_rgb color;
} image_pixel;

typedef struct
{
    float left;
    float right;
} linear_sample;

typedef struct
{
    linear_sample top;
    linear_sample bottom;
} bilinear_sample;

coordinates correctPixelCoordinates(int x, int y, int minX, int minY, int maxX, int maxY);
color_rgb get_pixel_rgb(XImage *image, int x, int y);
void set_pixel_color(XImage *image, int x, int y, unsigned long pixel_color);
unsigned long rgb_to_hex(uint8_t red, uint8_t green, uint8_t blue);

// float linear_interpolation(float ratio, int value_left, int value_right);
float linear_interpolation(float ratio, linear_sample sample);

// float bilinear_interpolation(float ratio_x, float ratio_y, int value_11, int value_12, int value_21, int value_22);
float bilinear_interpolatqion(float ratio_x, float ratio_y, bilinear_sample sample);

float quadratic_interpolation(float x, float x0, float f0, float x1, float f1, float x2, float f2);

color_rgb pixel_bilinear_interpolation(float fractionOfX, float fractionOfY, color_rgb p11, color_rgb p12, color_rgb p21, color_rgb p22);
color_rgb hex_to_rgb(unsigned long color_hex);
bool isRGBColorsEqual(color_rgb color_1, color_rgb color_2);
color_rgb get_pixel_color_or_black_if_its_mask_color(color_rgb pixel_color);
color_rgb get_pixel_color_if_fg_mask_or_black(color_rgb pixel_color);
color_rgb get_rgb_from_pixel(Display *display, unsigned long pixel);

#endif //SRC_GRAPHICS_COMMON_H
