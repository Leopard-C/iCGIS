#include "opengl/layermanager/opengllayerdescriptor.h"

OpenglFeatureLayerDescriptor::~OpenglFeatureLayerDescriptor()
{
	for (auto& featureDesc : featuresDescriptors)
		delete featureDesc;
}

OpenglFeatureDescriptor* OpenglFeatureLayerDescriptor::getFeatureDescriptor(int nFID) const
{
	for (const auto& featureDescriptor : featuresDescriptors) {
		if (featureDescriptor->FID == nFID) {
			return featureDescriptor;
		}
	}
	return nullptr;
}

