
offset = 0x69e30

with open("FontData_out.txt", "rb") as file:
    data = file.read()

with open("dds.exe", "r+b") as file:
    file.seek(offset)
    file.write(data)
    