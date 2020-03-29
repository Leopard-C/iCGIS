#include "geo/tool/geotool.h"
#include "util/env.h"

GeoTool::GeoTool(QWidget* parent)
    : QDialog(parent), map(Env::map) {

}

GeoTool::~GeoTool() {

}
