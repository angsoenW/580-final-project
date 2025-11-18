#include "EdgeDetectionShader.h"

IMPLEMENT_GLOBAL_SHADER(FEdgeDetectionShader, "/Plugins/StylizationPP/EdgeDetectionCS.usf", "MainCS", SF_Compute);