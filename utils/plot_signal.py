import sys
import numpy as np
import matplotlib.pyplot as plt

Y_MAX = 255
Y_MIN = 0


def plot_signal(file_noised_signal, file_filtered_signal):
    noised = np.fromfile(file_noised_signal, dtype=np.uint8)
    filtered = np.fromfile(file_filtered_signal, dtype=np.uint8)
    sample_size = noised.size
    t = np.linspace(0, sample_size - 1, sample_size)

    plt.subplot(2, 1, 1)
    plt.ylim(Y_MIN, Y_MAX)
    plt.plot(t, noised)
    plt.title("Signal with noise")

    plt.subplot(2, 1, 2)
    plt.ylim(0, 255)
    plt.plot(t, filtered)
    plt.title("Filtered signal")

    plt.show()


if __name__ == '__main__':
    ns = sys.argv[1]
    fs = sys.argv[2]
    plot_signal(ns, fs)
