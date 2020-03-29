#include "icgis.h"
#include <QtWidgets/QApplication>

#include <ctime>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

    srand(time(nullptr));

	ICGis w(nullptr);
	w.setGeometry(100, 100, 1200, 700);
	w.show();
	return a.exec();
}
