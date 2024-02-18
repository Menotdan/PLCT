#include "CreatePLCTGraph.h"
#include "Engine/ContentImporters/AssetsImportingManager.h"

void CreatePLCTGraph::RegisterCreator()
{
#if COMPILE_WITH_ASSETS_IMPORTER
	AssetCreator creator = AssetCreator();
	creator.Tag = "PLCTGraph";
	creator.Callback = Create;
	if (AssetsImportingManager::GetCreator(creator.Tag) == nullptr)
	{
		AssetsImportingManager::Creators.Add(creator);
	}
#endif
}
