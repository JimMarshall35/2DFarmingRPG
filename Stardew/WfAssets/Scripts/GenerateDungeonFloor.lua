-- Constants (Enums with C equivalents)
local TilemapType_Tile = 1
local TilemapType_Entity = 2

local DrawOrder_TopDown = 0
local DrawOrder_Index = 1

-- Constants
local TileSize = 32
local MaxMapSize = 100
local MinRoomSize = 10
local MaxRoomSize = 30
local RoomBorders = {
    t = 8,
    l = 8,
    b = 8,
    r = 8
}

-- Named Tiles
local floor_tile = nil
local dungeon_vertical_wall_bottom_1_tile = nil
local dungeon_vertical_wall_middle_1_tile = nil
local dungeon_vertical_wall_top_1_tile = nil
local dungeon_vertical_top_inside_left_middle_1_tile = nil
local dungeon_vertical_top_inside_right_middle_1_tile = nil

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
    floor_tile                                      = LookupNamedTile(hAtlas, "dungeon_floor_1")
    dungeon_vertical_wall_bottom_1_tile             = LookupNamedTile(hAtlas, "dungeon_vertical_wall_bottom_1")
    dungeon_vertical_wall_middle_1_tile             = LookupNamedTile(hAtlas, "dungeon_vertical_wall_middle_1")
    dungeon_vertical_wall_top_1_tile                = LookupNamedTile(hAtlas, "dungeon_vertical_wall_top_1")
    dungeon_vertical_top_inside_left_middle_1_tile  = LookupNamedTile(hAtlas, "dungeon_vertical_top_inside_left_middle_1")
    dungeon_vertical_top_inside_right_middle_1_tile = LookupNamedTile(hAtlas, "dungeon_vertical_top_inside_right_middle_1")
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

function SetCorridorFloorTile(pTileMap, x, y)
    TilemapSetTile(pTileMap, floor_tile, floor_tile_layer_i, x, y)
    local behindPlayerWallTile = GetTileAtXY(behind_player_walls_tile_layer_i, pTileMap, x, y)

    if behindPlayerWallTile ~= 0 then
        TilemapSetTile(pTileMap, 0, behind_player_walls_tile_layer_i, x, y)
        -- delete all of the wall that the tile is part of
        if behindPlayerWallTile == dungeon_vertical_wall_bottom_1_tile then
            TilemapSetTile(pTileMap, 0, behind_player_walls_tile_layer_i, x, y)
            TilemapSetTile(pTileMap, 0, behind_player_walls_tile_layer_i, x, y - 1)
            TilemapSetTile(pTileMap, 0, behind_player_walls_tile_layer_i, x, y - 2)
        elseif behindPlayerWallTile == dungeon_vertical_wall_middle_1_tile then
            TilemapSetTile(pTileMap, 0, behind_player_walls_tile_layer_i, x, y - 1)
            TilemapSetTile(pTileMap, 0, behind_player_walls_tile_layer_i, x, y)
            TilemapSetTile(pTileMap, 0, behind_player_walls_tile_layer_i, x, y + 1)
        elseif behindPlayerWallTile == dungeon_vertical_wall_top_1_tile then
            TilemapSetTile(pTileMap, 0, behind_player_walls_tile_layer_i, x, y)
            TilemapSetTile(pTileMap, 0, behind_player_walls_tile_layer_i, x, y + 1)
            TilemapSetTile(pTileMap, 0, behind_player_walls_tile_layer_i, x, y + 2)
        end
    end
    local inFrontOfPlayerWallTile = GetTileAtXY(in_front_of_player_walls_tile_layer_i, pTileMap, x, y)
    if inFrontOfPlayerWallTile ~= 0 then
        -- delete all of the wall that the tile is part of
        if inFrontOfPlayerWallTile == dungeon_vertical_wall_bottom_1_tile then
            TilemapSetTile(pTileMap, 0, in_front_of_player_walls_tile_layer_i, x, y)
            TilemapSetTile(pTileMap, 0, in_front_of_player_walls_tile_layer_i, x, y - 1)
            TilemapSetTile(pTileMap, 0, in_front_of_player_walls_tile_layer_i, x, y - 2)
        elseif inFrontOfPlayerWallTile == dungeon_vertical_wall_middle_1_tile then
            TilemapSetTile(pTileMap, 0, in_front_of_player_walls_tile_layer_i, x, y + 1)
            TilemapSetTile(pTileMap, 0, in_front_of_player_walls_tile_layer_i, x, y)
            TilemapSetTile(pTileMap, 0, in_front_of_player_walls_tile_layer_i, x, y - 1)
        elseif inFrontOfPlayerWallTile == dungeon_vertical_wall_top_1_tile then
            TilemapSetTile(pTileMap, 0, in_front_of_player_walls_tile_layer_i, x, y)
            TilemapSetTile(pTileMap, 0, in_front_of_player_walls_tile_layer_i, x, y + 1)
            TilemapSetTile(pTileMap, 0, in_front_of_player_walls_tile_layer_i, x, y + 2)
        end
    end
    if GetTileAtXY(wall_tops_tile_layer_i, pTileMap, x, y) ~= 0 then
        TilemapSetTile(pTileMap, 0, wall_tops_tile_layer_i, x, y)
    end

end

function AddWallAtCursorBase(cursor, pTileMap, layer)
    local cursor = { x = cursor.x, y = cursor.y}
    TilemapSetTile(pTileMap, dungeon_vertical_wall_bottom_1_tile, layer, cursor.x, cursor.y)
    TilemapSetTile(pTileMap, dungeon_vertical_wall_middle_1_tile, layer, cursor.x, cursor.y - 1)
    TilemapSetTile(pTileMap, dungeon_vertical_wall_top_1_tile,    layer, cursor.x, cursor.y - 2)

    TilemapSetTile(pTileMap, 0, wall_tops_tile_layer_i, cursor.x, cursor.y)
    TilemapSetTile(pTileMap, 0, wall_tops_tile_layer_i, cursor.x, cursor.y - 1)
    TilemapSetTile(pTileMap, 0, wall_tops_tile_layer_i, cursor.x, cursor.y - 2)

end

-- Make the floor of a four tile wide horizontal corridor
function FourWideCorridorHorizontal(pTileMap, cursor, potentialWallLocations)
    -- a corridor section "potentially" has walls
    potentialWallLocations.topWallTiles[#potentialWallLocations.topWallTiles + 1] = {
        x = cursor.x, y = cursor.y - 2
    }
    SetCorridorFloorTile(pTileMap, cursor.x, cursor.y - 1)
    SetCorridorFloorTile(pTileMap, cursor.x, cursor.y)
    SetCorridorFloorTile(pTileMap, cursor.x, cursor.y + 1)
    SetCorridorFloorTile(pTileMap, cursor.x, cursor.y + 2)
    potentialWallLocations.bottomWallTiles[#potentialWallLocations.bottomWallTiles + 1] = {
        x = cursor.x, y = cursor.y + 2
    }
end

-- Make the floor of a four tile wide vertical corridor
function FourWideCorridorVertical(pTileMap, cursor, potentialWallLocations)
    potentialWallLocations.leftWallTiles[#potentialWallLocations.leftWallTiles + 1] = {
        x = cursor.x - 1, y = cursor.y
    } 
    SetCorridorFloorTile(pTileMap, cursor.x - 1, cursor.y)
    SetCorridorFloorTile(pTileMap, cursor.x,     cursor.y)
    SetCorridorFloorTile(pTileMap, cursor.x + 1, cursor.y)
    SetCorridorFloorTile(pTileMap, cursor.x + 2, cursor.y)
    potentialWallLocations.rightWallTiles[#potentialWallLocations.rightWallTiles + 1] = {
        x = cursor.x + 2, y = cursor.y
    } 
end


function LinkRooms(roomA, roomB, rooms, pTileMap, potentialWallLocations)
    
    -- roomA and roombB are indices into rooms
    local roomAVal = rooms[roomA].floor
    local roomBVal = rooms[roomB].floor

    local centerA = {
        x = roomAVal.l + ((roomAVal.r - roomAVal.l) // 2),
        y = roomAVal.t + ((roomAVal.b - roomAVal.t) // 2)
    }

    local centerB = {
        x = roomBVal.l + ((roomBVal.r - roomBVal.l) // 2),
        y = roomBVal.t + ((roomBVal.b - roomBVal.t) // 2)
    }

    local cursor = { x = centerA.x, y = centerA.y }
    local incr = 0

    -- tunnel x
    if centerA.x > centerB.x then
        incr = -1
    else
        incr = 1
    end

    -- This implementation implicitly hard codes a corridor width of 4
    while cursor.x ~= centerB.x do
        FourWideCorridorHorizontal(pTileMap, cursor, potentialWallLocations)
        cursor.x = cursor.x + incr
    end

    -- tunnel y
    if centerA.y > centerB.y then
        -- "back fill" to produce squared off edges
        local backfillCursor = 
        {
            x = cursor.x,
            y = cursor.y + 1
        }

        FourWideCorridorVertical(pTileMap, backfillCursor, potentialWallLocations)
        
        backfillCursor = 
        {
            x = cursor.x,
            y = cursor.y + 2
        }
        FourWideCorridorVertical(pTileMap, backfillCursor, potentialWallLocations)

        potentialWallLocations.bottomWallTiles[#potentialWallLocations.bottomWallTiles + 1] = {
            x = backfillCursor.x - 1, y = backfillCursor.y
        } 
        potentialWallLocations.bottomWallTiles[#potentialWallLocations.bottomWallTiles + 1] = {
            x = backfillCursor.x, y = backfillCursor.y
        }
        potentialWallLocations.bottomWallTiles[#potentialWallLocations.bottomWallTiles + 1] = {
            x = backfillCursor.x + 1, y = backfillCursor.y
        }
        potentialWallLocations.bottomWallTiles[#potentialWallLocations.bottomWallTiles + 1] = {
            x = backfillCursor.x + 2, y = backfillCursor.y
        }

        incr = -1
    else
        -- "back fill" to produce squared off edges
        local backfillCursor = 
        {
            x = cursor.x,
            y = cursor.y - 1
        }

        FourWideCorridorVertical(pTileMap, backfillCursor, potentialWallLocations)

        potentialWallLocations.topWallTiles[#potentialWallLocations.topWallTiles + 1] = {
            x = backfillCursor.x, y = backfillCursor.y - 1
        }
        potentialWallLocations.topWallTiles[#potentialWallLocations.topWallTiles + 1] = {
            x = backfillCursor.x + 1, y = backfillCursor.y - 1
        }
        potentialWallLocations.topWallTiles[#potentialWallLocations.topWallTiles + 1] = {
            x = backfillCursor.x + 2, y = backfillCursor.y - 1
        }

        incr = 1
    end

    while cursor.y ~= centerB.y do
        FourWideCorridorVertical(pTileMap, cursor, potentialWallLocations)
        cursor.y = cursor.y + incr
    end
end

function AddCorridorWalls(pTileMap, potentialWallLocations)
    for i, v in ipairs(potentialWallLocations.topWallTiles) do
        if GetTileAtXY(floor_tile_layer_i, pTileMap, v.x, v.y) == 0 then
            AddWallAtCursorBase(v, pTileMap, behind_player_walls_tile_layer_i)
        end
    end
    for i, v in ipairs(potentialWallLocations.bottomWallTiles) do
        if GetTileAtXY(floor_tile_layer_i, pTileMap, v.x, v.y + 1) == 0 then
            AddWallAtCursorBase(v, pTileMap, in_front_of_player_walls_tile_layer_i)
        end
    end
    for i, v in ipairs(potentialWallLocations.leftWallTiles) do
        if GetTileAtXY(floor_tile_layer_i, pTileMap, v.x - 1, v.y) == 0 then
            TilemapSetTile(pTileMap, dungeon_vertical_top_inside_left_middle_1_tile, wall_tops_tile_layer_i, v.x, v.y)
        end
    end
    for i, v in ipairs(potentialWallLocations.rightWallTiles) do
        if GetTileAtXY(floor_tile_layer_i, pTileMap, v.x + 1, v.y) == 0 then
            TilemapSetTile(pTileMap, dungeon_vertical_top_inside_right_middle_1_tile, wall_tops_tile_layer_i, v.x, v.y)
        end
    end

end

function LinkAllRooms(rooms, pTileMap)
    local potentialWallLocations = {
        topWallTiles = {},
        bottomWallTiles = {},
        leftWallTiles = {},
        rightWallTiles = {}
    }
    for i, v in ipairs(rooms) do
        if i + 1 <= #rooms then
            LinkRooms(i, i + 1, rooms, pTileMap, potentialWallLocations)
        end
    end 
    AddCorridorWalls(pTileMap, potentialWallLocations)
end


function AddWallsToRoom(room, pTileMap)
    local backWallCursor = {
        x = room.floor.l,
        y = room.floor.t - 1
    }
    for i = 1, room.floor.r - room.floor.l do
        AddWallAtCursorBase(backWallCursor, pTileMap, behind_player_walls_tile_layer_i)
        backWallCursor.x = backWallCursor.x + 1
    end

    local frontWallCursor = {
        x = room.floor.l,
        y = room.floor.t + (room.floor.b - room.floor.t) - 1
    }

    for i = 1, room.floor.r - room.floor.l do
        AddWallAtCursorBase(frontWallCursor, pTileMap, in_front_of_player_walls_tile_layer_i)
        frontWallCursor.x = frontWallCursor.x + 1
    end

    local leftWallCursor = {
        x = room.floor.l,
        y = room.floor.t - 3
    }

    for i = 1, (room.floor.b - room.floor.t) do
        TilemapSetTile(pTileMap, dungeon_vertical_top_inside_left_middle_1_tile, wall_tops_tile_layer_i, leftWallCursor.x, leftWallCursor.y)
        leftWallCursor.y = leftWallCursor.y + 1
    end 

    local rightWallCursor = {
        x = room.floor.l + (room.floor.r - room.floor.l) - 1,
        y = room.floor.t - 3
    }

    for i = 1, (room.floor.b - room.floor.t) do
        TilemapSetTile(pTileMap, dungeon_vertical_top_inside_right_middle_1_tile, wall_tops_tile_layer_i, rightWallCursor.x, rightWallCursor.y)
        rightWallCursor.y = rightWallCursor.y + 1
    end 

end

function AddWallsToRooms(rooms, pTileMap)
    for i, v in ipairs(rooms) do
        AddWallsToRoom(v, pTileMap)
    end 
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
    AddWallsToRooms(rooms, pTileMap)
    LinkAllRooms(rooms, pTileMap)
    PlacePlayerStartInFirstRoom(rooms, pEntities)
end