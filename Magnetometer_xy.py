import matplotlib.pyplot as plt
import numpy as np

# Duomenys
telefono = np.array([
    57, 73, 80, 88, 94, 107, 115, 120, 131, 139, 147, 161,
    176, 197, 210, 216, 238, 251, 258, 270, 283, 299, 304,
    317, 322, 333, 342, 351, 356, 2, 5, 13, 20, 25, 32, 37, 46
])

device = np.array([
    67, 79, 92, 100, 110, 120, 130, 140, 150, 160, 170, 180,
    190, 200, 210, 220, 230, 240, 250, 260, 270, 280, 290,
    300, 310, 320, 330, 340, 350, 1, 10, 20, 30, 40, 50, 60, 70
])

# RMS skaičiavimas
rms = np.sqrt(np.mean((device - telefono) ** 2))

ideal = np.array([0, 360])

plt.figure()
plt.scatter(telefono, device)
plt.plot(ideal, ideal)
plt.xlabel("Telefono rodomi laipsniai")
plt.ylabel("Magnetometro (įrenginio) rodomi laipsniai")
plt.title(f"Ideali tiesė ir RMS paklaida\nRMS = {rms:.2f}°")
plt.grid(True)

plt.savefig(
    "xy.png",
    dpi=300,
    bbox_inches="tight"
)

print(f"RMS paklaida: {rms:.2f} laipsniai")

# Maksimali paklaida
max_klaida = np.max(np.abs(device - telefono))
print(f"Maksimali paklaida: {max_klaida:.2f} laipsniai")

min_klaida = np.min(np.abs(device - telefono))
print(f"Minimali paklaida: {min_klaida:.2f} laipsniai")