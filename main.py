import io
import time
from Classifier import ImageClassifier
import numpy as np
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
    stream = io.BytesIO()
    time.sleep(2)

    output = np.empty((240, 352, 3), dtype=np.uint8)
    camera.capture(output, format="rgb", resize=(352, 240))

    # Construct a numpy array from the stream
    features = img_clf.extract_image_features([output])
    print(img_clf.predict_labels(features))
