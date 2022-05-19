#ifndef __EXTSTRING_H__INCLUDED__
#define __EXTSTRING_H__INCLUDED__

#include <cextra.h>

INLINE int istrcmp(const char *s1, const char *s2)
{
	char *ptr1 = (char*)s1;
	char *ptr2 = (char*)s2;
	int c1, c2;
	
	do
	{
		c1 = *ptr1++;
		c2 = *ptr2++;
			
		if(c1 >= 'A' && c1 <= 'Z')
		{
			c1 = c1 - 'A' + 'a';
		}
		
		if(c2 >= 'A' && c2 <= 'Z')
		{
			c2 = c2 - 'A' + 'a';
		}
			
		if(c1 != c2)
		{
			return c1-c2;
		}
	}
	while(c1 != '\0' && c2 != '\0');
	
	return 0;
}

INLINE int istrncmp(const char *s1, const char *s2, size_t num)
{
	char *ptr1 = (char*)s1;
	char *ptr2 = (char*)s2;
	int c1, c2;
	
	do
	{
		if(num-- == 0)
		{
			break;
		}
		
		c1 = *ptr1++;
		c2 = *ptr2++;
			
		if(c1 >= 'A' && c1 <= 'Z')
		{
			c1 = c1 - 'A' + 'a';
		}
		
		if(c2 >= 'A' && c2 <= 'Z')
		{
			c2 = c2 - 'A' + 'a';
		}
		
		if(c1 != c2)
		{
			return c1-c2;
		}
	}
	while(c1 != '\0' && c2 != '\0');
	
	return 0;
}

#endif /* __EXTSTRING_H__INCLUDED__ */
