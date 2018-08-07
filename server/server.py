# Import Library
import time

from flask import Flask, request
from flask.templating import render_template

# Create App
app = Flask(__name__)

# Get Current Time
@app.route("/time", methods = ["GET", "POST"])
def get_time():
    return str(time.time())

# Receive Sensor JSON data
@app.route("/sensorData", methods = ["POST"])
def sensor_data():

    # Execute Only POST
    if request.method == "POST":

        # Parse JSON Data
        content = request.get_json()

        # Write CSV file
        writeCSV()

        # Append data list
        else:
            # Append sensor data list
            raw_samples = content['samples']

            for sample in raw_samples:
                CSV_file.raw_data.append(sample)

    return "sensorData Test"


# Changing ESP8266 IR current
@app.route("/getIRCurrent", methods = ["GET"])
def get_ir_current():

    return str(Current.current_level.getIrCurrentLevel())


# Changing ESP8266 IR current
@app.route("/getREDCurrent", methods = ["GET"])
def get_red_current():

    return str(Current.current_level.getRedCurrentLevel())


# Request for change current
@app.route("/changeCurrent", methods = ["POST"])
def change_current():

    # Read change_ir, change_red
    change_ir = int(request.form['change_ir'])
    change_red = int(request.form['change_red'])

    # Decide Current Level of ESP8266 Board
    Current.current_level.setIrCurrentLevel(change_ir)
    Current.current_level.setRedCurrentLevel(change_red)

    # log
    print("changed current " + "ir: " + str(change_ir) + " / red: " + str(change_red))

    return render_template('test.html')


# Main
if __name__ == "__main__":

    # Run App
    app.run(debug=True, host='0.0.0.0', port=8080)
