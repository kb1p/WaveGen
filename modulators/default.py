import math

def solidNoise(t, n):
    return n

def meander(t, n):
    freqHz = 500
    return 1.0 if math.floor(t * freqHz) % 2 == 0 else 0.0

def noiseMeander(t, n):
    return n * meander(t, 0)

def sine(t, n):
    freqHz = 1000
    return (math.sin(t * freqHz * math.pi) + 1.0) / 2.0

def noiseSine(t, n):
    return n * sine(t, 0)

def triangle(t, n):
    freqHz = 500
    return t**2 if math.floor(t * freqHz) % 2 == 0 else 1.0 - t

def noiseTriangle(t, n):
    return n * triangle(t, 0)
