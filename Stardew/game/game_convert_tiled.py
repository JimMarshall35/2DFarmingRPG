#
#   Serialize entity types for the game from the tiled json file
#   plugs into and extends engine/scripts/ConvertTiled.py
#

import struct

from engine.scripts.ConvertTiled import (
    main,
    register_entity_serializer,
    get_tiled_object_custom_prop
)
########################################### helpers

def serialize_string(file, string):
    file.write(struct.pack("I", len(string)))
    for c in string:
        file.write(struct.pack("c", c.encode()))

########################################### wooded area

def serialize_WoodedArea(file, obj):
    file.write(struct.pack("I", 1)) # version
    file.write(struct.pack("f", get_tiled_object_custom_prop(obj, "ConiferousPercentage")["value"]))
    file.write(struct.pack("f", get_tiled_object_custom_prop(obj, "DeciduousPercentage")["value"]))
    file.write(struct.pack("f", get_tiled_object_custom_prop(obj, "PerMeterDensity")["value"]))
    file.write(struct.pack("f", obj["width"]))
    file.write(struct.pack("f", obj["height"]))

def get_type_WoodedArea(obj):
    return 6

########################################### player start

def serialize_PlayerStart(file, obj):
    file.write(struct.pack("I", 1)) # version
    stringVal = get_tiled_object_custom_prop(obj, "from")["value"]
    serialize_string(file, stringVal)
    stringVal = get_tiled_object_custom_prop(obj, "thisLocation")["value"]
    serialize_string(file, stringVal)
    bUseOldX = get_tiled_object_custom_prop(obj, "matchPreviousX")["value"]
    bUseOldY = get_tiled_object_custom_prop(obj, "matchPreviousY")["value"]
    file.write(struct.pack("I", 1 if bUseOldX else 0))
    file.write(struct.pack("I", 1 if bUseOldY else 0))


def get_type_PlayerStart(obj):
    return 4

########################################### exit

def serialize_Exit(file, obj):
    file.write(struct.pack("I", 1)) # version
    file.write(struct.pack("f", obj["width"]))
    file.write(struct.pack("f", obj["height"]))
    stringVal = get_tiled_object_custom_prop(obj, "to")["value"]
    serialize_string(file, stringVal)

def get_type_Exit(obj):
    return 5

########################################### debris field

def serialize_DebrisField(file, obj):
    file.write(struct.pack("I", 1)) # version
    file.write(struct.pack("f", get_tiled_object_custom_prop(obj, "BigRocksPercentage")["value"]))
    file.write(struct.pack("f", get_tiled_object_custom_prop(obj, "Density")["value"]))
    file.write(struct.pack("f", get_tiled_object_custom_prop(obj, "LogsPercentage")["value"]))
    file.write(struct.pack("f", get_tiled_object_custom_prop(obj, "RocksPercentage")["value"]))
    file.write(struct.pack("f", obj["width"]))
    file.write(struct.pack("f", obj["height"]))

def get_type_DebrisField(obj):
    return 7


########################################### registration

if __name__ == "__main__":
    register_entity_serializer("WoodedArea", serialize_WoodedArea, get_type_WoodedArea, False)
    register_entity_serializer("PlayerStart", serialize_PlayerStart, get_type_PlayerStart, False)
    register_entity_serializer("Exit", serialize_Exit, get_type_Exit, False)
    register_entity_serializer("DebrisField", serialize_DebrisField, get_type_DebrisField, False)
    main()