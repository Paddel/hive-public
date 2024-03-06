#pragma once

class CWasp;
class CUnpacker;

class IExtension
{
private:
	CWasp *m_pWasp;

public:
	IExtension(CWasp *pWasp) { m_pWasp = pWasp; }

	virtual void Init() {};
	virtual bool ParsePacket(int MsgID, CUnpacker& Unpacker) { return false; }
	virtual void Tick() {};

	CWasp *GetWasp() const { return m_pWasp; };
};