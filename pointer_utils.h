#ifndef POINTER_UTILS_H
#define POINTER_UTILS_H

namespace pointer_utils
{
	template<class T>
	void delete_ptr(T * ptr)
	{
		delete ptr;
	};
}

#endif // POINTER_UTILS_H