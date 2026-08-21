import pandas as pd
import numpy as np
from idw import idw_interpolate

# Load the dataset
df = pd.read_csv("data/trainingData.csv")

# Narrow to one building and one floor, so we're testing within a single
# real physical space rather than mixing unrelated buildings together
subset = df[(df["BUILDINGID"] == 0) & (df["FLOOR"] == 0)].copy()

# Find the WAP column with the most actual readings (i.e. fewest "100" values)
# in this subset, since most WAPs are barely detected in any one area
wap_columns = [c for c in df.columns if c.startswith("WAP")]
detection_counts = (subset[wap_columns] != 100).sum()
best_wap = detection_counts.sort_values(ascending=False).index[1]
print(f"Using {best_wap}, detected in {detection_counts[best_wap]} readings")

# Keep only rows where this specific WAP was actually detected
subset = subset[subset[best_wap] != 100]

# Build a simple points list: x, y, and the RSSI reading for our chosen WAP
points = subset[["LONGITUDE", "LATITUDE", best_wap]].rename(
    columns={"LONGITUDE": "x", "LATITUDE": "y", best_wap: "rssi"}
).to_dict("records")

print(f"Total usable points: {len(points)}")

# Split into known points (80%) and holdout points (20%) to test against
np.random.seed(42)
np.random.shuffle(points)
split = int(len(points) * 0.8)
known_points, test_points = points[:split], points[split:]

# For each holdout point, predict its RSSI using only the known points,
# then compare to what was actually measured there
errors = []
for tp in test_points:
    predicted = idw_interpolate(known_points, tp["x"], tp["y"])
    actual = tp["rssi"]
    errors.append(abs(predicted - actual))

mae = sum(errors) / len(errors)
print(f"Mean Absolute Error: {mae:.2f} dBm across {len(test_points)} test points")