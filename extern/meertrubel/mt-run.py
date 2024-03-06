import os
import firebase_admin
from firebase_admin import credentials
from firebase_admin import messaging

key_path = os.path.dirname(os.path.abspath(__file__)) + "/firebase-mt.json"
cred = credentials.Certificate(key_path)
firebase_admin.initialize_app(cred)


message = messaging.Message(
	android=messaging.AndroidConfig(
		priority='high',
	),
    topic='meertrubel',
)
response = messaging.send(message)