/*******************************************************
** namespace:  utils
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include <string>
#include <QString>

#define PI 3.1415926

#ifndef MIN
#define MIN(x, y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#define MAX(x, y) ((x)>(y)?(x):(y))
#endif

// Square of distance between two points
#define DIS_SQURE(x1, y1, x2, y2) ((x1)-(x2))*((x1)-(x2))+((y1)-(y2))*((y1)-(y2))

#define PrintSize(x) std::cout << "sizeof("#x")" << ": " << sizeof(x) << std::endl


namespace utils {

using uchar = unsigned char;
using uint = unsigned int;
using ushort = unsigned short;

enum DataType {
    kUnknown	= 0,
    kByte		= 1,
    kInt		= 2,
    kUInt		= 3,
    kFloat		= 4,
    kDouble		= 5
};

template<typename To, typename From>
inline To down_cast(From* f) {
    return dynamic_cast<To>(f);
}

// Get file name from absolute path
QString getFileName(const QString& filepath);

// Replace the suffix of filepath
QString replaceExt(const QString& filepath, const QString& newExt);

// Generate continuois number
unsigned int* newContinuousNumber(unsigned int start, unsigned int count);

// color transform
void hex2rgb(std::string hexCode, uchar& r, uchar& g, uchar& b);

// Compare float numbers
bool isEqual(double a, double b, double eps = 1e-6);

// Generate random number
template<typename T>
inline T getRand(T lower, T upper)
{ return rand() / (double)RAND_MAX *(upper - lower) + lower; }

// Calculate the median
template<typename T>
inline T getMedian(T* values, int size) {
    std::sort(values, values + size);
    if (size % 2 == 0)
        return values[size / 2];
    else
        return (values[size / 2 - 1] + values[size / 2]) / 2.0;
}

unsigned int getRandomColor();

} // end namespace utils

