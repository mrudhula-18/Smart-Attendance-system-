import cv2
import os

video = cv2.VideoCapture(0)
facedetect = cv2.CascadeClassifier("haarcascade_frontalface_default.xml")

# Ask user for ID and Name
id = input("Enter your ID (numeric): ")
name = input("Enter your name: ")

# Create datasets directory if not exists
if not os.path.exists("datasets"):
    os.makedirs("datasets")

count = 0

while True:
    ret, frame = video.read()
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces = facedetect.detectMultiScale(gray, 1.3, 5)

    for (x, y, w, h) in faces:
        count += 1
        filename = f"datasets/User.{id}.{count}.jpg"
        cv2.imwrite(filename, gray[y:y+h, x:x+w])
        cv2.rectangle(frame, (x, y), (x+w, y+h), (50, 50, 255), 1)

    cv2.imshow("Collecting Dataset", frame)
    if cv2.waitKey(1) == ord("q") or count >= 100:
        break

video.release()
cv2.destroyAllWindows()

# Save name in a file
with open("names.txt", "a") as f:
    f.write(f"{id},{name}\n")

print("Dataset Collection Done.")
