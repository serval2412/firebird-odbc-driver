// Values.h: interface for the Values class.
//
//////////////////////////////////////////////////////////////////////

// copyright (c) 1999 - 2000 by James A. Starkey for IBPhoenix.


#if !defined(_VALUES_H_INCLUDED_)
#define _VALUES_H_INCLUDED_

class Value;


class Values  
{
public:
	void clear();
	void alloc (int n);
	Values();
	virtual ~Values();

	int		count;
	Value	*values;
};

#endif // !defined(_VALUES_H_INCLUDED_)
