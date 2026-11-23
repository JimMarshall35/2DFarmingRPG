import argparse
from zzfx import *
from time import sleep

def parse_cmd_args():
    parser = argparse.ArgumentParser(prog='myprogram')
    parser.add_argument("call")
    parser.add_argument("--volume", "-v", required=False, default=0.2)
    return parser.parse_args()

def parse_call_string(call) -> list[float]:
    return [float(x) for x in call.split(',')]

def main():
    zzfx_InitBackend()
    args = parse_cmd_args()
    zzfx_SetMasterVolume(args.volume)
    callList = parse_call_string(args.call)
    assert len(callList) == 21
    seconds = zzfx(*callList)
    sleep(seconds)
    zzfx_ShutdownBackend()

main()