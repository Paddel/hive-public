#pragma once

#include "array.hpp"

template <class T>
class CArraySorted : public CArray<T>
{
private:

	int search(T *Items, int Count, T Key, bool Closest = false)
	{
		int WinMin = 0;
		int WinMax = Count - 1;
		while (WinMin <= WinMax)
		{
			int CurIndex = WinMin + (WinMax - WinMin) / 2;
			int Comp = Items[CurIndex] - Key;
			if (Comp > 0)
				WinMax = CurIndex - 1;
			else if (Comp < 0)
				WinMin = CurIndex + 1;
			else
				return CurIndex;
		}

		return Closest ? WinMin : -1;
	}

public:

	virtual void Add(const T& Item)
	{
		CArray<T>::Insert(Item, search(CArray<T>::BasePointer(), CArray<T>::Size(), Item, true));
	}

	void Remove(int Index)
	{
		T *pBase = CArray<T>::BasePointer();
		for (int i = Index + 1; i < CArray<T>::Size(); i++)
			pBase[i - 1] = pBase[i];
		CArray<T>::Remove(CArray<T>::Size() - 1);
	}
};