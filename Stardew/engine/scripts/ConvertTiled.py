import argparse
import json
import xml.etree.ElementTree as ET
import typing
import os
import subprocess
import struct

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
    parser.add_argument("-A", "--assets_folder", default="./Assets")
    parser.add_argument("-r", "--rle", type=bool, default=False)

    args = parser.parse_args()
    return args

def find_tileset(index: int, tilesets: object) -> object:
    for t in tilesets:
        if index >= t["firstgid"]:
            return t

def get_normalized_index(index: int, tileset: object) -> int:
    return (index - tileset["firstgid"]) + 1

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
            "source" : "./Assets/" + self.path,
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
        counter = 0
        top = ET.Element("atlas")
        tree = ET.ElementTree(top)
        for s in self.sprites:
            ET.SubElement(top, "sprite", s.get_attributes(counter))
            counter += 1
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
                    l = margin + (((index - 1) % columns) * tileWidth  + spacing)
                    t = margin + (((index - 1) / columns) * tileHeight + spacing)
                    if not key in self.lut:
                        self.lut[key] = dict()
                    self.lut[key][index] = len(self.sprites) + 1
                    self.sprites.append(AtlasSprite(image, t, l, tileWidth, tileHeight, name))
    def __init__(self, originalNormalizedIndexes : dict[str, set[int]], args):
        self.originalNormalizedIndexes = originalNormalizedIndexes
        self.sprites = []
        self.lut = dict()
        self.args = args
        self.make_sprites()

def run_atlas_tool(path, xmlPath, args):
    binPath = os.path.join(os.path.abspath(args.outputDir), "main.atlas")
    print(binPath)
    result = subprocess.run([os.path.abspath(path), xmlPath, "-o", binPath], capture_output=True, cwd=os. getcwd(), shell=True)
    print(f"Running Atlas Tool...\n")
    print(f"Std Out:\n{result.stdout.decode("utf-8")}\n")
    print(f"Std Err:\n{result.stderr.decode("utf-8")}\n")

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

def write_rle(f, data, atlas, tilesets):
    runs = build_runs(f, data, atlas, tilesets)
    for r in runs:
        r.write()

def write_uncompressed(f, data, atlas, tilesets):
    print(tilesets)
    for i in data:
        ts = find_tileset(i, tilesets)
        if ts:
            norm = get_normalized_index(i, ts)
            converted = atlas.get_atlas_index(norm, ts["source"])
        else:
            converted = 0
        f.write(struct.pack("H", converted))

def build_tilemap_binaries(args, parsed_tile_maps, atlas):
    for p in parsed_tile_maps:
        newFileName = p["srcPath"].replace(".json", ".tilemap")
        newFileName = os.path.basename(newFileName)
        print(f"NEW NAME '{newFileName}'")
        binaryPath = os.path.join(os.path.abspath(args.outputDir), newFileName)
        tilesets = p["tilesets"]
        tilesets = sorted(tilesets, key=lambda tileset: tileset["firstgid"])   # sort by age
        tilesets.reverse()
        with open(binaryPath, "wb") as f:
            # VERSION
            f.write(struct.pack("I", TILEMAP_FILE_VERSION))
            # NUM LAYERS
            num_tilemap_layers = count_tilemap_layers(p["layers"])
            f.write(struct.pack("I", num_tilemap_layers))
            for layer in p["layers"]:
                if "data" in layer:
                    # INT FIELDS FOR LAYER
                    f.write(struct.pack("I", layer["width"]))
                    f.write(struct.pack("I", layer["height"]))
                    f.write(struct.pack("I", layer["x"]))
                    f.write(struct.pack("I", layer["y"]))
                    if args.rle:
                        # RUN LENGTH ENCODED TILES
                        write_rle(f, layer["data"], atlas, tilesets)
                    else:
                        # UNCOMPRESSED TILES
                        write_uncompressed(f, layer["data"], atlas, tilesets)

def convert_tile_maps(args):
    "convert tile map files to a set of three files, an atlas xml file, an game objects xml file, and a tilemap binary file"
    parsed_tile_maps = []
    for p in args.tile_maps:
        with open(p, "r") as f:
            tm = json.load(f)
            tm["srcPath"] = p
            parsed_tile_maps.append(tm)

    used : dict[str, set[int]] = get_tiles_used_per_tileset(parsed_tile_maps)
    print(used)
    atlas = Atlas(used, args)
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