#ifndef CONF_DUMMY
#include <mosquitto.h>

#include <core/wrapper.h>

#include "mqtt.h"

CMQTT::CMQTT(void *pUserData, void (*MessageCallback)(void *, const char *, const void *, int), void(*ConnectCallback)(void *))
{
	m_pUserData = pUserData;
	m_MessageCallback = MessageCallback;
	m_ConnectCallback = ConnectCallback;
}

void CMQTT::OnMessage(struct mosquitto *m_pMosquitto, void *pUserData, const struct mosquitto_message *pMessage)
{
	CMQTT *pThis = (CMQTT *)pUserData;
	if (pMessage->payloadlen) {
		if (pThis->m_MessageCallback != 0x0)
			pThis->m_MessageCallback(pThis->m_pUserData, pMessage->topic, pMessage->payload, pMessage->payloadlen);
	}
	else {
		print("MQTT: %s (null)", pMessage->topic);
	}
}

void CMQTT::OnConnect(struct mosquitto *m_pMosquitto, void *pUserData, int Result)
{
	CMQTT *pThis = (CMQTT *)pUserData;
	if (!Result) {
		if (pThis->m_ConnectCallback != 0x0)
			pThis->m_ConnectCallback(pThis->m_pUserData);
	}
	else {
		print("MQTT: Connect failed");
	}
}

void CMQTT::OnSubscribe(struct mosquitto *m_pMosquitto, void *pUserData, int Mid, int QosCount, const int *pGrantedQos)
{
	print("MQTT: Subscribed (Mid: %d): %d", Mid, pGrantedQos[0]);
	for (int i = 1; i<QosCount; i++) {
		print("MQTT: %d", pGrantedQos[i]);
	}
}

void CMQTT::OnLog(struct mosquitto *m_pMosquitto, void *pUserData, int Level, const char *pStr)
{
	//print("MQTT: %s", pStr);
}

void CMQTT::LoopThread(void *pUser)
{
	CMQTT *pThis = (CMQTT *)pUser;

	while (true)
	{
		if (mosquitto_connect(pThis->m_pMosquitto, "127.0.0.1", 1883, 60))
			print("MQTT: Unable to connect.");
		else
		{
			int Result = mosquitto_loop_forever(pThis->m_pMosquitto, -1, 1);
			print("MQTT: MQTT Loop %i", Result);
		}

		thread_sleep(10000);
	}
}

void CMQTT::Init(const char *pName, const char *pPassword)
{
	mosquitto_lib_init();

	m_pMosquitto = mosquitto_new(0x0, true, 0x0);
	if (!m_pMosquitto) {
		print("MQTT: Error: Failed to initialize.");
		return;
	}

	mosquitto_user_data_set(m_pMosquitto, this);

	mosquitto_log_callback_set(m_pMosquitto, OnLog);

	mosquitto_connect_callback_set(m_pMosquitto, OnConnect);
	mosquitto_message_callback_set(m_pMosquitto, OnMessage);
	mosquitto_subscribe_callback_set(m_pMosquitto, OnSubscribe);

	mosquitto_username_pw_set(m_pMosquitto, pName, pPassword);

	thread_create(LoopThread, this);
}

void CMQTT::Publish(const char *pTopic, const void *pPayload, int PayloadSize)
{
	mosquitto_publish(m_pMosquitto, 0x0, pTopic, PayloadSize, pPayload, 0, 0);
}

void CMQTT::Subscribe(const char *pTopic, int QoS)
{
	mosquitto_subscribe(m_pMosquitto, 0x0, pTopic, QoS);
}

void CMQTT::Deinit()
{
	mosquitto_destroy(m_pMosquitto);
	mosquitto_lib_cleanup();
}
#endif