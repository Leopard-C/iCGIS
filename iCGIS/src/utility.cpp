#include "utility.h"

#include <cstring>
#include <sstream>

#include <QFileInfo>


/*************************************/
/*      Get filename fron filepath   */
/*************************************/
//int utils::getFileName(const char* filepath, char* fnameOut, int fnameOutSize)
//{
//	char drive[_MAX_DRIVE];
//	char dir[_MAX_DIR];
//	char fname[_MAX_FNAME];
//	char ext[_MAX_EXT];
//
//	_splitpath(filepath, drive, dir, fname, ext);
//
//	int size = strlen(fname);
//	size = size > fnameOutSize - 1 ? fnameOutSize - 1 : size;
//
//	strncpy_s(fnameOut, fnameOutSize, fname, size);
//
//	return size;
//}

QString utils::getFileName(const QString& filepath)
{
	QFileInfo fileInfo(filepath);
	return fileInfo.baseName();
}

//bool utils::replaceExt(const char* filepath, const char* newExt, char* fpathOut, int fpathOutSize)
//{
//	const char* res = strrchr(filepath, '.');
//	if (res) {
//		int npos = res - filepath;
//		char temp[_MAX_PATH] = { 0 };
//		strncpy_s(temp, _MAX_PATH, filepath, npos);
//		sprintf_s(fpathOut, fpathOutSize, "%s%s", temp, newExt);
//		return true;
//	}
//	else {
//		return false;
//	}
//}

QString utils::replaceExt(const QString& filepath, const QString& newExt)
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
unsigned int* utils::newContinuousNumber(unsigned int start, unsigned int count) {
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
void utils::hex2rgb(std::string hexCode, uchar& r, uchar& g, uchar& b) {
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
/*     Is two float number equal       */
/***************************************/
bool utils::isEqual(float a, float b)
{
	return fabs(a - b) < EPS;
}

/***************************************/
/*        Generate random Color        */
/***************************************/
utils::Color utils::getRandColor()
{
	uchar r = getRand(0, 256);
	uchar g = getRand(0, 256);
	uchar b = getRand(0, 256);
	return { r, g, b };
}


