import sys
import os
import numpy as np

SAMPLE_SIZE = 100

Y_MAX = 255
Y_MIN = 0
MID = 127
LO = MID - 32
HI = MID + 32
SIG_MIN_DUR = 2
SIG_MAX_DUR = 10


def get_signal(sample_size):
    sample = np.empty(sample_size, dtype=np.uint8)
    sample.fill(0)
    active = np.random.choice([LO, HI])
    delta_t = int(np.random.randint(SIG_MIN_DUR, SIG_MAX_DUR, 1)[0])
    a = 0        # start of interval
    b = delta_t  # end of interval

    sample[a:b] = active

    while b != sample_size:
        a = b
        delta_t = int(np.random.randint(SIG_MIN_DUR, SIG_MAX_DUR, 1)[0])
        b = a + delta_t if a + delta_t < sample_size else sample_size
        active = LO if active == HI else HI
        sample[a:b] = active

    return sample, np.linspace(0, sample_size - 1, sample_size)


if __name__ == '__main__':
    s, t = get_signal(SAMPLE_SIZE)
    noise = np.random.normal(0, 10, SAMPLE_SIZE)
    ns = (s + noise).astype(np.uint8)
    
    if len(sys.argv) > 1:
        file_noised_signal = sys.argv[1]
        ns.tofile(file_noised_signal, "")

    with os.fdopen(sys.stdout.fileno(), "wb", closefd=False) as stdout:
        stdout.write(ns.tobytes())
        stdout.flush()
