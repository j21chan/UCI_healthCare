# Import Library
from flask import Flask, request
import time
import CSV as csv
import count as Count
import current_level as Current
from flask.templating import render_template


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

        # Write 250 data
        if Count.count.num % 5 == 0:

            # Write CSV file
            csv.writeCSV(Count.count.num,
                         csv.CSV_file.raw_data,
                         Current.current_level.getIrCurrentLevel(),
                         Current.current_level.getRedCurrentLevel())

        # Append data list
        else:

            # Append sensor data list
            csv.CSV_file.raw_data.append(content['samples'])
            print(csv.CSV_file.raw_data)

    return "sensorData Test"


# Changing ESP8266 IR current
@app.route("/getIRCurrent", methods = ["GET"])
def getIRCurrent():

    return str(Current.current_level.getIrCurrentLevel())


# Changing ESP8266 IR current
@app.route("/getREDCurrent", methods = ["GET"])
def getREDCurrent():

    return str(Current.current_level.getRedCurrentLevel())


# Request for change current
@app.route("/changeCurrent", methods = ["POST"])
def changeCurrent():

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
