from heatmap import generate_heatmap

fake_points = [
    {"x": 0, "y": 0, "rssi": -50},
    {"x": 10, "y": 0, "rssi": -80},
    {"x": 0, "y": 10, "rssi": -60},
    {"x": 10, "y": 10, "rssi": -90},
]

generate_heatmap(fake_points)