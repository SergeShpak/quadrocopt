import numpy as np
import matplotlib.pyplot as plt

def main():
    vals = np.loadtxt("results.txt")
    time = vals[:, 0]
    y = vals[:, 1]
    plt.plot(time, y)
    plt.xlabel('time')
    plt.show()

if __name__ == "__main__":
    main()
