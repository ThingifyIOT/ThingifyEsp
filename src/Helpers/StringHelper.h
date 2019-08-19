#ifndef _STRING_HELPER_H
#define _STRING_HELPER_H

#include <FixedString.h>

class StringHelper
{
public:
	template<int N>
	static FixedString<N> GenerateRandomString()
	{
		FixedString<N> randomString;
		for(int i=0; i < N-1; i++)
		{
			char c;
			switch (random(0, 2))
			{
			case 0:
				c = random('A', 'Z');
				break;
			case 1:
				c = random('a', 'z');
				break;
			case 2:
				c = random('0', '9');
				break;
			default:;
			}

			randomString.append(c);
		}
		return randomString;
	}
};
#endif
