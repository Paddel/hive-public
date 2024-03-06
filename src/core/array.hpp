#pragma once

template <class T>
class CArray
{
private:
	T *m_pArray;
	int m_NumElements;
	int m_Size;
	bool m_AutoOptimize;

	void Realloc()
	{
		if(m_NumElements > 0)
		{
			T *pNewList = new T[m_NumElements];
			if (m_pArray != 0x0)
			{
				int To = m_Size < m_NumElements ? m_Size : m_NumElements;
				for (int i = 0; i < To; i++)
					pNewList[i] = m_pArray[i];
				delete[] m_pArray;
			}
			m_pArray = pNewList;
		}
		else
			m_pArray = 0x0;
		
		m_Size = m_NumElements;
	}

public:
	CArray(bool AutoOptimize = true)
	{
		m_AutoOptimize = AutoOptimize;

		m_pArray = 0x0;
		m_NumElements = 0;
		m_Size = 0;
	}

	T& Get(int Index)
	{
		return m_pArray[Index];;
	}

	virtual void Add(const T& Item)
	{
		m_NumElements++;
		Realloc();
		m_pArray[m_NumElements - 1] = Item;
	}

	void Insert(const T& Item, int At)
	{
		m_NumElements++;
		Realloc();
		for(int i = m_NumElements - 1; i > At; i--)
			m_pArray[i] = m_pArray[i - 1];
		m_pArray[At] = Item;
	}

	virtual void Remove(int Index)
	{
		for(int i = Index; i < m_NumElements - 1; i++)
			m_pArray[i] = m_pArray[i + 1];
		m_NumElements--;
		if (m_AutoOptimize == true)
			Realloc();
	}

	void Remove(const T& Item)
	{
		for (int i = 0; i < m_NumElements; i++)
		{
			if (m_pArray[i] == Item)
			{
				Remove(i);
				return;
			}
		}
	}

	void RemoveAll()
	{
		m_pArray = 0x0;
		m_NumElements = 0;
	}

	void Delete(int Index)
	{
		delete m_pArray[Index];
		Remove(Index);
	}

	void Delete(const T& Item)
	{
		delete Item;
		Remove(Item);
	}

	void DeleteAll()
	{
		delete[] m_pArray;
		m_pArray = 0x0;
		m_NumElements = 0;
	}

	void Optimize() { Realloc(); };
	void SetAutoOptimize(bool Value) { m_AutoOptimize = Value; };
	T *BasePointer() const { return m_pArray; }

	T& operator[] (int Index) { return m_pArray[Index]; };
	const T& operator[] (int Index) const { return m_pArray[Index]; };
	T& operator +=(const T& Item) { Add(Item); return m_pArray[m_NumElements-1]; };

	int Size() const { return m_NumElements; };
};