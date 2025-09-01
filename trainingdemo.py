import cv2
import numpy as np
from PIL import Image
import os

# Initialize the recognizer
recognizer = cv2.face.LBPHFaceRecognizer_create()
path = "datasets"

def getImageID(path):
    imagePaths = [os.path.join(path, f) for f in os.listdir(path) if f.endswith(".jpg")]
    faces = []
    ids = []

    for imagePath in imagePaths:
        try:
            # Convert image to grayscale
            faceImage = Image.open(imagePath).convert('L')
            faceNP = np.array(faceImage, 'uint8')

            # Extract ID from the filename
            Id = int(os.path.split(imagePath)[-1].split(".")[1])

            faces.append(faceNP)
            ids.append(Id)

            cv2.imshow("Training", faceNP)
            cv2.waitKey(10)

        except Exception as e:
            print(f"⚠️ Skipped file {imagePath}: {e}")
            continue

    return ids, faces

print("[INFO] Collecting face data...")
IDs, facedata = getImageID(path)

print(f"[INFO] Total images found: {len(facedata)}")
if len(facedata) == 0:
    print("❌ No valid images found in the dataset. Training aborted.")
    exit()

print("[INFO] Training the model...")
recognizer.train(facedata, np.array(IDs))
recognizer.write("Trainer.yml")
cv2.destroyAllWindows()

print("✅ Training Completed Successfully.")
