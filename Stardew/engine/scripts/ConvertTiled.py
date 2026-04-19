import argparse
import json
import xml.etree.ElementTree as ET
import typing
import os
import subprocess
import struct
import platform


TILEMAP_FILE_VERSION = 1

OBJECTS_FILE_VERSION = 1

ATLAS_FILE_VERSION = 1

def do_cmd_args():
    parser = argparse.ArgumentParser(
                    prog='ConvertTiled',
                    description='convert data files from the level editor program "tiled" into our game engine format',
                    epilog='Jim Marshall - 2025')
    parser.add_argument('-m', '--tile_maps', nargs='+', default=[], help="list of tilemap json files output from Tiled. In total, an atlas file for all levels and a level binary file for each level will be output")
    parser.add_argument("-a", '--atlas_tool', default=None, help="optional path to atlas tool which will compile an atlas xml file AOT for faster loading")
    parser.add_argument("outputDir", type=str, help="the output directory")
    parser.add_argument("-A", "--assets_folder", default="./WfAssets/")
    parser.add_argument("-r", "--rle", type=bool, default=False)
    parser.add_argument("-bmp", "--atlasBmp", type=str, default=None, help="Optional atlas debug bitmap output path")
    parser.add_argument("-iw", "--atlasIW", type=int, default=512, help="Atlas initial width.")
    parser.add_argument("-ih", "--atlasIH", type=int, default=512, help="Atlas initial height.")
    parser.add_argument("-iqtlx", type=float, default=-200)
    parser.add_argument("-iqtly", type=float, default=-200)
    parser.add_argument("-iqbrx", type=float, default=10200.0)
    parser.add_argument("-iqbry", type=float, default=10200.0)

    args = parser.parse_args()
    return args

def find_tileset(index: int, tilesets: object) -> object:
    for t in tilesets:
        if index >= t["firstgid"]:
            return t
        
def parse_tileset_file(path):
    with open(path, "r") as f:
        ts = json.load(f)
        return ts

def get_normalized_index(index: int, tileset: object) -> int:
    return (index - tileset["firstgid"]) + 1

def get_named_tiles(parsed_tile_maps) -> dict[str, (int,str)]:
    d = dict()
    for p in parsed_tile_maps:
        map_dir = os.path.dirname(os.path.abspath(p["srcPath"]))
        for t in p["tilesets"]:
            ts_path = os.path.join(map_dir, t["source"])
            ts = parse_tileset_file(ts_path)
            if "tiles" in ts:
                for tile in ts["tiles"]:
                    d[tile["type"]] = (tile["id"] + 1, t["source"]) 
    return d

def get_tiles_used_per_tileset(parsed_tile_maps) -> dict[str, set[int]]:
    used_tiles = dict()
    for p in parsed_tile_maps:
        tilesets = p["tilesets"]
        tilesets = sorted(tilesets, key=lambda tileset: tileset["firstgid"])   # sort by age
        tilesets.reverse()
        for t in tilesets:
            t["tiles"] = set()
        for layer in p["layers"]:
            if "data" in layer:
                for index in layer["data"]:
                    tileset = find_tileset(index, tilesets)
                    if tileset:
                        tileset["tiles"].add(get_normalized_index(index, tileset))
        for t in tilesets:
            if t["source"] in used_tiles:
                used_tiles[t["source"]] = used_tiles[t["source"]].union(t["tiles"])
            else:
                used_tiles[t["source"]] = t["tiles"]
    return used_tiles

class AtlasSprite:
    def __init__(self, path, top, left, width, height, name):
        self.path = path
        self.top = top
        self.left = left
        self.width = width
        self.height = height
        self.name = name
    def get_attributes(self, counter):
        return {
            "source" : "./WfAssets/" + self.path,
            "top" : str(int(self.top)),
            "left" : str(int(self.left)),
            "width" : str(int(self.width)),
            "height" : str(int(self.height)),
            "name" : f"{self.name}_{counter}"
        }

class Atlas:
    sprites: list[AtlasSprite]
    lut: dict[str, dict[int, int]] # by path, a lookup table from normalized index to index within the atlas
    def get_atlas_index(self, normalized_index, path):
        assert path in self.lut
        assert normalized_index in self.lut[path]
        return self.lut[path][normalized_index]
    def output_atlas_xml_file(self, outPath):
        print(f"outputting atlas xml file. The engine can load this directly or you can use the Atlas tool to convert it into a binary file.\n This script will try to invoke the atlas tool if its path is passed with the flag -a")
        counter = 0
        top = ET.Element("atlas")
        tree = ET.ElementTree(top)
        top.set("tilesetStart",f"{counter}")
        print(f"tilesetStart: {counter}")
        for s in self.sprites:
            ET.SubElement(top, "sprite", s.get_attributes(counter))
            counter += 1
        if len(self.named_tiles) > 0:
            nt = ET.SubElement(top, "named-tiles")
            print("named tiles:")
            for t in self.named_tiles:
                print(f"name {t} index {self.get_atlas_index(self.named_tiles[t][0], self.named_tiles[t][1])}")
                attr = {
                    "name": t,
                    "index": str(self.get_atlas_index(self.named_tiles[t][0], self.named_tiles[t][1]))
                }
                m = ET.SubElement(nt, "mapping", attr)
        top.set("tilesetEnd",f"{counter}")
        print(f"tilesetEnd: {counter}")
        print(f"Num Sprites: {len(self.sprites)}")
        ET.indent(tree, space=" ", level=3)
        with open(outPath, "wb") as f:
            f.write(ET.tostring(top))
    def make_sprites(self):
        for key in self.originalNormalizedIndexes:
            with open(os.path.join(self.args.assets_folder, key), "r") as f:
                j = json.load(f)
                imgWidth = j["imagewidth"]
                imgHeight = j["imageheight"]
                tileHeight = j["tileheight"]
                tileWidth = j["tilewidth"]
                margin = j["margin"]
                spacing = j["spacing"]
                columns = j["columns"]
                name = j["name"]
                image = j["image"]
                for index in self.originalNormalizedIndexes[key]:
                    l = ((index - 1) % columns) * tileWidth  + spacing + margin
                    t = ((index - 1) // columns) * tileHeight + spacing + margin
                    if not key in self.lut:
                        self.lut[key] = dict()
                    self.lut[key][index] = len(self.sprites) + 1
                    self.sprites.append(AtlasSprite(image, t, l, tileWidth, tileHeight, name))
    def __init__(self, originalNormalizedIndexes : dict[str, set[int]], namedTiles : dict[str,int], args):
        self.originalNormalizedIndexes = originalNormalizedIndexes
        self.sprites = []
        self.lut = dict()
        self.args = args
        self.named_tiles = namedTiles
        self.make_sprites()

def run_atlas_tool(path, xmlPath, args):
    binPath = os.path.join(os.path.abspath(args.outputDir), "main.atlas")
    argsList = None
    if platform.system() == "Windows":
        argsList = [os.path.abspath(path), xmlPath, "-o", binPath, "-iw", str(args.atlasIW), "-ih", str(args.atlasIH)]
    elif platform.system() == "Linux":
        argsList = [xmlPath, "-o", binPath, "-iw", str(args.atlasIW), "-ih", str(args.atlasIH)] 
    if args.atlasBmp:
        argsList.append("-bmp")
        argsList.append(args.atlasBmp)
    result = subprocess.run(argsList, capture_output=True, cwd=os. getcwd(), shell=True)
    print(f"Running Atlas Tool...\n")
    print(binPath)
    print(argsList)
    print(f"Std Out:\n{result.stdout.decode('utf-8')}\n")
    print(f"Std Err:\n{result.stderr.decode('utf-8')}\n")

def count_tilemap_layers(layers):
    i = 0
    for layer in layers:
        if "data" in layer:
            i += 1
    return i

U16MAX = 65535

class Run:
    def __init__(self, file, runLen, runVal):
        self.file = file
        self.runLen = runLen
        self.runVal = runVal
    def write(self):
        # H == u16
        self.file.write(struct.pack("H", self.runLen))
        self.file.write(struct.pack("H", self.runVal))

def build_runs(file, data : list[int], atlas : Atlas, tilesets) -> list[Run]:
    i = 0
    runs = []
    ts = find_tileset(data[i], tilesets)
    if ts:
            norm = get_normalized_index(data[i], ts)
            converted = atlas.get_atlas_index(norm, ts["source"])
    else:
        converted = 0
    currentVal = converted
    currentCount = 1
    for i in range(1, len(data)):
        ts = find_tileset(data[i], tilesets)
        if ts:
            norm = get_normalized_index(data[i], ts)
            converted = atlas.get_atlas_index(norm, ts["source"])
        else:
            converted = 0
        if converted != currentVal or currentVal == U16MAX:
            runs.append(Run(file, currentCount, currentVal))
            currentCount = 1
            currentVal = converted
        else:
            currentCount += 1
    # sentinel value: run of length 0
    runs.append(Run(file, 0, 0))
    return runs

def get_tile_layer_tile_dims(layer, atlas, tilesets):
    lastW = -1
    lastH = -1
    if "properties" in layer:
        props = layer["properties"]
        tileSizeXProps = [x["value"] for x in props if x["name"] == "TileSizeX"]
        tileSizeYProps = [x["value"] for x in props if x["name"] == "TileSizeY"]
        assert (len(tileSizeXProps) == 1 or len(tileSizeXProps) == 0)
        if len(tileSizeXProps) == 1:
            assert len(tileSizeYProps) == 1
        else:
            assert len(tileSizeYProps) == 0
        print("tile dims specified by custom property")
        return tileSizeXProps[0], tileSizeYProps[0]
        
    data = layer["data"]
    for i in data:
        ts = find_tileset(i, tilesets)
        if ts:
            norm = get_normalized_index(i, ts)
            converted = atlas.get_atlas_index(norm, ts["source"])
            if lastW == -1:
                assert lastH == -1
                if converted > 0 and converted < len(atlas.sprites):
                    lastH = atlas.sprites[converted].height
                    lastW = atlas.sprites[converted].width
            elif converted > 0 and converted < len(atlas.sprites):
                assert lastH == atlas.sprites[converted].height
                assert lastW == atlas.sprites[converted].width
                lastH = atlas.sprites[converted].height
                lastW = atlas.sprites[converted].width
        else:
            pass
            #assert False
    return lastW, lastH


def write_rle(f, data, atlas, tilesets):
    runs = build_runs(f, data, atlas, tilesets)
    for r in runs:
        r.write()

def write_uncompressed(f, data, atlas, tilesets):
    for i in data:
        ts = find_tileset(i, tilesets)
        if ts:
            norm = get_normalized_index(i, ts)
            converted = atlas.get_atlas_index(norm, ts["source"])
        else:
            converted = 0
        f.write(struct.pack("H", converted))

def write_draw_order_enum(draw_order_text, file):
    if draw_order_text == "topdown":
        file.write(struct.pack("I", 1))
    elif draw_order_text == "index":
        file.write(struct.pack("I", 2))
    else:
        assert False


EBET_StaticColliderRect = 0
EBET_StaticColliderCircle = 1
EBET_StaticColliderPoly = 2
EBET_StaticColliderEllipse = 3

def tiled_object_has_custom_prop(obj, prop_name):
    if not ("properties" in obj):
        return False
    return len(filter(lambda x : x["name"] == prop_name, obj["properties"])) > 0

def get_tiled_object_custom_prop(obj, prop_name):
    return list(filter(lambda x : x["name"] == prop_name, obj["properties"]))[0]

def get_static_collider_type(obj):
    if  tiled_object_has_custom_prop(obj, "radius"):
        return EBET_StaticColliderCircle
    if "polygon" in obj:
        return EBET_StaticColliderPoly
    if "ellipse" in obj:
        return EBET_StaticColliderEllipse
    return EBET_StaticColliderRect

def serialize_static_collider_ent(file, obj):
    t = get_static_collider_type(obj)
    if t == EBET_StaticColliderRect:
        file.write(struct.pack("I", 1))
        file.write(struct.pack("f", obj["width"]))
        file.write(struct.pack("f", obj["height"]))
    elif t == EBET_StaticColliderCircle:
        file.write(struct.pack("I", 1))
        file.write(struct.pack("f", get_tiled_object_custom_prop(obj, "radius")))
    elif t == EBET_StaticColliderPoly:
        assert False
    elif t == EBET_StaticColliderEllipse:
        assert False
    else:
        assert False
    pass


class EntitySerializer:
    def __init__(self, serialize_fn, get_type_fn, b_keep_in_quad):
        self.serialze_fn = serialize_fn
        self.get_type_fn = get_type_fn
        self.b_keep_in_quad = b_keep_in_quad
    def serialize_common(self, f, obj):
        f.write(struct.pack("I", 1)) # version
        f.write(struct.pack("f", obj["x"]))
        f.write(struct.pack("f", obj["y"]))
        f.write(struct.pack("f", 1))
        f.write(struct.pack("f", 1))
        f.write(struct.pack("f", obj["rotation"]))
        f.write(struct.pack("f", 0.0)) # rotation point relative - must be set programatically
        f.write(struct.pack("f", 0.0))
        f.write(struct.pack("I", 1 if self.b_keep_in_quad else 0))
    def serialize(self, f, o):
        f.write(struct.pack("I", self.get_type_fn(o)))
        self.serialize_common(f, o)
        self.serialze_fn(f , o)

entity_binary_serializers = {
    "StaticCollider" : EntitySerializer(serialize_static_collider_ent, get_static_collider_type, False)
}

def register_entity_serializer(name : str, serialize, get_type, b_keep_in_quad):
    print(f"Registered Type {name}")
    entity_binary_serializers.update({name : EntitySerializer(serialize, get_type, b_keep_in_quad)})

def count_serializable_ents(entities):
    i = 0
    types = {}
    for e in entities:
        if not e["type"] in types:
            types[e["type"]] = 1
        else:
            types[e["type"]] += 1

        if e["type"] in  entity_binary_serializers:
            i += 1
    print(f"types present: {", ".join([f"{x[0]}({x[1]})" for x in types.items()])}")
    return i
def build_tilemap_binaries(args, parsed_tile_maps, atlas):
    print("outputting tilemap binary files...\n")
    for p in parsed_tile_maps:
        newFileName = p["srcPath"].replace(".json", ".tilemap")
        newFileName = os.path.basename(newFileName)
        print(f"Doing file: '{newFileName}'")
        binaryPath = os.path.join(os.path.abspath(args.outputDir), newFileName)
        tilesets = p["tilesets"]
        tilesets = sorted(tilesets, key=lambda tileset: tileset["firstgid"])   # sort by age
        tilesets.reverse()
        with open(binaryPath, "wb") as f:
            # VERSION
            f.write(struct.pack("I", TILEMAP_FILE_VERSION))
            # QUADTREE
            f.write(struct.pack("f", args.iqtlx))
            f.write(struct.pack("f", args.iqtly))
            f.write(struct.pack("f", args.iqbrx))
            f.write(struct.pack("f", args.iqbry))
            # NUM LAYERS
            num_tilemap_layers = len(p["layers"])
            f.write(struct.pack("I", num_tilemap_layers))
            layerNum = 0
            for layer in p["layers"]:
                if "data" in layer:
                    # WRITE 1 FOR TILE LAYER
                    f.write(struct.pack("I", 1))
                    data = layer["data"]
                    tw, th = get_tile_layer_tile_dims(layer, atlas, tilesets)
                    print(f"Layer {str(layerNum)} Tile width: {tw} Tile height: {th} Width: {layer["width"]} tiles, Height: {layer["height"]} tiles.")
                    layerNum += 1
                    # INT FIELDS FOR LAYER
                    f.write(struct.pack("I", layer["width"]))
                    f.write(struct.pack("I", layer["height"]))
                    f.write(struct.pack("I", layer["x"]))
                    f.write(struct.pack("I", layer["y"]))
                    f.write(struct.pack("I", tw if tw > 0 else 0))
                    f.write(struct.pack("I", th if th > 0 else 0))
                    if args.rle:
                        f.write(struct.pack("I", 1))     # enum value to signify RLE
                        # RUN LENGTH ENCODED TILES
                        write_rle(f, data, atlas, tilesets)
                    else:
                        f.write(struct.pack("I", 2))     # enum value to signify uncompressed
                        # UNCOMPRESSED TILES
                        write_uncompressed(f, data, atlas, tilesets)
                else:
                    # WRITE 2 FOR OBJECT LAYER
                    f.write(struct.pack("I", 2))
                    write_draw_order_enum(layer["draworder"], f)
                    f.write(struct.pack("I", 1))
                    num_ents = count_serializable_ents(layer["objects"])
                    print(f"Number of entities: {num_ents}")
                    f.write(struct.pack("I", num_ents))
                    for o in layer["objects"]:
                        if o["type"] in entity_binary_serializers.keys():
                            serializer =  entity_binary_serializers[o["type"]]
                            serializer.serialize(f, o)
                        else:
                            print(f"Warning: No serializer for entity type {o["type"]}")
                        pass

        print("\n")
    print("\n")

def convert_tile_maps(args):
    "convert tile map files to a set of three files, an atlas xml file, an game objects xml file, and a tilemap binary file"
    parsed_tile_maps = []
    for p in args.tile_maps:
        with open(p, "r") as f:
            tm = json.load(f)
            tm["srcPath"] = p
            parsed_tile_maps.append(tm)
    named = get_named_tiles(parsed_tile_maps)
    used : dict[str, set[int]] = get_tiles_used_per_tileset(parsed_tile_maps)
    atlas = Atlas(used, named, args)
    atlasXMLPath = os.path.join(os.path.abspath(args.outputDir), "atlas.xml")
    atlas.output_atlas_xml_file(atlasXMLPath)
    if args.atlas_tool != None:
        run_atlas_tool(args.atlas_tool, atlasXMLPath, args)
    else:
        print("no atlas tool, specify with -A")
    build_tilemap_binaries(args, parsed_tile_maps, atlas)

def main():
    args = do_cmd_args()
    convert_tile_maps(args)
main()
