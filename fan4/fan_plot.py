import matplotlib.pyplot as plt


def calc_pwm(t):
    # _/^_/^_/^
    n1 = 1
    n2 = 3
    np = 33 # 0..320 = 10*t
    n = n1 + np + n2
    _t = t % n
    if _t < n1:
        return 1000
    if _t < (n1 + np):
        return (_t - n1) * 10
    return 1320


def calc_pwm2(t):
    # _/^\__/^\__/^\_
    n1 = 1
    n2 = 3
    n3 = 1
    np = 33
    n = n1 + np + n2 + np + n3
    _t = t % n
    if _t < n1:
        return 1000
    if _t < (n1 + np):
        return (_t - n1) * 10
    if _t < (n1 + np + n2):
        return 1320
    if _t < (n1 + np + n2 + np):
        return 320 - ((_t - (n1 + np + n2)) * 10)
    # if _t >= (n1 + np + n2 + np):
    return 1000


t = [i for i in range(0, 500)]

x = [calc_pwm2(i) for i in t]

for a, b in zip(t, x):
    print(f"{a=:5d} {b=:5d}")

plt.plot(t, x, "o-r")
plt.grid(True)
plt.show()
