import tkinter as tk
from tkinter import ttk

import zzfx
import random

from zzfx_gui_lib import *


import matplotlib.pyplot as plt
import numpy as np

BACKEND_TICK_MS = 100
BACKEND_TICK_S = 0.1

def add_param_row_standard(frame, onRow, paramsObject, paramInfo):
    l = tk.Label(frame, text=paramInfo["nicename"])
    l.grid(column=0, row=onRow)
    e = tk.Entry(frame, textvariable=paramInfo["var"])
    e.grid(column=1, row=onRow)

def add_param_row_wave_combi(frame, onRow, paramsObject, paramInfo):
    l = tk.Label(frame, text=paramInfo["nicename"])
    l.grid(column=0, row=onRow)
    c = ttk.Combobox(frame, textvariable=paramInfo["var"])
    c['values'] = list(WAVEFORM_NAME_LUT.keys())
    c.grid(column=1, row=onRow)

class SynthParams:
    def call_zzfx(self):
        call_args = [self.params[x]["get_float_val"](self.params[x]) for x in self.call_order]
        len_seconds = zzfx.zzfx(*call_args)
        buf = zzfx.zzfx_get_buffer_last()
        x = np.array([(x / len(buf)) * len_seconds for x in range(len(buf))])
        y = np.array(buf)
        plt.clf()
        plt.plot(x,y)
        plt.show()
        
    def random_explosion(self):
        random_explosion(self.params)

    def random_powerup(self):
        random_powerup(self.params)
    
    def random_hit(self):
        random_hit(self.params)
            
    def set_default_values(self, params):
        for p in params:
            if p == "volume":
                self.params["volume"]["var"].set(1.0)
            elif p == "randomness":
                self.params["randomness"]["var"].set(0.05)
            elif p == "frequency":
                self.params["frequency"]["var"].set(220)
            elif p == "attack":
                self.params["attack"]["var"].set(0)
            elif p == "sustain":
                self.params["sustain"]["var"].set(0)
            elif p == "release":
                self.params["release"]["var"].set(0.1)
            elif p == "shape":
                self.params["shape"]["var"].set("sine")
            elif p == "shapeCurve":
                self.params["shapeCurve"]["var"].set(1)
            elif p == "slide":
                self.params["slide"]["var"].set(0)
            elif p == "deltaSlide":
                self.params["deltaSlide"]["var"].set(0)
            elif p == "pitchJump":
                self.params["pitchJump"]["var"].set(0)
            elif p == "pitchJumpTime":
                self.params["pitchJumpTime"]["var"].set(0)
            elif p == "repeatTime":
                self.params["repeatTime"]["var"].set(0)
            elif p == "noise":
                self.params["noise"]["var"].set(0)
            elif p == "modulation":
                self.params["modulation"]["var"].set(0)
            elif p == "bitCrush":
                self.params["bitCrush"]["var"].set(0)
            elif p == "delay":
                self.params["delay"]["var"].set(0)
            elif p == "sustainVolume":
                self.params["sustainVolume"]["var"].set(0)
            elif p == "decay":
                self.params["decay"]["var"].set(0)
            elif p == "tremolo":
                self.params["tremolo"]["var"].set(0)
            elif p == "filter":
                self.params["filter"]["var"].set(0)
    
    def set_command_string_var(self):
        s = "{"
        s += f"{self.params['volume']["var"].get()},"
        s += f"{self.params['randomness']["var"].get()},"
        s += f"{self.params['frequency']["var"].get()},"
        s += f"{self.params['attack']["var"].get()},"
        s += f"{self.params['sustain']["var"].get()},"
        s += f"{self.params['release']["var"].get()},"
        s += f"{WAVEFORM_NAME_LUT[self.params['shape']["var"].get()]},"
        s += f"{self.params['shapeCurve']["var"].get()},"
        s += f"{self.params['slide']["var"].get()},"
        s += f"{self.params['deltaSlide']["var"].get()},"
        s += f"{self.params['pitchJump']["var"].get()},"
        s += f"{self.params['pitchJumpTime']["var"].get()},"
        s += f"{self.params['repeatTime']["var"].get()},"
        s += f"{self.params['noise']["var"].get()},"
        s += f"{self.params['modulation']["var"].get()},"
        s += f"{self.params['bitCrush']["var"].get()},"
        s += f"{self.params['delay']["var"].get()},"
        s += f"{self.params['sustainVolume']["var"].get()},"
        s += f"{self.params['decay']["var"].get()},"
        s += f"{self.params['tremolo']["var"].get()},"
        s += f"{self.params['filter']["var"].get()}"
        s += "}"
        self.command_string.set(s)

    def on_param_changed(self):
        self.set_command_string_var()

    def get_command_string_var(self):
        return self.command_string
    
    def __init__(self):
        self.params = {
            "volume" : { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Volume",
                "help" : "Volume scale (percent)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "randomness": { 
                "var" : tk.DoubleVar(),
                "step" : .05,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Randomness",
                "help" : "How much to randomize frequency (percent Hz)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "frequency": { 
                "var" : tk.DoubleVar(),
                "step" : 1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Frequency",
                "help" : "Frequency of sound (Hz)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "attack": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Attack",
                "help" : "Attack time, how fast sound starts (seconds)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "sustain": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Sustain",
                "help" : "Sustain time, how long sound holds (seconds)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "release": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Release",
                "help" : "Release time, how fast sound fades out (seconds)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "shape": { 
                "var" : tk.StringVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Wave Shape",
                "help" : "Shape of the sound wave",
                "add_row_widgets" : add_param_row_wave_combi,
                "get_float_val" : lambda p : WAVEFORM_NAME_LUT[p["var"].get()]
            },
            "shapeCurve": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Shape Curve",
                "help" : "Squarenes of wave (0=square, 1=normal, 2=pointy), for square waves 0-2 is duty cycle",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "slide": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Slide",
                "help" : "How much to slide frequency (kHz/s)'",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "deltaSlide": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Delta Slide",
                "help" : "How much to change slide (kHz/s/s)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "pitchJump": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Pitch Jump",
                "help" : "Frequency of pitch jump (Hz)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "pitchJumpTime": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Pitch Jump Time",
                "help" : "Time of pitch jump (seconds)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "repeatTime": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Repeat Time",
                "help" : "Resets some parameters periodically (seconds)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "noise": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Noise",
                "help" : "How much random noise to add (percent)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "modulation": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Modulation",
                "help" : "Frequency of modulation wave, negative flips phase (Hz)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "bitCrush": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Bit Crush",
                "help" : "Resamples at a lower frequency in (samples*100)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "delay": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Delay",
                "help" : "Overlap sound with itself for reverb and flanger effects (seconds)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "sustainVolume": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Sustain Volume",
                "help" : "Volume level for sustain (percent)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "decay": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Decay",
                "help" : "Decay time, how long to reach sustain after attack (seconds)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "tremolo": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Tremolo",
                "help" : "Trembling effect, rate controlled by repeat time (precent)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            },
            "filter": { 
                "var" : tk.DoubleVar(),
                "step" : 0.1,
                "min" : -1e9,
                "max" : 1e9,
                "nicename" : "Filter",
                "help" : "Filter cutoff frequency, positive for HPF, negative for LPF (Hz)",
                "add_row_widgets" : add_param_row_standard,
                "get_float_val" : lambda p : p["var"].get()
            }
        }
        self.call_order = ["volume",
                "randomness",
                "frequency",
                "attack",
                "sustain",
                "release",
                "shape",
                "shapeCurve",
                "slide",
                "deltaSlide",
                "pitchJump",
                "pitchJumpTime",
                "repeatTime",
                "noise",
                "modulation",
                "bitCrush",
                "delay",
                "sustainVolume",
                "decay",
                "tremolo",
                "filter"]
        for k in self.params.keys():
            p = self.params[k]
            p["var"].trace_add("write", lambda var, index, mode: self.on_param_changed())
                
        self.command_string = tk.StringVar()
        self.set_default_values(self.params.keys())
        self.set_command_string_var()

root = None

params = None

def update_zzfx_backend():
    zzfx.zzfx_Update(BACKEND_TICK_S)
    root.after(BACKEND_TICK_MS, update_zzfx_backend)

def add_parameter_rows(frame):
    onRow = 0
    for key in params.params.keys():
        val = params.params[key]
        val["add_row_widgets"](frame, onRow, params, val)
        onRow += 1

def play_sfx():
    params.call_zzfx()

def explosion():
    params.set_default_values(params.params.keys())
    params.random_explosion()

def powerup():
    params.set_default_values(params.params.keys())
    params.random_powerup()

def hit():
    params.set_default_values(params.params.keys())
    params.random_hit()

def add_right_pane(frame):
    play_btn = tk.Button(frame, text="Play", command=play_sfx)
    play_btn.grid(column=0, row=0)
    powerup_btn = tk.Button(frame, text="Powerup", command=powerup)
    powerup_btn.grid(column=0,row=1)
    explosion_bt = tk.Button(frame, text="Explosion", command=explosion)
    explosion_bt.grid(column=1, row=1)
    explosion_bt = tk.Button(frame, text="Hit", command=explosion)
    explosion_bt.grid(column=2, row=1)

def add_bottom_pane(frame, params):
    text_cmd = tk.Entry(frame, textvariable=params.get_command_string_var(), width=150)
    text_cmd.grid(column=0, row=0)

def main():
    global root, params
    zzfx.zzfx_InitBackend()
    root = tk.Tk()
    root.title = "ZzFX GUI"
    root.after(BACKEND_TICK_MS, update_zzfx_backend)

    # initialize params
    params = SynthParams()

    # setup widgets
    left_hand_pane = tk.Frame(root)
    left_hand_pane.grid(row=0, column=0)
    add_parameter_rows(left_hand_pane)

    right_hand_pane = tk.Frame(root)
    right_hand_pane.grid(row=0, column=1)
    add_right_pane(right_hand_pane)

    bottom_pane = tk.Frame(root)
    bottom_pane.grid(row=1, column=0, columnspan=2)
    add_bottom_pane(bottom_pane, params)

    root.mainloop()
    zzfx.zzfx_ShutdownBackend()

main()