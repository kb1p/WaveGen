import math

# The global variables below are set / modified from application
# Set frequency here
FREQ_HZ = 220
PERIOD = 1.0 / FREQ_HZ

# Modulation depth [0.0, 1.0]
DEPTH = 1.0

def solidNoise(t, n):
    return n

def meander(t, n):
    return 1.0 if math.floor(t * FREQ_HZ * 2.0) % 2 == 1 else (1.0 - DEPTH)

def noiseMeander(t, n):
    return n * meander(t, 0)

def sine(t, n):
    return (math.sin(t * FREQ_HZ * math.pi * 2.0) + 1.0) / 2.0 * DEPTH + (1.0 - DEPTH)

def noiseSine(t, n):
    return n * sine(t, 0)

def sawtooth(t, n):
    return math.fmod(t, PERIOD) / PERIOD * DEPTH + (1.0 - DEPTH)

def noiseSawtooth(t, n):
    return n * sawtooth(t, 0)

def triangle(t, n):
    return 2.0 / PERIOD * abs(math.fmod(t + PERIOD / 2.0, PERIOD) - PERIOD / 2.0) * DEPTH + (1.0 - DEPTH)

def noiseTriangle(t, n):
    return n * triangle(t, 0)
