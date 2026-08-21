import numpy as np
import matplotlib.pyplot as plt
from idw import idw_interpolate

def generate_heatmap(known_points, grid_size=20, output_path="heatmap.png"):
    """
    known_points: list of dicts like {"x":.., "y":.., "rssi":..}
    grid_size: how many points per axis to estimate (higher = smoother, slower)
    output_path: where to save the resulting image
    """
    x_vals = np.linspace(0, 10, grid_size)
    y_vals = np.linspace(0, 10, grid_size)
    grid = np.zeros((grid_size, grid_size))

    for i, x in enumerate(x_vals):
        for j, y in enumerate(y_vals):
            grid[j, i] = idw_interpolate(known_points, x, y)

    plt.figure(figsize=(6, 5))
    plt.imshow(grid, extent=[0, 10, 0, 10], origin="lower", cmap="RdYlGn")
    plt.colorbar(label="RSSI (dBm)")
    plt.title("Wi-Fi Signal Coverage Map")
    plt.xlabel("X (room grid)")
    plt.ylabel("Y (room grid)")
    plt.savefig(output_path)
    plt.close()
    print(f"Heatmap saved to {output_path}")