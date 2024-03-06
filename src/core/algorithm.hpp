#pragma once

template <class T>
void sort_bubble(T *Items, int Count)
{
	T Item;
	for (int i = 1; i < Count; i++)
	{
		for (int j = Count - 1; j >= i; j--)
		{
			if (Items[j - 1] - Items[j] > 0)
			{
				Item = Items[j - 1];
				Items[j - 1] = Items[j];
				Items[j] = Item;
			}
		}
	}
}

template <class T, typename U>
int search_binary(T *Items, int Count, U Key)
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
	return -1;
}

template<class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi)
{
	return v <= lo ? lo : v >= hi ? hi : v;
}