import math
import random

# The global variables below are set / modified from application
# Set frequency here
FREQ_HZ = 220
PERIOD = 1.0 / FREQ_HZ

# Modulation depth [0.0, 1.0]
DEPTH = 1.0

# Modulation function (notice internal prefix '_', such function will not be listed by wavegen UI)
def _modulate(bs, ms):
    mw = (ms + 1.0) / 2.0
    return bs * (mw * DEPTH - DEPTH + 1.0 if bs >= 0.0 else 1.0 - mw * DEPTH)

def uniformWhiteNoise(t, n, m):
    return n

def gaussWhiteNoise(*_):
    v = random.gauss(0.0, 1.0 / 3.0)
    if v > 1.0:
        v = 1.0
    elif v < -1.0:
        v = -1.0
    return v

def randomWalkNoise(t, n, m):
    v = m + (0.01 if n > 0.0 else -0.01)
    if v > 1.0:
        v = 2.0 - v
    elif v < -1.0:
        v = -2.0 - v
    return v

def brownianNoise(t, n, m):
    v = m + random.gauss(0.0, 0.01)
    if v > 1.0:
        v = 2.0 - v
    elif v < -1.0:
        v = -2.0 - v
    return v

def meander(t, *_):
    return 1.0 if math.floor(t * FREQ_HZ * 2.0) % 2 == 1 else -1.0

def noiseMeander(t, n, _):
    return _modulate(n, meander(t, 0))

def sine(t, *_):
    return math.sin(t * FREQ_HZ * math.pi * 2.0)

def noiseSine(t, n, _):
    return _modulate(n, sine(t, 0))

def sawtooth(t, *_):
    return math.fmod(t, PERIOD) / PERIOD * 2.0 - 1.0

def noiseSawtooth(t, n, _):
    return _modulate(n, sawtooth(t, 0))

def triangle(t, *_):
    return 2.0 / PERIOD * abs(math.fmod(t + PERIOD / 2.0, PERIOD) - PERIOD / 2.0) * 2.0 - 1.0

def noiseTriangle(t, n, _):
    return _modulate(n, triangle(t, 0))
