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

def uniformWhiteNoise(t, n):
    return n

def gaussWhiteNoise(*a):
    v = random.gauss(0.0, 1.0 / 3.0)
    if v > 1.0:
        v = 1.0
    elif v < -1.0:
        v = -1.0
    return v

memRWN = 0.0

def randomWalkNoise(t, n):
    global memRWN
    memRWN += 0.01 if n > 0.0 else -0.01
    if memRWN > 1.0:
        memRWN = 2.0 - memRWN
    elif memRWN < -1.0:
        memRWN = -2.0 - memRWN
    return memRWN

memBN = 0.0

def brownianNoise(*a):
    global memBN
    memBN += random.gauss(0.0, 0.01)
    if memBN > 1.0:
        memBN = 2.0 - memBN
    elif memBN < -1.0:
        memBN = -2.0 - memBN
    return memBN

def meander(t, n):
    return 1.0 if math.floor(t * FREQ_HZ * 2.0) % 2 == 1 else -1.0

def noiseMeander(t, n):
    return _modulate(n, meander(t, 0))

def sine(t, n):
    return math.sin(t * FREQ_HZ * math.pi * 2.0)

def noiseSine(t, n):
    return _modulate(n, sine(t, 0))

def sawtooth(t, n):
    return math.fmod(t, PERIOD) / PERIOD * 2.0 - 1.0

def noiseSawtooth(t, n):
    return _modulate(n, sawtooth(t, 0))

def triangle(t, n):
    return 2.0 / PERIOD * abs(math.fmod(t + PERIOD / 2.0, PERIOD) - PERIOD / 2.0) * 2.0 - 1.0

def noiseTriangle(t, n):
    return _modulate(n, triangle(t, 0))
