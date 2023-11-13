#pragma once

#include "MassRepresentationTypes.h"
#include "ExMassInstancedStaticMeshInfo.generated.h"

class UExMassVisualizationComponent;

USTRUCT()
struct FExMassInstancedStaticMeshInfo : public FMassInstancedStaticMeshInfo
{
	GENERATED_BODY()
protected:
	friend class UExMassVisualizationComponent;
};
