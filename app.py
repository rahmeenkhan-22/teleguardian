from flask import Flask, jsonify, send_file, request
from firebase_config import push_reading, get_all_readings
from heatmap import generate_heatmap

app = Flask(__name__)

@app.route("/")
def hello():
    return "TeleGuardian backend is alive!"

@app.route("/readings", methods=["POST"])
def add_reading():
    data = request.get_json()
    try:
        push_reading(data)
        return jsonify({"status": "success"}), 201
    except ValueError as e:
        return jsonify({"status": "error", "message": str(e)}), 400

@app.route("/readings", methods=["GET"])
def list_readings():
    readings = get_all_readings()
    if not readings:
        return jsonify({"status": "success", "data": {}}), 200
    return jsonify({"status": "success", "data": readings}), 200
        

@app.route("/heatmap", methods=["GET"])
def heatmap():
    readings = get_all_readings()
    if not readings:
        return jsonify({"status": "error", "message": "No readings available yet"}), 404

    points = [
        {"x": r["x"], "y": r["y"], "rssi": r["rssi"]}
        for r in readings.values()
    ]
    generate_heatmap(points, output_path="heatmap.png")
    return send_file("heatmap.png", mimetype="image/png")

@app.route("/diagnosis", methods=["GET"])
def diagnosis():
    # Stub for now — Person B's real model plugs in here later
    return jsonify({"fault": "congestion", "confidence": 0.82})

if __name__ == "__main__":
    app.run(debug=True, port=5000)