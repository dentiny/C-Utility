#ifndef _MACRO_H_
#define _MACRO_H_

#define DISALLOW_COPY_AND_ASSIGN(Class) \
	Class(const Class &) = delete; \
	Class & operator=(const Class &) = delete;

#endif // _MACRO_H_