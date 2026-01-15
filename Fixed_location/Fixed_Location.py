import math
import matplotlib.pyplot as plt
import geopandas as gpd
from shapely.geometry import Point
import contextily as ctx

# =====================================================
# NMEA -> decimal degrees
# =====================================================

def nmea_to_decimal(coord, direction):
    coord = float(coord)
    degrees = int(coord // 100)
    minutes = coord - degrees * 100
    decimal = degrees + minutes / 60.0
    if direction in ("S", "W"):
        decimal *= -1
    return decimal


# =====================================================
# Haversine atstumas (metrais)
# =====================================================

def haversine(lat1, lon1, lat2, lon2):
    R = 6371000.0
    phi1 = math.radians(lat1)
    phi2 = math.radians(lat2)
    dphi = math.radians(lat2 - lat1)
    dlambda = math.radians(lon2 - lon1)

    a = math.sin(dphi / 2)**2 + \
        math.cos(phi1) * math.cos(phi2) * math.sin(dlambda / 2)**2

    return 2 * R * math.atan2(math.sqrt(a), math.sqrt(1 - a))


# =====================================================
# Referencinis taškas
# =====================================================

REF_LAT = 54.90358640
REF_LON = 23.95663114


# =====================================================
# Duomenų nuskaitymas
# =====================================================

FILE_NAME = "Fixed_data.txt"

points = []
distances = []

with open(FILE_NAME, "r", encoding="utf-8") as f:
    for line in f:
        if line.startswith("$GNGLL"):
            p = line.strip().split(",")
            lat = nmea_to_decimal(p[1], p[2])
            lon = nmea_to_decimal(p[3], p[4])

            points.append(Point(lon, lat))
            distances.append(haversine(REF_LAT, REF_LON, lat, lon))

# RMS
rms = math.sqrt(sum(d**2 for d in distances) / len(distances))

print(f"Taškų skaičius: {len(points)}")
print(f"RMS paklaida: {rms:.2f} m")


# =====================================================
# GeoDataFrame
# =====================================================

gdf = gpd.GeoDataFrame(geometry=points, crs="EPSG:4326")
gdf_ref = gpd.GeoDataFrame(
    geometry=[Point(REF_LON, REF_LAT)],
    crs="EPSG:4326"
)

# Projekcija žemėlapiui
gdf = gdf.to_crs(epsg=3857)
gdf_ref = gdf_ref.to_crs(epsg=3857)


# =====================================================
# STATINIS ŽEMĖLAPIS – MINIMALUS UI (NE MILŽINIŠKAS)
# =====================================================

fig, ax = plt.subplots(figsize=(6, 6), dpi=200)

# GPS taškai – LABAI MAŽI
gdf.plot(
    ax=ax,
    color="blue",
    markersize=1,
    alpha=0.6
)

# Referencinis taškas – SUBTILUS
gdf_ref.plot(
    ax=ax,
    color="red",
    marker="x",
    markersize=15,
    linewidth=1.0
)

# Ribos pagal duomenis
xmin, ymin, xmax, ymax = gdf.total_bounds
dx = xmax - xmin
dy = ymax - ymin
pad = max(dx, dy) * 1

ax.set_xlim(xmin - pad, xmax + pad)
ax.set_ylim(ymin - pad, ymax + pad)

# =====================================================
# MASTELIO JUOSTA (SCALE BAR)
# =====================================================

# Pasirenkamas mastelis (metrais)
scale_length_m = 1  # 5 metrai – idealiai GPS sklaidai

# Dabartinės ribos
xmin, xmax = ax.get_xlim()
ymin, ymax = ax.get_ylim()

# Scale bar pozicija (apatinis kairys kampas)
bar_x_start = xmin + (xmax - xmin) * 0.05
bar_y = ymin + (ymax - ymin) * 0.05

# Nubrėžiame liniją
ax.plot(
    [bar_x_start, bar_x_start + scale_length_m],
    [bar_y, bar_y],
    linewidth=2
)

# Užrašas
ax.text(
    bar_x_start + scale_length_m / 2,
    bar_y + (ymax - ymin) * 0.015,
    f"{scale_length_m} m",
    ha="center",
    va="bottom",
    fontsize=8
)


# OpenStreetMap Mapnik
ctx.add_basemap(
    ax,
    source=ctx.providers.OpenStreetMap.Mapnik,
    crs=gdf.crs
)

# VISIŠKAI IŠJUNGIAM UI
ax.set_axis_off()
ax.set_title("GPS taškai (OSM Mapnik)", fontsize=9)

plt.tight_layout()
plt.savefig("gps_map_osm_mapnik_minimal.png", dpi=200)
plt.show()


# =====================================================
# ATSTUMO GRAFIKAS SU RMS
# =====================================================

plt.figure(figsize=(10, 4))
plt.plot(distances, linewidth=1.2, label="Atstumas")
plt.axhline(
    rms,
    linestyle="--",
    linewidth=1.2,
    label=f"RMS = {rms:.2f} m"
)
plt.xlabel("Matavimo indeksas")
plt.ylabel("Atstumas (m)")
plt.title("Atstumas nuo referencinio taško")
plt.grid(True, alpha=0.3)
plt.legend(fontsize=9)
plt.tight_layout()
plt.savefig("distance_rms.png", dpi=300)
plt.show()

