import face_recognition
from PIL import Image, ImageDraw
import cv2
import time
#from toml import load
from parinya import LINE as Line
line = Line('QNMFunzQzONROzHBCFyE3lefVfPN4jgmx54V9x1Y9cZ')


def face_reG():
    image_of_chon = face_recognition.load_image_file('./Dataset/chontrain.png')
    chon_face_encoding = face_recognition.face_encodings(image_of_chon)[0]

    image_of_boss = face_recognition.load_image_file('./Dataset/Boss.jpg')
    boss_face_encoding = face_recognition.face_encodings(image_of_boss)[0]

    image_of_toy = face_recognition.load_image_file('./Dataset/thianrawit.jpg')
    toy_face_encoding = face_recognition.face_encodings(image_of_toy)[0]

    known_face_encodings =[
        chon_face_encoding, 
        boss_face_encoding,
        toy_face_encoding
        ]

    known_face_name = [
        "Chon", 
        "Boss",
        "TOY"
    ]

    # Load test Image
    #path=r'D:/IOT/Smart-Doorbell/FaceRecognition/chonPR.jpg'
    # cv2.imread(path)
    #filename_image = r"D:/IOT/Smart-Doorbell/FaceRecognition/chonPR.jpg"
    test_first = face_recognition.load_image_file('./result.jpg')
    loadimage = cv2.imread('./result.jpg')
    h , w , c = loadimage.shape
    if w - h :
        test_image = cv2.rotate(test_first, cv2.ROTATE_90_CLOCKWISE)
    else:     
        test_image = face_recognition.load_image_file('./result.jpg')
    
    # filename_image = r"D:/IOT/Smart-Doorbell/FaceRecognition/chonPR.jpg"
    #test_image = face_recognition.load_image_file(r'D:/IOT/Smart-Doorbell/FaceRecognition/chonPR.jpg')

    # Find faces in test image
    face_locations = face_recognition.face_locations(test_image)
    face_encodings = face_recognition.face_encodings(test_image, face_locations)

    # Convert to PIL format
    pil_image = Image.fromarray(test_image)

    # Create a ImageDraw instances
    draw = ImageDraw.Draw(pil_image)

    # Loop through face in test image
    for(top, right , bottom , left), face_encoding in zip(face_locations, face_encodings):
        matches = face_recognition.compare_faces(known_face_encodings, face_encoding)

        name = "Unknow person"
        print("Kao ma lae ==================>")
        #If match
        if True in matches:
            first_match_index = matches.index(True)
            name = known_face_name[first_match_index]
        #Draw Box 
        draw.rectangle(((left,top) , (right , bottom)) , outline = (0,0,0))

        #Draw labels
        text_width , text_hieght = draw.textsize(name)
        time.sleep(1)
        Line.sendimage(line,test_image,"User ==> " + name )
        draw.rectangle(((left, bottom - text_hieght - 10), (right, bottom)), fill=(0,0,0), outline=(0,0,0))
        draw.text((left + 6, bottom - text_hieght - 5), name , fill=(255,255,255,255))

    del draw

    #Display image
    pil_image.show()
