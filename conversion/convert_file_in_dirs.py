import struct
import csv

struct_format = '<4f'  # Little-Endian, 4 floats (time, x1, y1, z1)
struct_size = struct.calcsize(struct_format)

input_file = 'currentLog-54.csv'  # Name deiner Binärdatei
output_file = 'converted_data54.csv'

with open(input_file, 'rb') as bin_file, \
     open(output_file, 'w', newline='') as csv_file:
    
    writer = csv.writer(csv_file, delimiter=';')
    writer.writerow(['Time [s]', 'X [deg]', 'Y [deg]', 'Z [deg]'])
    
    while True:
        raw_data = bin_file.read(struct_size)
        if not raw_data:
            break
        time, x, y, z = struct.unpack(struct_format, raw_data)
        writer.writerow([f"{time:.3f}", f"{x:.2f}", f"{y:.2f}", f"{z:.2f}"])
    
print("Konvertierung abgeschlossen! CSV-Datei:", output_file)
