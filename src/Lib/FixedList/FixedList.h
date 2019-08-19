#ifndef _FIXED_LIST_H
#define _FIXED_LIST_H

#include <stdint.h>
extern bool FixedList_OverflowDetected;

template <class T, unsigned char CAPACITY>
class FixedList
{
	//uint32_t used here to ensure _array is aligned to 4
	uint32_t _array[(CAPACITY*sizeof(T))/sizeof(uint32_t) +1];
	unsigned char _length;
public:

	FixedList()
	{
		_length = 0;
	}

	FixedList(const FixedList& list)
	{
		clear();
		for (int i = 0; i < list.size(); i++)
		{
			add(list[i]);
		}
	}

	void add(T& item)
	{
		if (_length >= CAPACITY)
		{
			FixedList_OverflowDetected = true;
			return;
		}
		T* ptrArray = reinterpret_cast<T*>(_array);
		new (&ptrArray[_length])T(item);
		_length++;
	}

	T& operator[](int index) const
	{
		T* ptrArray = const_cast<T*>(reinterpret_cast<const T*>(_array));
		return ptrArray[index];
	}

	unsigned char size() const
	{
		return _length;
	}

	void clear()
	{
		_length = 0;
	}

	bool empty()
	{
		return _length == 0;
	}

	T* begin()
	{
		T* ptrArray = reinterpret_cast<T*>(_array);
		return &ptrArray[0];
	}
	T* end()
	{
		T* ptrArray = reinterpret_cast<T*>(_array);
		return &ptrArray[_length];
	}
};

#endif
