import csv

class CSV_file:
    raw_data = []


def writeCSV(count, data):

    # Open File cusor
    f = open("sensor/sensorData" + str(count) + ".csv", "w", encoding="utf-8", newline='')
    writer = csv.writer(f, delimiter = ",", quotechar = '"')

    writer.writerow(["time","ir","red","x","y","z", "ir_current", "red_current"])

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