from flask import Flask, request, jsonify
import json
import os

app = Flask(__name__)
LOG_FILE = 'log.json'

@app.route('/upload', methods=['POST'])
def upload_data():
    data = request.get_json()

    if not data or "temperature" not in data or "humidity" not in data or "timestamp" not in data:
        return jsonify({"status": "error", "message": "Invalid data"}), 400

    # Load or initialize
    if os.path.exists(LOG_FILE):
        with open(LOG_FILE, 'r') as f:
            try:
                log_data = json.load(f)
            except json.JSONDecodeError:
                log_data = {}
    else:
        log_data = {}

    next_index = str(len(log_data))
    log_data[next_index] = data

    with open(LOG_FILE, 'w') as f:
        json.dump(log_data, f, indent=4)

    return jsonify({"status": "success", "entry": next_index}), 200
        
@app.route('/log', methods=['GET'])
def get_log():
    if not os.path.exists(LOG_FILE):
        return jsonify([])  # return empty list if file doesn't exist

    with open(LOG_FILE, 'r') as f:
        try:
            data = json.load(f)
        except json.JSONDecodeError:
            return jsonify([])

    # Convert to list of values sorted by index
    sorted_items = sorted(data.items(), key=lambda x: int(x[0]))
    entry_list = [entry for _, entry in sorted_items]
    return jsonify(entry_list)


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
