import tkinter as tk
from tkinter import ttk
import argparse
import xml.etree.ElementTree as ET
from itertools import product
import json

desc_text = """
    ------> X
    |
    |      ________
    V     |_0|_1|_2|
    Y     |_3|_A|_4|
          |_5|_6|_7|


    A GUI to produce a C lookup table for "terrain" sets.
    
    The tile A in the middle is determined by the surrounding tiles.
    The bits of an 8 bit index are set according tiles 1-7 according to the diagram above.

    Use this gui to set whether each position 0-7 contains a tile from the set, doesn't or can contain one or not.

    It generates a C lookup table and json file containing the permutations for the "wildcard" tiles.

"""
class Tile:
    def __init__(self, check_vars = None):
        if check_vars == None:
            self.check_vars = ["tristate","tristate","tristate","tristate","tristate","tristate","tristate","tristate"]
        else:
            self.check_vars = check_vars
    def generate_indices(self) -> list[int]:
        return generate_permutations(self.check_vars)
    def is_set(self) -> bool:
        return not all(x == "tristate" for x in self.check_vars)

check_vars : list[tk.StringVar]  = []

tileset_name_var: tk.StringVar = None


tiles : dict[str, Tile] = {}

selected_tile : str = None

listbox : tk.Listbox = None

def generate_permutations(values) -> list[int]:
    # Find indices of tristate values
    tri_indices = [i for i, v in enumerate(values) if v == "tristate"]
    
    results = []
    
    # Generate all combinations for tristates
    for combo in product([0, 1], repeat=len(tri_indices)):
        temp = values[:]
        
        # Replace tristates with current combination
        for idx, bit in zip(tri_indices, combo):
            temp[idx] = bit
        
        # Convert to 0/1 list
        bits = [
            1 if v == "on" else
            0 if v == "off" else
            v  # already 0 or 1 from combo
            for v in temp
        ]
        bits.reverse()
        
        # Convert to integer
        number = int("".join(map(str, bits)), 2)
        results.append(number)
    
    return results


def add_left_pane(frame: tk.Frame):
    global listbox
    entries = tuple(list(tiles.keys()))
    list_variable = tk.Variable(value=entries)
    listbox = tk.Listbox(
        frame,
        listvariable=list_variable,
        height=6,
        width=50,
        selectmode="single",
    )
    listbox.grid(column=0, row=0)
    def handle_item_select(event):
        global selected_tile
        selected_indices = listbox.curselection()
        #assert len(selected_indices) == 1        
        s = listbox.get(selected_indices[0])
        for i in range(len(tiles[s].check_vars)):
            check_vars[i].set(tiles[s].check_vars[i])

        selected_tile = s
        

    listbox.bind('<<ListboxSelect>>', handle_item_select)
    pass

def toggle_checkbutton(event, userInt):
    checkbutton = event.widget
    varname = checkbutton.cget("variable")
    current_value = checkbutton.getvar(varname)
    if current_value == "on":
        new_value = "off"
    elif current_value == "off":
        new_value = "tristate"
    else:
        new_value = "on"
    checkbutton.setvar(varname, new_value)

    if selected_tile != None:
        tiles[selected_tile].check_vars[userInt] = new_value

    return "break"

def add_right_pane(frame: tk.Frame):
    check_0 = tk.Checkbutton(frame, variable=check_vars[0], onvalue="on", offvalue="off", tristatevalue="tristate")
    check_0.grid(column=0, row=0)
    check_1 = tk.Checkbutton(frame, variable=check_vars[1], onvalue="on", offvalue="off", tristatevalue="tristate")
    check_1.grid(column=1, row=0)
    check_2 = tk.Checkbutton(frame, variable=check_vars[2], onvalue="on", offvalue="off", tristatevalue="tristate")
    check_2.grid(column=2, row=0)
    check_3 = tk.Checkbutton(frame, variable=check_vars[3], onvalue="on", offvalue="off", tristatevalue="tristate")
    check_3.grid(column=0, row=1)
    check_4 = tk.Checkbutton(frame, variable=check_vars[4], onvalue="on", offvalue="off", tristatevalue="tristate")
    check_4.grid(column=2, row=1)
    check_5 = tk.Checkbutton(frame, variable=check_vars[5], onvalue="on", offvalue="off", tristatevalue="tristate")
    check_5.grid(column=0, row=2)
    check_6 = tk.Checkbutton(frame, variable=check_vars[6], onvalue="on", offvalue="off", tristatevalue="tristate")
    check_6.grid(column=1, row=2)
    check_7 = tk.Checkbutton(frame, variable=check_vars[7], onvalue="on", offvalue="off", tristatevalue="tristate")
    check_7.grid(column=2, row=2)

    check_0.bind("<1>", lambda event : toggle_checkbutton(event, 0))
    check_1.bind("<1>", lambda event : toggle_checkbutton(event, 1))
    check_2.bind("<1>", lambda event : toggle_checkbutton(event, 2))
    check_3.bind("<1>", lambda event : toggle_checkbutton(event, 3))
    check_4.bind("<1>", lambda event : toggle_checkbutton(event, 4))
    check_5.bind("<1>", lambda event : toggle_checkbutton(event, 5))
    check_6.bind("<1>", lambda event : toggle_checkbutton(event, 6))
    check_7.bind("<1>", lambda event : toggle_checkbutton(event, 7))

def gather_json_object() -> dict:
    set_tiles = [(k, tiles[k]) for k in tiles.keys() if tiles[k].is_set()]
    out = {}
    for k, v in set_tiles:
        out[k] = v.generate_indices()
    return out

def serialize_raw():
    set_tiles = [(k, tiles[k]) for k in tiles.keys() if tiles[k].is_set()]
    out = {}
    for k, v in set_tiles:
        out[k] = v.check_vars
    return out

def output_json():
    obj = gather_json_object()
    out = {
        "name" : tileset_name_var.get(),
        "default_tile" : default_tile_text_var.get(),
        "indices" : obj,
        "raw" : serialize_raw()
    }
    with open(f"{tileset_name_var.get()}.json", "w") as fp:
        json.dump(out, fp, indent=2)

def longest_index_array(obj):
    longest = 0
    for k in obj.keys():
        if len(obj[k]) > longest:
            longest = len(obj[k])
    return longest

def find_tile_for_index(obj, index):
    for k in obj.keys():
        v = obj[k]
        for i in v:
            if i == index:
                return k
    return None

def output_c_lookup_table():
    obj = gather_json_object()
    longest = longest_index_array(obj)
    out = "// Generated by TerrainTileCodegenGUI.py\n\n"
    out += f'#include "{tileset_name_var.get()}.h"\n'
    out += f"const char* g{tileset_name_var.get()}[256] = " + "{\n"
    for i in range(256):
        tileName = find_tile_for_index(obj, i)
        if tileName:
            out += f'\t"{tileName}",\n'
        else:
            out += f'\t"{default_tile_text_var.get()}",\n'
    out += "};\n\n"
    #out += f"#define {tileset_name_var.get()}_NamesLen {len(obj.keys())}\n\n"
    out += f"const char* g{tileset_name_var.get()}_Names[{tileset_name_var.get()}_NamesLen] = " + "{\n"
    for k in obj.keys():
        out += f'\t"{k}",\n'
    if not (default_tile_text_var.get() in obj.keys()):
        out += f'\t"{default_tile_text_var.get()}",\n'
    out += "};\n"

    with open(f"{tileset_name_var.get()}.c", "w") as f:
        f.write(out)
    
    out = f"#ifndef {tileset_name_var.get()}_H\n#define {tileset_name_var.get()}_H\n"
    out += f"#define {tileset_name_var.get()}_NamesLen {len(obj.keys())}\n\n"
    out += f"extern const char* g{tileset_name_var.get()}_Names[{tileset_name_var.get()}_NamesLen];\n"
    out += f"extern const char* g{tileset_name_var.get()}[256];\n"
    out += "#endif\n"

    with open(f"{tileset_name_var.get()}.h", "w") as f:
        f.write(out)

def listbox_contains(listbox : tk.Listbox, item : str) -> bool:
    for i in range(listbox.size()):
        if listbox.get(i) == item:
            return True
    return False

def new_row():
    global listbox
    tiles[new_tile_var.get()] = Tile()
    entries = list(tiles.keys())
    listbox.delete(0, tk.END)  # clear old items
    for e in entries:
        listbox.insert(tk.END, e)
    

def add_bottom_pane(frame: tk.Frame):
    c_btn = tk.Button(frame, text="C LUT", command=output_c_lookup_table)
    json_btn = tk.Button(frame, text="JSON", command=output_json)
    label_default_tile = tk.Label(frame, text="Default Tile")
    default_tile_name = tk.Entry(frame, width=30, textvariable=default_tile_text_var)
    label_tileset_name = tk.Label(frame, text="Tileset Name")
    tileset_name = tk.Entry(frame, width=30, textvariable=tileset_name_var)

    label_new_tile_name = tk.Label(frame, text="New Tile Name")
    new_tile_name = tk.Entry(frame, width=30, textvariable=new_tile_var)
    new_tile_btn = tk.Button(frame, text="new tile", command=new_row)

    c_btn.grid(column=0, row=0)
    json_btn.grid(column=1, row=0)
    default_tile_name.grid(column=1, row=1)
    label_default_tile.grid(column=0, row=1)

    tileset_name.grid(column=1, row=2)
    label_tileset_name.grid(column=0, row=2)

    label_new_tile_name.grid(column=0, row=3)
    new_tile_name.grid(column=1, row=3)
    new_tile_btn.grid(column=2, row=3)


def init_vars():
    global check_vars, default_tile_text_var, tileset_name_var, new_tile_var
    check_vars = [
        tk.StringVar(),
        tk.StringVar(),
        tk.StringVar(),
        tk.StringVar(),
        tk.StringVar(),
        tk.StringVar(),
        tk.StringVar(),
        tk.StringVar(),
    ]
    default_tile_text_var = tk.StringVar()
    tileset_name_var = tk.StringVar()
    new_tile_var = tk.StringVar()

def parse_args():
    
    parser = argparse.ArgumentParser(prog='Terrain Tile Code generator', description=desc_text, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--atlas_xml", help="atlas_xml produced by compile_assets.sh")
    parser.add_argument("--json", help="json file output by this tool")
    return parser.parse_args()

def parse_xml_named_tiles(path):
    global tiles
    tree = ET.parse(path)
    root = tree.getroot()
    named_tiles = root.find("named-tiles")
    for child in named_tiles:
        tiles[child.attrib["name"]] = Tile()

def load_json(path):
    data = {}
    with open(path, 'r') as f:
        # Parsing the JSON file into a Python dictionary
        data = json.load(f)
    for k in data["raw"].keys():
        tiles[k] = Tile(data["raw"][k])
        tileset_name_var.set(data["name"])
        default_tile_text_var.set(data["default_tile"])
    pass

def main():
    args = parse_args()

    root = tk.Tk()
    root.title = "Terrain Set Generator"
    init_vars()

    if args.atlas_xml:
        parse_xml_named_tiles(args.atlas_xml)

    if args.json:
        load_json(args.json)

    
    left_hand_pane = tk.Frame(root)

    left_hand_pane.grid(row=0, column=0)
    add_left_pane(left_hand_pane)

    right_hand_pane = tk.Frame(root)
    right_hand_pane.grid(row=0, column=1)
    add_right_pane(right_hand_pane)

    bottom_pane = tk.Frame(root)
    bottom_pane.grid(row=1, column=0, columnspan=2)
    add_bottom_pane(bottom_pane)

    root.mainloop()

main()