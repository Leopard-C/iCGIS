#include "utility.h"

#include <cstring>
#include <sstream>
#include <QRgb>

#include <QFileInfo>
#include <cmath>


namespace utils {

QString getFileName(const QString& filepath)
{
    QFileInfo fileInfo(filepath);
    return fileInfo.baseName();
}

QString replaceExt(const QString& filepath, const QString& newExt)
{
    int indexOfDot = filepath.lastIndexOf(".");
    int indexOfSlash = 0;
#ifdef WIN32
    indexOfSlash = filepath.lastIndexOf("\\");
#else
    indexOfSlash = filepath.lastIndexOf("/");
#endif

    if (indexOfDot > indexOfSlash) {
        if (newExt.indexOf(".") == -1)
            return filepath.left(indexOfDot + 1) + newExt;
        else
            return filepath.left(indexOfDot) + newExt;
    }
    else {
        if (newExt.indexOf(".") == -1)
            return filepath + "." + newExt;
        else
            return filepath + newExt;
    }
}

/************************************/
/*    Generate Continuous Number    */
/*    [start, start+bandsCount)          */
/************************************/
unsigned int* newContinuousNumber(unsigned int start, unsigned int count) {
    if (count < 1)
        return nullptr;
    else {
        unsigned int* ret = new unsigned int[count];
        for (unsigned int i = 0; i < count; ++i) {
            ret[i] = start + i;
        }
        return ret;
    }
}

/************************************/
/*   Color in hex convert ot RGB    */
/************************************/
void hex2rgb(std::string hexCode, uchar& r, uchar& g, uchar& b) {
    if (hexCode[0] == '#')
        hexCode.erase(0, 1);
    if (hexCode.size() < 6)
        return;

    int iR, iG, iB;

    std::istringstream(hexCode.substr(0, 2)) >> std::hex >> iR;
    std::istringstream(hexCode.substr(2, 2)) >> std::hex >> iG;
    std::istringstream(hexCode.substr(4, 2)) >> std::hex >> iB;

    r = uchar(iR);
    g = uchar(iG);
    b = uchar(iB);
}

/***************************************/
/*     Compare double numbers          */
/***************************************/
bool isEqual(double a, double b, double eps/* = EPS*/) {
    return fabs(a - b) < eps;
}

/***************************************/
/*        Generate random Color        */
/***************************************/
//Color getRandColor()
//{
//	uchar r = getRand(0, 256);
//	uchar g = getRand(0, 256);
//	uchar b = getRand(0, 256);
//	return { r, g, b };
//}

QRgb getRandomColor() {
    return qRgb(getRand(50, 200), getRand(50, 200), getRand(50, 200));
}

} // namespace util
