# Import Library
import time

from flask import Flask, request
from flask.templating import render_template

from myCsv import write_csv
from myCsv import init_csv
from current_level import current_level as ESP_current


# Create App
app = Flask(__name__)


# Get Current Time
@app.route("/time", methods = ["GET", "POST"])
def get_time():
    return str(time.time())


# Receive Sensor JSON data
@app.route("/sensorData", methods = ["POST"])
def get_sensor_data():

    # Execute Only POST
    if request.method == "POST":

        # Parse JSON Data
        content = request.get_json()

        # Read sensor data
        sensor_data = content['samples']

        # Write CSV file
        write_csv(sensor_data)

    return str(ESP_current.get_red_current() << 4 | ESP_current.get_ir_current())


# Request for change current
@app.route("/changeCurrent", methods = ["POST"])
def change_current():

    # Read change_ir, change_red
    change_ir = int(request.form['change_ir'])
    change_red = int(request.form['change_red'])

    # Decide Current Level of ESP8266 Board
    ESP_current.set_ir_current(change_ir)
    ESP_current.set_red_current(change_red)

    # log
    print("changed current " + "ir: " + str(change_ir) + " / red: " + str(change_red))

    return render_template('test.html')


# Main
if __name__ == "__main__":

    # Initialize csv file
    init_csv()

    # Run App
    app.run(debug=True, host='0.0.0.0', port=8080)
