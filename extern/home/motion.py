import cv2, time, imageio, threading, os
from datetime import datetime

RECORDS_FOLDER = '/home/records'

def save_capture(args):
    if(len(frames) > 1):
        frames_copy = frames.copy()
        del frames[:]
        now = datetime.now().strftime('%Y-%m-%d--%H-%M-%S-%f')[:-3]
        file_name = RECORDS_FOLDER + '/' + now + '.gif'
        imageio.mimsave(file_name, frames_copy, duration=500)
        del frames_copy[:]
    else:
        del frames[:]

if __name__ == "__main__":
    last_frame = None
    video = cv2.VideoCapture(0)
    frames = []
    captureTime = 0
    if video.isOpened() == False:
        exit(1)

    print(os.getpid(), flush=True)

    if os.path.exists(RECORDS_FOLDER) == False:
        print('Path "' + RECORDS_FOLDER + "' does not exist.")
        exit(1)

    while True:
        check, frame = video.read()
        if check == False:
            continue

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        gray = cv2.GaussianBlur(gray, (21, 21), 0)

        if last_frame is None:
            last_frame = gray
            continue
    
        diff_frame = cv2.absdiff(last_frame, gray)
        last_frame = gray
    
        thresh_frame = cv2.threshold(diff_frame, 30, 255, cv2.THRESH_BINARY)[1]
        thresh_frame = cv2.dilate(thresh_frame, None, iterations = 2)
    
        cnts,_ = cv2.findContours(thresh_frame.copy(),
                        cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
        motion = False
        for contour in cnts:
            if cv2.contourArea(contour) < 200:
                continue
            #(x, y, w, h) = cv2.boundingRect(contour)
            #cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
            motion = True

        if motion is True:
            if len(frames) == 0:
                    print('.', flush=True)
            capture_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            capture_frame = cv2.resize(capture_frame, (640, 640))
            frames.append(capture_frame)
            captureTime = time.time()

        if len(frames) > 0 and time.time() - captureTime > 3:
            x = threading.Thread(target=save_capture, args=(1,))
            x.start()
            #save_capture(None)
            #cv2.imwrite(now + '.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 50])
    
        #cv2.imshow("Gray Frame", gray)
        #cv2.imshow("Difference Frame", diff_frame) 
        #cv2.imshow("Threshold Frame", thresh_frame)
        #cv2.imshow("Color Frame", frame)
        #cv2.waitKey(1)
    
    #video.release()
    #cv2.destroyAllWindows()