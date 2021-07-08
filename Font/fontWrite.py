from PIL import Image

def main():
    with open("FontData_out.txt","wb") as file:
        for i in range(0x1f + 1,0xe0):
            im = Image.open("modified\\{}.bmp".format(i))
            im = im.convert("1")
            data = im.tobytes("raw", "1", 0,1)
            for x in data:
                file.write(bytes([x]))
main()