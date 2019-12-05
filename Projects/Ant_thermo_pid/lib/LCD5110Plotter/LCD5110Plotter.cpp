#include "LCD5110Plotter.h"
#include <LCD5110_Graph.h>

LCD5110Plotter::LCD5110Plotter(LCD5110* display, float minValue, float maxValue) {
    this->display = display;
    this->minValue = minValue;
    this->maxValue = maxValue;
    pos = left = 0;
    top = 0;
    right = MAX_WIDTH;
    bottom = MAX_HEIGHT;
}

void LCD5110Plotter::InitSize(uint8_t left, uint8_t top, uint8_t right, uint8_t bottom) {
    this->pos = this->left = left;
    this->top = top;
    this->right = right;
    this->bottom = bottom;
}

void LCD5110Plotter::Push(float newValue) {
    uint8_t v = int(float(bottom) - (newValue - minValue)/(maxValue - minValue)*(bottom - top) - 0.5f);
    if (pos < right) {
        if (v >= top && v < bottom) {
            display->setPixel(pos, v);
        }
        data[pos++] = v;
    }
    else {
        ShiftData();
        data[pos-1] = v;
        Draw();
    }
}

void LCD5110Plotter::Draw() {
    for (uint16_t x = left; x < pos; ++x) {
        uint8_t v = data[x];
        if (v >= top && v < bottom) {
            display->setPixel(x, data[x]);
        }
    }
}

void LCD5110Plotter::Clear() {
    for (uint16_t x = left; x < pos; ++x) {
        uint8_t v = data[x];
        if (v >= top && v < bottom) {
            display->clrPixel(x, data[x]);
        }
    }
}

void LCD5110Plotter::ShiftData() {
    Clear();
    for (uint8_t x = left; x < pos-1; ++x) {
        data[x] = data[x+1];
    }
}