-- Constants (Enums with C equivalents)
local TilemapType_Tile = 1
local TilemapType_Entity = 2

local DrawOrder_TopDown = 0
local DrawOrder_Index = 1

-- Constants
local TileSize = 32
local MaxMapSize = 100
local MinRoomSize = 5
local MaxRoomSize = 30

-- Named Tiles
local floor_tile = nil
local dungeon_vertical_wall_bottom_1_tile = nil
local dungeon_vertical_wall_middle_1_tile = nil
local dungeon_vertical_wall_top_1_tile = nil

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


function dump(o)
   if type(o) == 'table' then
      local s = '{ '
      for k,v in pairs(o) do
         if type(k) ~= 'number' then k = '"'..k..'"' end
         s = s .. '['..k..'] = ' .. dump(v) .. ','
      end
      return s .. '} '
   else
      return tostring(o)
   end
end



function AddFloorTileRect(pTileMap, topleft_x, topleft_y, width, height)
    for y = topleft_y, topleft_y + height - 1 do
        for x = topleft_x, topleft_x + width - 1 do
            TilemapSetTile(pTileMap, floor_tile, floor_tile_layer_i, x, y)
        end
    end
end

function LookupNamedTileIndices(hAtlas)
    floor_tile                          = LookupNamedTile(hAtlas, "dungeon_floor_1")
    dungeon_vertical_wall_bottom_1_tile = LookupNamedTile(hAtlas, "dungeon_vertical_wall_bottom_1")
    dungeon_vertical_wall_middle_1_tile = LookupNamedTile(hAtlas, "dungeon_vertical_wall_middle_1")
    dungeon_vertical_wall_top_1_tile    = LookupNamedTile(hAtlas, "dungeon_vertical_wall_top_1")
end

function SetupLayers(pTileMap)
    floor_tile_layer                    = TilemapAddLayer(pTileMap, TilemapType_Tile, 0, 0, 69)
    behind_player_walls_tile_layer      = TilemapAddLayer(pTileMap, TilemapType_Tile, 0, 0, 0)
    object_layer                        = TilemapAddLayer(pTileMap, TilemapType_Entity, 0, 0, DrawOrder_TopDown)
    in_front_of_player_walls_tile_layer = TilemapAddLayer(pTileMap, TilemapType_Tile, 0, 0, 0)
    wall_tops_tile_layer                = TilemapAddLayer(pTileMap, TilemapType_Tile, 0, 0, 0)
end

function RandomRoomFloorRect(borders)
    local t = math.random(borders.t, borders.t + ( MaxMapSize - (borders.t + borders.b)))
    local l = math.random(borders.l, borders.l + ( MaxMapSize - (borders.l + borders.r)))
    local b = t + math.random(MinRoomSize, MaxRoomSize)
    local r = l + math.random(MinRoomSize, MaxRoomSize)
    return {
        t = t,
        l = l,
        b = b,
        r = r
    }
end

function AABBIntersect(a, b)
    return a.l <= b.r and a.r >= b.l
       and a.t <= b.b and a.b >= b.t
end

function CheckForIntersects(adjustedRect, borders, rooms)
    for i, v in ipairs(rooms) do
        if AABBIntersect(v.adjustedRect, adjustedRect) then
            return true
        end
    end
    return false
end

function GetRoomFloorRects(numberOfRooms, borders)
    local rooms = {}
    for i = 1, numberOfRooms do
        local candidate = {}
        local adjustedRect = {}
        repeat
            candidate = RandomRoomFloorRect(borders)
            adjustedRect = {
                t = candidate.t - borders.t,
                l = candidate.l - borders.l,
                b = candidate.b + borders.b,
                r = candidate.r + borders.r
            }
        until not CheckForIntersects(adjustedRect, borders, rooms)
        rooms[#rooms + 1] = {
            floor = candidate,
            adjustedRect = adjustedRect
        }
    end
    dump(rooms)
    return rooms
end

function PlaceRoomFloorTiles(pTileMap, rooms)
    for i, v in ipairs(rooms) do
        AddFloorTileRect(pTileMap, v.floor.l, v.floor.t, (v.floor.r - v.floor.l), (v.floor.b - v.floor.t))
    end
end

function PlacePlayerStartInFirstRoom(rooms, pEntities)
    local playerX = (rooms[1].floor.l * TileSize) + ((rooms[1].floor.r - rooms[1].floor.l) * TileSize) / 2
    local playerY = (rooms[1].floor.t * TileSize) + ((rooms[1].floor.b - rooms[1].floor.t) * TileSize) / 2
    AddPlayerStartEntityAt("Farm", "dungeon", false, false, pEntities, playerX, playerY)
end

function Generate(pTileMap, pDC, hAtlas, pData, pUser, pEntities)
    LookupNamedTileIndices(hAtlas)
    SetupLayers(pTileMap)
    print("Generate")
    rooms = GetRoomFloorRects(4, {
        t = 3,
        l = 0,
        b = 0,
        r = 0
    })
    PlaceRoomFloorTiles(pTileMap, rooms)
    PlacePlayerStartInFirstRoom(rooms, pEntities)
end