-- Constants
local TilemapType_Tile = 1
local TilemapType_Entity = 2

local DrawOrder_TopDown = 0
local DrawOrder_Index = 1

-- Named Tiles
local floor_tile = nil

-- Layer 0
local floor_tile_layer = nil
local floor_tile_layer_i = 0

-- layer 1
local behind_player_walls_tile_layer = nil
local behind_player_walls_tile_layer_i = 1

-- Layer 2
local object_layer = nil
local object_layer_i = 2

-- layer 3
local in_front_of_player_walls_tile_layer = nil
local in_front_of_player_walls_tile_layer_i = 3

-- layer 4
local wall_tops_tile_layer = nil
local wall_tops_tile_layer_i = 4

function AddFloorTileRect(pTileMap, topleft_x, topleft_y, width, height)
    for y = topleft_y, height do
        for x = topleft_x, width do
            TilemapSetTile(pTileMap, floor_tile, floor_tile_layer_i, x, y)
        end
    end
end

function Generate(pTileMap, pDC, hAtlas, pData, pUser, pEntities)
    print("FUCK OFF")
    print(pTileMap)
    floor_tile = LookupNamedTile(hAtlas, "dungeon_floor_1")
    print(floor_tile)
    floor_tile_layer                    = TilemapAddLayer(pTileMap, TilemapType_Tile, 0, 0, 69)
    behind_player_walls_tile_layer      = TilemapAddLayer(pTileMap, TilemapType_Tile, 0, 0, 0)
    object_layer                        = TilemapAddLayer(pTileMap, TilemapType_Entity, 0, 0, DrawOrder_TopDown)
    in_front_of_player_walls_tile_layer = TilemapAddLayer(pTileMap, TilemapType_Tile, 0, 0, 0)
    wall_tops_tile_layer                = TilemapAddLayer(pTileMap, TilemapType_Tile, 0, 0, 0)

    AddFloorTileRect(pTileMap, 0, 0, 20, 20)
    --from, thislocation, bUsePrevLocationX, bUsePrevLocationY, struct Entity2DCollection* pEntities, float x, float y
    AddPlayerStartEntityAt("Farm", "dungeon", false, false, pEntities, 10.0, 10.0)
end