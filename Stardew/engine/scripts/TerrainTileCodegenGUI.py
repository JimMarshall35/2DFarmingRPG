import tkinter as tk
from tkinter import ttk
import argparse

check_vars = []

def add_left_pane(frame: tk.Frame):
    pass

def toggle_checkbutton(event):
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

    check_0.bind("<1>", toggle_checkbutton)
    check_1.bind("<1>", toggle_checkbutton)
    check_2.bind("<1>", toggle_checkbutton)
    check_3.bind("<1>", toggle_checkbutton)
    check_4.bind("<1>", toggle_checkbutton)
    check_5.bind("<1>", toggle_checkbutton)
    check_6.bind("<1>", toggle_checkbutton)
    check_7.bind("<1>", toggle_checkbutton)

    pass

def add_bottom_pane(frame: tk.Frame):
    pass

def init_vars():
    global check_vars
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

def parse_args():
    parser = argparse.ArgumentParser(prog='Terrain Tile Code generator')
    parser.add_argument("atlas_xml", required=True)
    return parser.parse_args()

def main():
    args = parse_args()
    root = tk.Tk()
    root.title = "Terrain Set Generator"

    init_vars()
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