/*******************************************************
** namespace:  utils
**
** description:	一些常用的函数
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

// 计算两点间的距离(的平方)
#define DIS_SQURE(x1, y1, x2, y2) ((x1)-(x2))*((x1)-(x2))+((y1)-(y2))*((y1)-(y2))

// 打印数据类型的大小（字节数）
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

	// 指针转换，失败返回nullptr
	template<typename To, typename From>
	inline To down_cast(From* f) {
		return dynamic_cast<To>(f);
	}

	// 比较float相等的精度
	const float EPS = 1e-6;

	// 从文件路径获取文件名
	QString getFileName(const QString& filepath);

	// 替换文件路径的扩展名
	QString replaceExt(const QString& filepath, const QString& newExt);

	// 生成连续数值串
	unsigned int* newContinuousNumber(unsigned int start, unsigned int count);

	// 16进制颜色转RGB
	// 如 "#FF00FF" 转为 255, 0, 255
	void hex2rgb(std::string hexCode, uchar& r, uchar& g, uchar& b);

	// RGB 颜色结构体
	struct Color {
		Color() {}
		Color(uchar rr, uchar gg, uchar bb) : r(rr), g(gg), b(bb) {}
		void set(uchar rr, uchar gg, uchar bb) { r = rr; g = gg; b = b; }
		uint toUInt() { return (uint)(((uint)r << 16) | (ushort)(((ushort)g << 8  ) | b)); }
		uchar r = 255;
		uchar g = 255;
		uchar b = 255;
	};

	// 两个float是否相等
	bool isEqual(float a, float b);

	// 生成随机数
	template<typename T>
	inline T getRand(T lower, T upper)
		{ return rand() / (double)RAND_MAX *(upper - lower) + lower; }
	Color getRandColor();

	// 计算中位数
	template<typename T>
	inline T getMedian(T* values, int size) {
		std::sort(values, values + size);
		if (size % 2 == 0)
			return values[size / 2];
		else
			return (values[size / 2 - 1] + values[size / 2]) / 2.0;
	}

} // end namespace utils

