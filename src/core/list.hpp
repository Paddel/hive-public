#pragma once

#include "wrapper.h"

template <class T>
class CList
{
	class CListItem
	{
	private:
		T m_Item;
		CListItem *m_pNext;

	public:

		CListItem(const T& Item) {
			m_Item = Item;
			m_pNext = 0x0;
		}

		void Execute(void(*ExecuteFunction)(void *pData, T Item), void *pData)
		{
			ExecuteFunction(pData, m_Item);

			if (m_pNext)
				m_pNext->Execute(ExecuteFunction, pData);
		}

		T *Get(int Index)
		{
			if (Index == 0)
				return &m_Item;
			else if (m_pNext != 0x0)
				return m_pNext->Get(Index - 1);
			else
				return 0x0;
		}

		T& Add(const T& Item)
		{
			dbg_assert(m_pNext != this, "list item loop");

			if (m_pNext == 0x0)
			{
				m_pNext = new CListItem(Item);
				return m_pNext->GetItem();
			}
			else
				return m_pNext->Add(Item);
		}

		bool Insert(const T& Item, int At)
		{
			if (At == 0)
			{
				CListItem *pNew = new CListItem(Item);
				pNew->SetNext(m_pNext);
				m_pNext = pNew;
				return true;
			}
			else if (m_pNext != 0x0)
				return m_pNext->Insert(Item, At - 1);
			return false;
		}

		void SetNext(CListItem *pNext)
		{
			dbg_assert(pNext != this, "list item loop");
			m_pNext = pNext;
		}

		CListItem *RemoveItem(int Index)
		{
			if (m_pNext == 0x0)
				return 0x0;

			if (Index == 0)
			{
				CListItem *pItem = m_pNext->GetNext();
				m_pNext = pItem;
				return pItem;
			}
			else
				return m_pNext->Remove(Index - 1);
		}

		CListItem *RemoveItem(const T& Item)
		{
			if (m_pNext == 0x0)
				return 0x0;

			if (m_pNext->GetItem() == Item)
			{
				CListItem *pItem = m_pNext->GetNext();
				m_pNext = pItem;
				return pItem;
			}
			else
				return m_pNext->Remove(Item);
		}

		void DeleteAll()
		{
			if (m_pNext != 0x0)
			{
				m_pNext->DeleteAll();
				delete m_pNext;
			}
			DeleteItem();
		}

		void DeleteItem()
		{
			delete m_Item;
		}

		T& GetItem() { return m_Item; }
		CListItem *GetNext() { return m_pNext; }
	};

private:
	CListItem *m_pFirst;
	int m_Size;

	T *Get(int Index)
	{
		if (m_pFirst != 0x0)
			return m_pFirst->Get(Index);
		return 0x0;
	}

	T& AddItem(const T& Item)
	{
		// dbg_assert(m_Size < 100000, "list size overflow");
		m_Size++;
		if (m_pFirst == 0x0)
		{
			m_pFirst = new CListItem(Item);
			return m_pFirst->GetItem();
		}
		else
			return m_pFirst->Add(Item);
	}

	CListItem *RemoveItem(int Index)
	{
		if (m_pFirst == 0x0)
			return 0x0;

		CListItem *pItem = 0x0;

		if (Index == 0)
		{
			pItem = m_pFirst->GetNext();
			m_pFirst = pItem;
			m_Size--;
		}
		else
		{
			pItem = m_pFirst->Remove(Index - 1);
			if (pItem != 0x0)
				m_Size--;
		}

		return pItem;
	}

	CListItem *RemoveItem(const T& Item)
	{
		if (m_pFirst == 0x0)
			return 0x0;

		CListItem *pItem = 0x0;

		if (m_pFirst->GetItem() == Item)
		{
			CListItem *pItem = m_pFirst->GetNext();
			m_pFirst = pItem;
			m_Size--;
		}
		else
		{
			pItem = m_pFirst->Remove(Item);
			if (pItem != 0x0)
				m_Size--;
		}

		return pItem;
	}

public:
	CList()
	{
		m_pFirst = 0x0;
		m_Size = 0;
	}

	void Execute(void(*ExecuteFunction)(void *pData, T Item), void *pData)
	{
		if (m_pFirst)
			m_pFirst->Execute(ExecuteFunction, pData);
	}

	void Add(const T& Item)
	{
		AddItem(Item);
	}

	void Insert(const T& Item, int At)
	{
		if (At == 0)
		{
			CListItem *pNew = new CListItem(Item);
			pNew->SetNext(m_pFirst);
			m_pFirst = pNew;
			m_Size++;
		}
		else if (m_pFirst != 0x0)
			if(m_pFirst->Insert(Item, At - 1) == true)
				m_Size++;
	}

	void Remove(int Index)
	{
		CListItem *pItem = RemoveItem(Index);
		if (pItem != 0x0)
			delete pItem;
	}

	void Remove(const T& Item)
	{
		CListItem *pItem = RemoveItem(Item);
		if (pItem != 0x0)
			delete pItem;
	}

	void RemoveAll()
	{
		m_pFirst = 0x0;
		m_Size = 0;
	}

	void Delete(int Index)
	{
		CListItem *pItem = RemoveItem(Index);
		if (pItem != 0x0)
		{
			delete pItem->DeleteItem();
			delete pItem;
		}
	}

	void Delete(const T& Item)
	{
		CListItem *pItem = Remove(Item);
		if (pItem != 0x0)
		{
			delete pItem->DeleteItem();
			delete pItem;
		}
	}

	void DeleteAll()
	{
		if (m_pFirst != 0x0)
		{
			m_pFirst->DeleteAll();
			delete m_pFirst;
		}

		m_pFirst = 0x0;
		m_Size = 0;
	}


	T& operator[] (int Index) { return *Get(Index); };
	const T& operator[] (int Index) const { return *Get(Index); };
	T& operator +=(const T& Item) { return AddItem(Item); };

	int Size() const { return m_Size; };
};