import sys
import time
import picamera



myargs = sys.argv[1:]

camera = picamera.PiCamera()
camera.resolution = (352, 240)
camera.color_effects = (128, 128)  # turn camera to black and white
camera.start_preview()
time.sleep(2)


if len(myargs) <= 1:
    sys.exit("Incorrect arguments")

num_images_per_type = int(myargs[0])  # number of images to take of each type of object

print("Taking ", num_images_per_type, "images each of ", myargs[1:])

for image_type in myargs[1:]:
    for i in range(num_images_per_type):
        filename = image_type + "_" + str(i) + ".bmp"
        camera.capture("images/"+filename, format='rgb', resize=(352, 240))
        print(filename)
        time.sleep(1)
