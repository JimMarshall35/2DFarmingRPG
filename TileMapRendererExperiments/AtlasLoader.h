#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "Tile.h"

struct TileSetInfo;
struct TileMapConfigOptions;

typedef u32 TileSetFrameHandle;

class IRenderer;

enum class AtlasLoaderStates {
	Unknown,
	Unloaded,
	Loading,
	FinishedLoading,
	// leave this one at the end
	NumStates
};

class AtlasLoader
{
public:
	AtlasLoader(const TileMapConfigOptions& configOptions, const std::shared_ptr<IRenderer>& renderer);
	void StartLoadingTilesets();
	void StopLoadingTilesets();
	bool TryLoadTileset(const TileSetInfo& info);
	void DebugDumpTiles(std::string path);
	// get a handle that can be used to free tiles that were loaded afterwards
	TileSetFrameHandle GetTilesetFrameHandle();
	// delete all tiles that were loaded after the handle was called
	void ReleaseTilesetFrameHandle(TileSetFrameHandle handle);
	inline const std::vector<Tile>& GetIndividualTiles() {
		return _individualTiles;
	}
	inline u32 GetAtlasTextureHandle() {
		return _atlasTextureHandle;
	}
	const inline std::map<TypeOfTile, u32>& GetrrayTexureMap() {
		return _arrayTextureForType;
	}; 
	u32 TestGetFirstArrayTexture() {
		for (auto& pair : _arrayTextureForType) {
			if ("sprites\\roguelikeCity_magenta.png") {
				return pair.second;
			}
		}
	}
private:
	u32 GetRequiredAtlasSizeInBytes(u32& rows);
	void MakeAtlas();
	void MakeAtlasAsArrayTexture();
	
private:
	std::vector<Tile> _individualTiles;
	std::map<TypeOfTile, u32> _numberOfEachTileType;
	std::map<TypeOfTile, u32> _arrayTextureForType;

	u32 _atlasWidth;
	u32 _atlasTextureHandle = 0;
	AtlasLoaderStates _currentState;
	std::shared_ptr<IRenderer> _renderer;
};

