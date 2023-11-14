import math

def meander(t, n):
    freqHz = 500
    return n * (1.0 if math.floor(t * freqHz) % 2 == 0 else 0.0)

def sine(t, n):
    freqHz = 1000
    return n * (math.sin(t * freqHz * math.pi) + 1.0) / 2.0

# Current signal
modulate = sine
