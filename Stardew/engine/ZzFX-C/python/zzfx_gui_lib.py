import random

DECIMALS_DISPLAYED = 3

WAVEFORM_NAME_LUT = {
    "sine" : 0,
    "triangle" : 1,
    "saw" : 2,
    "tan" : 3,
    "noise" : 4,
    "square" : 5
}


def uniform(x, y):
    return round(random.uniform(x,y), DECIMALS_DISPLAYED)

def random_explosion(params):
    params["frequency"]["var"].set(uniform(30, 99))
    keys_list = list(WAVEFORM_NAME_LUT.keys())
    i = int(uniform(0, len(keys_list)))
    params["shape"]["var"].set(keys_list[i])
    params["attack"]["var"].set(uniform(0.0, 0.1))
    params["decay"]["var"].set(uniform(0.05, 0.2))
    params["sustain"]["var"].set(uniform(0.0, 0.3))
    params["sustainVolume"]["var"].set(uniform(0.3, 0.5))
    params["release"]["var"].set(uniform(0.2, 0.6))

    if uniform(0, 1) < 0.5:
        params["slide"]["var"].set(0.0)
    else:
        params["slide"]["var"].set(uniform(-9,9))
    pass

    if uniform(0, 1) < 0.5:
        params["deltaSlide"]["var"].set(0.0)
    else:
        params["deltaSlide"]["var"].set(uniform(-9,9))

    if uniform(0, 1) < 0.5:
        params["delay"]["var"].set(0.0)
    else:
        params["delay"]["var"].set(uniform(0, 0.5))

    params["noise"]["var"].set(uniform(0.0, 2.0))

    if uniform(0, 1) < 0.8:
        params["modulation"]["var"].set(0.0)
    else:
        params["modulation"]["var"].set(uniform(0, 1.0) ** 2 * 99)

    params["bitCrush"]["var"].set(uniform(0.1, 1.0))

    if uniform(0, 1) < 0.5:
        params["tremolo"]["var"].set(0.0)
    else:
        params["tremolo"]["var"].set(uniform(0,0.5))

    if uniform(0, 1) < 0.5 or params["tremolo"]["var"].get() != 0:
        params["repeatTime"]["var"].set(uniform(0.05,0.3))

    filter = 0
    if random.random() < 0.5:
        filter = 0
    else:
        if random.random() < 0.5:
            filter = 99 + random.random() ** 2 * 2e3
        else:
            filter = random.random() ** 2 * 2e3 - 3500

    params["filter"]["var"].set(round(filter, DECIMALS_DISPLAYED))

def random_powerup(params):
    params["frequency"]["var"].set(uniform(99, 700))
    if uniform(0,1) < 0.2 :
        params["shape"]["var"].set("square")
    else:
        if uniform(0,1) < 0.5:
            params["shape"]["var"].set("sine")
        else:
            params["shape"]["var"].set("triangle")
    params["attack"]["var"].set(uniform(0.0, 0.1))
    params["decay"]["var"].set(uniform(0.1, 0.3))
    params["sustain"]["var"].set(uniform(0.1, 0.3))
    params["sustainVolume"]["var"].set(uniform(0.5, 1.0))
    params["release"]["var"].set(uniform(0.05, 0.4))
    if uniform(0, 1) < 0.8:
        params["delay"]["var"].set(0)
    else:
        params["delay"]["var"].set(0.2)
    params["repeatTime"]["var"].set(uniform(0.02, 0.2))
    if uniform(0, 1) < 0.5:
        params["slide"]["var"].set(0.0)
    else:
        params["slide"]["var"].set(uniform(-1,1)**3*20)

    if uniform(0, 1) < 0.5:
        params["deltaSlide"]["var"].set(0.0)
    else:
        params["deltaSlide"]["var"].set(uniform(-1,1)**3*400)

    if uniform(0, 1) < 0.8:
        params["noise"]["var"].set(0.0)
    else:
        params["noise"]["var"].set(uniform(0, 0.5))

    if uniform(0, 1) < 0.5:
        params["bitCrush"]["var"].set(0.0)
    else:
        params["bitCrush"]["var"].set(uniform(0, 0.2))
    
    if uniform(0, 1) < 0.5:
        params["tremolo"]["var"].set(0.0)
    else:
        params["tremolo"]["var"].set(uniform(0, 0.5))

    if uniform(0, 1) < 0.8:
        params["modulation"]["var"].set(0.0)
    else:
        params["modulation"]["var"].set(uniform(0, 1.0) ** 2 * 50)

    if uniform(0, 1) < .5 or (params["tremolo"]["var"].get() != 0) and (params["slide"]["var"].get() != 0) and (params["deltaSlide"]["var"].get() != 0):
        if uniform(0, 1) < 0.5:
            params["pitchJump"]["var"].set(-uniform(50, 200))
        else:
            params["pitchJump"]["var"].set(uniform(50, 500))
        if params["pitchJump"] == 0:
            params["pitchJumpTime"]["var"].set(0)
        else:
            params["pitchJumpTime"]["var"].set(uniform(0.05, 0.2))

def random_hit(params):
    
    params["frequency"]["var"].set(uniform(30, 500))
    keys_list = list(WAVEFORM_NAME_LUT.keys())
    i = int(uniform(0, len(keys_list)))
    params["shape"]["var"].set(keys_list[i])
    params["attack"]["var"].set(uniform(0.0, 0.03))
    params["decay"]["var"].set(uniform(0.0, 0.1))
    params["sustain"]["var"].set(uniform(0.0, 0.1))
    params["sustainVolume"]["var"].set(uniform(0.4, 1.0))
    params["release"]["var"].set(uniform(0.0, 0.2))

    if uniform(0, 1) < 0.5:
        params["delay"]["var"].set(0.0)
    else:
        params["delay"]["var"].set(uniform(0, 0.2))

    if uniform(0, 1) < 0.5:
        params["slide"]["var"].set(0.0)
    else:
        params["slide"]["var"].set(uniform(-1,1) ** 3 * 10)

    if uniform(0, 1) < 0.5:
        params["deltaSlide"]["var"].set(0.0)
    else:
        params["deltaSlide"]["var"].set(uniform(-1,1) ** 3 * 20)

    params["noise"]["var"].set(uniform(0.0, 2.0))

    if uniform(0, 1) < 0.8:
        params["modulation"]["var"].set(0.0)
    else:
        params["modulation"]["var"].set(round(random.random() ** 2 * 50, DECIMALS_DISPLAYED))

    params["bitCrush"]["var"].set(uniform(0.0, 0.5))

    if uniform(0, 1) < 0.6:
        params["tremolo"]["var"].set(0.0)
    else:
        params["tremolo"]["var"].set(uniform(0,0.5))

    if uniform(0, 1.0) < 0.5 or params["tremolo"]["var"].get() != 0:
        params["repeatTime"]["var"].set(uniform(0.01,0.1))

    filter = 0
    if random.random() < 0.5:
        filter = 0
    else:
        if random.random() < 0.5:
            filter = 99 + random.random() ** 2 * 2e3
        else:
            filter = random.random() ** 2 * 2e3 - 3500

    params["filter"]["var"].set(round(filter, DECIMALS_DISPLAYED))

