import io
import time
from Classifier import ImageClassifier
import numpy as np
from PIL import Image
from joblib import dump, load
import picamera


print("Starting camera")
camera = picamera.PiCamera()
camera.resolution = (352, 240)
camera.color_effects = (128, 128)  # turn camera to black and white

print("starting model training")

try:
    img_clf = load('model.joblib')
except Exception as e:
    img_clf = ImageClassifier()
    # load images
    (train_raw, train_labels) = img_clf.load_data_from_folder('./train/')
    # convert images into features
    train_data = img_clf.extract_image_features(train_raw)
    # train model and test on training data
    img_clf.train_classifier(train_data, train_labels)
    # dump classifier into file for later use
    dump(img_clf, 'model.joblib')

print("Model Trained")
camera.start_preview()

while True:
    time.sleep(2)
    camera.capture("current.bmp", resize=(352, 240))
    image = Image.open("current.bmp")
    data = np.array(image)
    # Construct a numpy array from the stream
    features = img_clf.extract_image_features([data])
    print(img_clf.predict_labels(features))
