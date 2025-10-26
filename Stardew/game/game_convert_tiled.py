#
#   Serialize entity types for the game from the tiled json file
#   plugs into and extends engine/scripts/ConvertTiled.py
#

import sys
import os
import struct

print(os.path.abspath("../Stardew/engine/scripts"))
sys.path.insert(1, os.path.abspath("../Stardew/engine/scripts"))  # add Folder_2 path to search list
from ConvertTiled import main, register_entity_serializer, get_tiled_object_custom_prop

########################################### helpers

def serialize_string(file, string):
    print(f"SERIALIZING STRING {string}")
    file.write(struct.pack("I", len(string)))
    for c in string:
        print(c.encode())
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

########################################### registration

register_entity_serializer("WoodedArea", serialize_WoodedArea, get_type_WoodedArea, False)
register_entity_serializer("PlayerStart", serialize_PlayerStart, get_type_PlayerStart, False)
register_entity_serializer("Exit", serialize_Exit, get_type_Exit, False)

main()