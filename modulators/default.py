import math

# Set frequency here
freqHz = 220

period = 1.0 / freqHz

def solidNoise(t, n):
    return n

def meander(t, n):
    return 1.0 if math.floor(t * freqHz * 2.0) % 2 == 1 else 0.0

def noiseMeander(t, n):
    return n * meander(t, 0)

def sine(t, n):
    return (math.sin(t * freqHz * math.pi * 2.0) + 1.0) / 2.0

def noiseSine(t, n):
    return n * sine(t, 0)

def sawtooth(t, n):
    return math.fmod(t, period) / period

def noiseSawtooth(t, n):
    return n * sawtooth(t, 0)

def triangle(t, n):
    return 2.0 / period * abs(math.fmod(t + period / 2.0, period) - period / 2.0)

def noiseTriangle(t, n):
    return n * triangle(t, 0)
