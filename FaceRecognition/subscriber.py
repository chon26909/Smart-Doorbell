from indentify import *
import paho.mqtt.client as mqtt
import datetime
import cv2
import time



def on_connect(client, userdata, flags, rc):

    print("Connected with result code "+str(rc))
    client.subscribe("camera/picture")


def on_message(client, userdata, message):

    if (message.topic == "camera/picture"):
        print("Take Picture")
        # suffix = datetime.datetime.now().strftime("%y%m%d%H%M%S")

        filename_image = r"D:/IOT/Smart-Doorbell/FaceRecognition/result.jpg"

        print(filename_image)
        image = cv2.imread('./result.jpg')

        cv2.waitKey(0) 
  
        #closing all open windows 
        # cv2.destroyAllWindows() 

        f = open(filename_image, "wb")  # 'w' for 'write', 'b' for 'write as binary, not text'
        f.write(message.payload)
        # cv2.imshow(window_name, image)
        f.close()

        #time.sleep(1)
        face_reG()



client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set("chon", "1234")
client.connect("171.6.128.164", 1883, 60)

client.loop_forever()