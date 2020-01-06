#pragma once

#include <vector>

#include <QString>
#include <QColor>

#include <pugixml/pugiconfig.hpp>
#include <pugixml/pugixml.hpp>

#include "geo/map/geofielddefn.h"
#include "utility.h"

class GeoFeatureLayer;


struct SLDInfo {
	struct Rule {
		Rule(void* valueIn, QColor colorIn, int lineWidthIn = 1) :
			fieldValue(valueIn), fillColor(colorIn), lineWidth(lineWidthIn) {}
		void* fieldValue;
		QColor fillColor;
		int lineWidth;
	};
	SLDInfo() {}
	~SLDInfo() {
		for (auto& pair : rules) {
			switch (fieldType) {
			case kFieldInt:    delete (int*)pair.fieldValue; break;
			case kFieldDouble: delete (double*)pair.fieldValue; break;
			case kFieldText:   delete (QString*)pair.fieldValue; break;
			case kFieldUnknown:	break;
			default: break;
			}
		}
	}

	void setFieldName(const QString& name) { fieldName = name; }
	void setFieldType(GeoFieldType fieldTypeIn) { fieldType = fieldTypeIn; }

	template<typename T>
	Rule* getRule(T value) {
		int rulesCount = rules.size();
		for (int iRule = 0; iRule < rulesCount; ++iRule) {
			if (*(T*)(rules[iRule].fieldValue) == value) {
				return &(rules[iRule]);
			}
		}
		return nullptr;
	}

	QString fieldName;
	GeoFieldType fieldType;
	std::vector<Rule> rules;
};


class SLD {
public:
	SLD(GeoFeatureLayer* layer) : layer(layer) {}
	~SLD() {}
	using Node = pugi::xml_node;
public:
	bool read(const QString& filepath, SLDInfo** sldInfoOut = nullptr);

private:
	bool parseNamedLayer(const Node& XNamedLayer, SLDInfo* sldInfoOut);
	bool parseUserStyle(const Node& XUserStyle, SLDInfo* sldInfoOut);
	bool parseFeatureTypeStyle(const Node& XFeatureTypeStyle, SLDInfo* sldInfoOut);

private:
	GeoFeatureLayer* layer;
};
