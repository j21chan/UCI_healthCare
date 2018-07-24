# Import Library
from flask import Flask, request
from datetime import datetime
import CSV as csv
import count as Count
import current_level as Current


# Create App
app = Flask(__name__)


# Get Current Time
@app.route("/time", methods = ["GET", "POST"])
def getCurrentTime():
    return str(time.time())


# Receive Sensor JSON data
@app.route("/sensorData", methods = ["POST"])
def sensorData():

    # Increase Count
    Count.count.increase_num()

    # Execute Only POST
    if request.method == "POST":

        # Parse JSON Data
        content = request.get_json()

        # Write CSV file
        csv.writeCSV(Count.count.num, content['samples'])

        # Decide Current Level of ESP8266 Board

        # Method of Decision

        Current.current_level.setIrCurrentLevel(8)
        Current.current_level.setRedCurrentLevel(8)

    return "sensorData Test"


# Changing ESP8266 IR current
@app.route("/getIRCurrent", methods = ["GET"])
def getIRCurrent():

    return str(Current.current_level.getIrCurrentLevel())


# Changing ESP8266 IR current
@app.route("/getREDCurrent", methods = ["GET"])
def getREDCurrent():

    return str(Current.current_level.getRedCurrentLevel())

# Main
if __name__ == "__main__":
    app.run(debug=True, host='0.0.0.0', port=8080)