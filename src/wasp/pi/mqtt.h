#pragma once

class CMQTT
{
private:
	struct mosquitto *m_pMosquitto;
	void *m_pUserData;
	void(*m_MessageCallback)(void *, const char *, const void *, int);
	void(*m_ConnectCallback)(void *);

	static void OnMessage(struct mosquitto *m_pMosquitto, void *pUserData, const struct mosquitto_message *pMessage);
	static void OnConnect(struct mosquitto *m_pMosquitto, void *pUserData, int Result);
	static void OnSubscribe(struct mosquitto *m_pMosquitto, void *pUserData, int Mid, int QosCount, const int *pGrantedQos);
	static void OnLog(struct mosquitto *m_pMosquitto, void *pUserData, int Level, const char *pStr);
	static void LoopThread(void *pUser);

public:
	CMQTT(void *pUserData, void(*MessageCallback)(void *, const char *, const void *, int), void(*ConnectCallback)(void *));

	void Init(const char *pName, const char *pPassword);
	void Deinit();
	void Publish(const char *pTopic, const void *pPayload, int PayloadSize);
	void Subscribe(const char *pTopic, int QoS);
};