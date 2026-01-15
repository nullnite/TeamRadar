import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import folium
from shapely.geometry import Point, LineString
from pyproj import Transformer

# =========================
# 1. NMEA -> decimal
# =========================
def nmea_to_decimal(coord, direction):
    if not coord:
        return None
    if direction in ["N", "S"]:
        deg = int(coord[:2])
        minutes = float(coord[2:])
    else:
        deg = int(coord[:3])
        minutes = float(coord[3:])
    dec = deg + minutes / 60
    if direction in ["S", "W"]:
        dec *= -1
    return dec


gps_points = []

with open("Moving_data.txt", "r", encoding="utf-8") as f:
    for line in f:
        if line.startswith("$GNRMC"):
            p = line.split(",")
            if p[2] == "A":
                lat = nmea_to_decimal(p[3], p[4])
                lon = nmea_to_decimal(p[5], p[6])
                if lat and lon:
                    gps_points.append((lon, lat))  # (lon, lat)

gps_df = pd.DataFrame(gps_points, columns=["lon", "lat"])

# =========================
# 2. Etaloninė linija (WGS84)
# =========================
reference_line_wgs84 = [
    (23.949407, 54.904190),
    (23.946603, 54.903636),
    (23.943218, 54.902985),
    (23.941391, 54.902631),
    (23.937650, 54.901901),
    (23.935858, 54.901564),
    (23.933418, 54.901087),
    (23.931866, 54.900781)
]

# =========================
# 3. Projekcija: WGS84 -> LKS94
# =========================
transformer = Transformer.from_crs(
    "EPSG:4326", "EPSG:3346", always_xy=True
)

def project(coords):
    return [transformer.transform(x, y) for x, y in coords]

gps_proj = project(gps_df.values.tolist())
ref_proj = project(reference_line_wgs84)

# =========================
# 4. Shapely atstumai
# =========================
line = LineString(ref_proj)

errors = []
for x, y in gps_proj:
    p = Point(x, y)
    errors.append(p.distance(line))

errors = np.array(errors)

# =========================
# 5. RMS
# =========================
rms = np.sqrt(np.mean(errors ** 2))
print(f"RMS paklaida: {rms:.2f} m")

# =========================
# 6. Error grafikas
# =========================
plt.figure(figsize=(10, 4), dpi=300)
plt.plot(errors, label="Statmeninė paklaida (m)")
plt.axhline(rms, linestyle="--", label=f"RMS = {rms:.2f} m")
plt.xlabel("GPS taško indeksas")
plt.ylabel("Atstumas iki linijos (m)")
plt.title("GPS trajektorijos paklaida nuo etaloninės linijos")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(
    "gps_trajektorijos_paklaida.png",
    dpi=300,
    bbox_inches="tight"
)

# =========================
# 7. HTML žemėlapis (Folium)
# =========================
center_lat = gps_df.lat.mean()
center_lon = gps_df.lon.mean()

m = folium.Map(location=[center_lat, center_lon], zoom_start=16)

# Raudona etaloninė linija
folium.PolyLine(
    [(lat, lon) for lon, lat in reference_line_wgs84],
    color="red",
    weight=4,
    tooltip="Etaloninė linija"
).add_to(m)

# Mėlyni GPS taškai
for _, row in gps_df.iterrows():
    folium.CircleMarker(
        location=[row.lat, row.lon],
        radius=3,
        color="blue",
        fill=True,
        fill_color="blue",
        fill_opacity=0.7
    ).add_to(m)

m.save("trajektorija.html")
print("HTML žemėlapis išsaugotas kaip trajektorija.html")
