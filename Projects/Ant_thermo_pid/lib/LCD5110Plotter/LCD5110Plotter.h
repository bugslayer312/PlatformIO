#ifndef LCD5110Plotter_h
#define LCD5110Plotter_h

#include <stdint.h>

class LCD5110;

#define MAX_WIDTH 84
#define MAX_HEIGHT 48

class LCD5110Plotter {
public:
    LCD5110Plotter(LCD5110* lcd, float minValue, float maxValue);
    void InitSize(uint8_t left, uint8_t top, uint8_t right, uint8_t bottom);
    void Push(float newValue);

protected:
    void ShiftData();
    void Draw();
    void Clear();

private:
    LCD5110* display;
    float minValue;
    float maxValue;
    uint8_t left;
    uint8_t top;
    uint8_t right;
    uint8_t bottom;
    uint8_t pos;
    uint8_t data[MAX_WIDTH];
};

#endif