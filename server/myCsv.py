import csv


"""
function name       : init_csv
input parameter     : none
output parameter    : none
function            : initialize csv file
"""
def init_csv():

    # Open File cusor
    f = open("sensor/sensorData.csv", "w", encoding="utf-8", newline='')

    # create file writer
    writer = csv.writer(f, delimiter = ",", quotechar = '"')

    # init csv file column
    writer.writerow(["time","ir","red","x","y","z", "ir_current", "red_current"])

    # file close
    f.close()

    return


"""
function name       : write_csv
input parameter     : sensor data list ["time","ir","red","x","y","z", "ir_current", "red_current"]
output parameter    : none
function            : append sensor data to csv file
"""
def write_csv(data):

    # Open File cusor
    f = open("sensor/sensorData.csv", "a", encoding="utf-8", newline='')
    writer = csv.writer(f, delimiter = ",", quotechar = '"')

    # Execute JSON Length
    for temp in data:

        # Read Sensor Data
        time = temp['time']
        ir = str(temp['ir'])
        red = str(temp['r'])
        x = str(temp['x'])
        y = str(temp['y'])
        z = str(temp['z'])
        ir_current = str(temp['ir_c'])
        red_current = str(temp['red_c'])

        # Write CSV
        writer.writerow([time, ir, red, x, y, z, ir_current, red_current])

    # Close File cusor
    f.close()

    print("CSV file write")

    return