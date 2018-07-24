import csv

def writeCSV(count, data):

    # Open File cusor
    f = open("sensor/sensorData" + str(count) + ".csv", "w", encoding="utf-8", newline='')
    writer = csv.writer(f, delimiter = ",", quotechar = '"')

    writer.writerow(["time","ir","red","x","y","z"])

    # Execute JSON Length
    for temp in data:

        # Read Sensor Data
        time = temp['time']
        ir = str(temp['ir'])
        red = str(temp['r'])
        x = str(temp['x'])
        y = str(temp['y'])
        z = str(temp['z'])

        # Write CSV
        writer.writerow([time, ir, red, x, y, z])

        # Print Test Data
        # print(time)
        # print(ir)
        # print(red)
        # print(x)
        # print(y)
        # print(z)

    # Close File cusor
    f.close()

    print("CSV file write")

    return