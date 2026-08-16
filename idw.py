import numpy as np

def idw_interpolate(known_points, target_x, target_y, power=2):
    """
    known_points: list of dicts like {"x":.., "y":.., "rssi":..}
    target_x, target_y: the coordinate you want to estimate
    power: how strongly distance affects weight (2 is standard)
    """
    weights = []
    values = []

    for point in known_points:
        distance = np.sqrt((point["x"] - target_x) ** 2 + (point["y"] - target_y) ** 2)
        if distance == 0:
            return point["rssi"]  # exact match, no need to blend
        weight = 1 / (distance ** power)
        weights.append(weight)
        values.append(point["rssi"] * weight)

    return sum(values) / sum(weights)