import sys
import time
import picamera


def main():
    myargs = sys.argv[1:]

    camera = picamera.PiCamera()

    if len(myargs) <= 1:
        sys.exit("Incorrect arguments")

    num_images_per_type = int(myargs[0])  # number of images to take of each type of object

    print("Taking ", num_images_per_type, "images each of ", myargs[1:])

    for image_type in myargs[1:]:
        for i in range(num_images_per_type):
            time.sleep(.5)
            camera.start_preview()
            time.sleep(1)
            filename = image_type + "_" + str(i) + ".bmp"
            camera.capture(filename, format='bmp')
            camera.stop_preview()
            print(image_type + str(i))
            time.sleep(2)
