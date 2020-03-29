#include "sld.h"

#include "util/utility.h"
#include "util/logger.h"

#include "geo/map/geolayer.h"

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>


bool SLD::read(const QString& filepath, SLDInfo** sldInfoOut /* = nullptr */)
{
    SLDInfo* sldInfo = new SLDInfo();

    bool ret = false;

    do {
        QByteArray bytes = filepath.toLocal8Bit();
        const char* path = bytes.constData();

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(path);
        if (result.status != pugi::status_ok)
            break;

        Node XStyleLayerDescriptor = doc.child("StyledLayerDescriptor");
        if (XStyleLayerDescriptor.empty())
            break;

        Node XNamedLayer = XStyleLayerDescriptor.child("NamedLayer");
        if (XNamedLayer.empty())
            break;

        if (!parseNamedLayer(XNamedLayer, sldInfo))
            break;

        ret = true;
    } while (false);

    if (sldInfoOut && ret)
        *sldInfoOut = sldInfo;
    else
        delete sldInfo;

    return ret;
}

bool SLD::parseNamedLayer(const Node& XNamedLayer, SLDInfo* sldInfoOut)
{
    Node XLayerName = XNamedLayer.child("se:Name");
    if (XLayerName.empty()) {
        return false;
    }

    std::string layerName = XLayerName.text().as_string();

    Node XUserStyle = XNamedLayer.child("UserStyle");
    if (XUserStyle.empty()) {
        return false;
    }

    return parseUserStyle(XUserStyle, sldInfoOut);
}

bool SLD::parseUserStyle(const Node& XUserStyle, SLDInfo* sldInfoOut)
{
    Node XStyleName = XUserStyle.child("se:Name");
    if (XStyleName.empty()) {
        return false;
    }

    std::string styleName = XStyleName.text().as_string();

    Node XFeatureTypeStyle = XUserStyle.child("se:FeatureTypeStyle");
    if (XFeatureTypeStyle.empty()) {
        return false;
    }

    return parseFeatureTypeStyle(XFeatureTypeStyle, sldInfoOut);
}

bool SLD::parseFeatureTypeStyle(const Node& XFeatureTypeStyle, SLDInfo* sldInfoOut)
{
    int hasReadFieldInfo = false;

    for (const auto& XRule : XFeatureTypeStyle) {
        Node XName = XRule.child("se:Name");
        if (XName.empty()) {
            return false;
        }

        Node XDescription = XRule.child("se:Description");
        if (XDescription.empty()) {
            return false;
        }

        Node XTitle = XDescription.child("se:Title");
        if (XTitle.empty()) {
            return false;
        }

        Node XOgcFilter = XRule.child("ogc:Filter");
        if (XOgcFilter.empty()) {
            return false;
        }

        Node XPropertyIsEqualTo = XOgcFilter.child("ogc:PropertyIsEqualTo");
        if (XPropertyIsEqualTo.empty()) {
            continue;
        }

        Node XPropertyName = XPropertyIsEqualTo.child("ogc:PropertyName");
        if (XPropertyName.empty()) {
            continue;
        }

        std::string fieldName = XPropertyName.text().as_string();

        // read filed info
        // only once
        if (!hasReadFieldInfo) {
            hasReadFieldInfo = true;
            GeoFieldDefn* fieldDefn = layer->getFieldDefn(fieldName.c_str());
            if (fieldDefn) {
                sldInfoOut->setFieldName(fieldName.c_str());
                sldInfoOut->setFieldType(fieldDefn->getType());
            }
            else {
                return false;
            }
        }

        // Do not read field's value now
        // Read fillcolor first
        QColor fillColor;
        Node XPolygonSymbolizer = XRule.child("se:PolygonSymbolizer");
        if (XPolygonSymbolizer.empty()) {
            return false;
        }

        Node XFill = XPolygonSymbolizer.child("se:Fill");
        if (XFill.empty()) {
            return false;
        }

        Node XSvgParameter = XFill.child("se:SvgParameter");
        if (XSvgParameter.empty()) {
            return false;
        }

        for (const auto& parmeter : XFill) {
            std::string name = parmeter.attribute("name").as_string();
            if (name == "fill") {
                std::string fillColorString = parmeter.text().as_string();
                unsigned char r, g, b;
                utils::hex2rgb(fillColorString, r, g, b);
                fillColor.setRgb(r, g, b);
            }
        }


        Node XStroke = XPolygonSymbolizer.child("se:Stroke");
        if (XStroke.empty()) {
            return false;
        }

        for (const auto& parmeter : XStroke) {
            std::string name = parmeter.attribute("name").as_string();
            if (name == "stroke") {
                std::string strokeColor = parmeter.text().as_string();
            }
            else if (name == "stroke-width") {
                int strokeWidth = parmeter.text().as_int();
                log(strokeWidth);
            }
            else if (name == "stroke-linejoin") {
                std::string strokeLinejoin = parmeter.text().as_string();
                if (strokeLinejoin == "bevel") {

                }
                else if (strokeLinejoin == "miter") {

                }
                else if (strokeLinejoin == "round") {

                }
            }
        }

        // Read field's value
        Node XLiteral = XPropertyIsEqualTo.child("ogc:Literal");
        if (XLiteral.empty()) {
            return false;
        }

        switch (sldInfoOut->fieldType) {
        default:
            break;
        case kFieldInt:
        {
            int fieldValue = XLiteral.text().as_int();
            sldInfoOut->rules.emplace_back(new int(fieldValue), fillColor);
            break;
        }
        case kFieldDouble:
        {
            double fieldValue = XLiteral.text().as_double();
            sldInfoOut->rules.emplace_back(new double(fieldValue), fillColor);
            break;
        }
        case kFieldText:
        {
            QString fieldValue = XLiteral.text().as_string();
            sldInfoOut->rules.emplace_back(new QString(fieldValue), fillColor);
            break;
        }
        }
    }

    return true;
}
