#ifndef __POLY_SYMTABLE_H__
#define __POLY_SYMTABLE_H__

#ifdef __cplusplus
#include <string>

class CSymTable  
{
    public:
	CSymTable(int aTok, std::string aStr);
	virtual ~CSymTable();

	double		dVal;
	int		token;
	std::string	strlex;
};
#else
#include_next "symtable.h"
#endif

#endif 
