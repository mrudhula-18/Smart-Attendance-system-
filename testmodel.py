import cv2
import requests
import time

# Your deployed Google Apps Script Web App URL
web_app_url = "https://script.google.com/macros/s/AKfycbxOp6eQklSo8HoRbbQoLEiR5E941uzn6v6cPplEKNtthgMSZjk2PY5McSnYJM-yzfZGuA/exec" \
""

# Load trained model and Haar cascade
recognizer = cv2.face.LBPHFaceRecognizer_create()
recognizer.read("Trainer.yml")
facedetect = cv2.CascadeClassifier("haarcascade_frontalface_default.xml")
video = cv2.VideoCapture(0)

# Load names from file
name_dict = {}
try:
    with open("names.txt", "r") as f:
        for line in f:
            id, name = line.strip().split(",")
            name_dict[int(id)] = name
except:
    print("No names found. Please add users via dataset collection.")

recognized = False
recognized_time = 0

while True:
    ret, frame = video.read()
    if not ret:
        print("Failed to capture frame from camera.")
        break

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces = facedetect.detectMultiScale(gray, 1.3, 5)

    for (x, y, w, h) in faces:
        id_pred, conf = recognizer.predict(gray[y:y+h, x:x+w])

        if conf < 50:
            name = name_dict.get(id_pred, "Unknown")
        else:
            name = "Unknown"

        cv2.putText(frame, name, (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 1, (50, 50, 255), 2)
        cv2.rectangle(frame, (x, y), (x + w, y + h), (50, 50, 255), 2)

        # If recognized and not yet logged
        if name != "Unknown" and not recognized:
            try:
                params = {'name': name}  # ← Changed from 'name' to 'face'
                response = requests.get(web_app_url, params=params)
                print(f"Logged attendance for: {name}")
                print("Response:", response.text)
            except Exception as e:
                print("Failed to send to Google Sheets:", e)

            recognized = True
            recognized_time = time.time()

    cv2.imshow("Face Recognition", frame)

    # Close after 3 seconds of successful recognition or if 'q' is pressed
    if recognized and (time.time() - recognized_time) > 3:
        break
    if cv2.waitKey(1) == ord("q"):
        break

video.release()
cv2.destroyAllWindows()
