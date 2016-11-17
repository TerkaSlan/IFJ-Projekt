/**
 *  \file ial.h
 *  \brief Contains declarations structures and functions regarding Symbol table
 *  \author CharvN
 *  \date 15. 10. 2016
 */

#ifndef IAL_H
#define IAL_H

#include "str.h"
#include <stdint.h>
#include <stdbool.h>
#include "instruction.h"

/**
 *  \brief Typedef enum of Symbol types.
 */
typedef enum {
	eNULL,
	eINT,
	eDOUBLE,
	eBOOL,
	eSTRING,
	eFUNCTION,
	eCLASS,
} eSymbolType;


struct tHashTableItem;

/**
 *  \brief Typedef of Hash Table (symbol table) structure
 */
typedef struct {
	uint32_t              Size;               ///Size of rows of the hash table
	uint32_t              NumberOfItems;      ///Number of symbols in the hash table
	struct tHashTableItem *Parent;            ///Pointer to the parent symbol, if there is one
	struct tHashTableItem *Data[];            ///Symbol pointer array
} *tHashTablePtr;

/*
 * \brief Typedef struct of argument list item
 */
typedef struct tArgumentListItem {
	struct tArgumentListItem *Next;
	struct tHashTableItem    *Symbol; //pointer to a symbol in the local symbol table for quick access
} tArgumentListItem;

/**
 *  \brief Typedef struct of function symbol data
 */
typedef struct {
	tArgumentListItem *ArgumentList;         ///Pointer to Linked list of function arguments
	uint32_t          NumberOfArguments;    ///Number of parameters the function has
	eSymbolType       ReturnType;           ///Type of return value
	uint32_t          InstructionIndex;     ///Pointer to the first instruction of this funtion
	tHashTablePtr     LocalSymbolTable;     ///Pointer to the local symbol hash table.
} tFuncData;

/*
 * \brief Typedef struct of class data
 */
typedef struct {
	tHashTablePtr LocalSymbolTable;
} tClassData;


/**
 *  \brief Typedef union of Symbol Data
 */
typedef union {
	tClassData ClassData;
	tFuncData  FunctionData;
	int32_t    Integer;     //64?
	double     Double;
	bool       Bool;
	dtStrPtr   String;
} tSymbolData;


/**
 * \brief Typedef of Symbol structure
 */
typedef struct tHashTableItem {
	struct tHashTableItem *Next;            ///Pointer to the next symbol in the linked list
	dtStrPtr              Name;             ///Pointer to string structure representing Name of the symbol
	eSymbolType           Type;             ///Symbol Type
	bool                  Defined;          ///Defined - if static var/func is initialized/defined. note: initialized flag for local variables is stored in the local frame, not here
	bool                  Const;            ///Constant/static flag - data are stored globaly (in the symbol): eg literals, static variables
	int32_t               Index;            ///index to local frame, -1 not defined
	tSymbolData           Data;             ///Symbol Data for literals/static variables
} tSymbol, *tSymbolPtr;


#define HTAB_DEFAULT_SIZE 4099

/**
 *  \brief Creates a new hash table of size specified.
 *
 *	\param [in] size, Size of hash table created (number of rows), should be prime number
 *  \return tHashTablePtr, Returns pointer to the new hash table. If an error occures, NULL is returned.
 *
 */
tHashTablePtr htabInit(uint32_t size);

/**
 *  \brief Creates a new hash table identic to the hash table passed as an argument.
 *
 *  \param [in] table tHashTablePtr, Pointer to a hash table to be copied.
 *  \return tHashTablePtr, Returns a pointer to the new hashtable. If an error occures, NULL is returned.
 *
 *  \details If table param is NULL, nothing happens and NULL is returned.
 */
tHashTablePtr htabCopy(const tHashTablePtr table);


/**
 * \brief Generates frame index for each symbol
 * \param [in] table tHashTablePtr, Pointer to the hash table
 */
void htabGenerateIndexes(tHashTablePtr table);

/**
 *  \brief Adds Symbol to the hash table.
 *
 *  \param [in] table tHashTablePtr, Pointer to the hashtable.
 *  \param [in] symbol tSymbolPtr, Pointer to the symbol to be added.
 *  \param [in] overwrite bool, specifies if symbols should be overwritten if already exist
 *  \return tSymbolPtr, Returns pointer to the added symbol. If symbol with the same name already exists, or other error occures, nothing happens and NULL is returned.
 *
 *  \details Hash table creates its own copy of a symbol passed by reference and deallocates it when htabFree() is called.
 */
tSymbolPtr htabAddSymbol(tHashTablePtr table, const tSymbolPtr symbol, bool overwrite);

/**
 *  \brief Looks up a symbol in the hash table with the name specified.
 *
 *  \param [in] table tHashTablePtr, Pointer to the hashtable.
 *  \param [in] name dtStrPtr, Pointer to a string structure representing name of symbol.
 *  \return tSymbolPtr, Returns pointer to the symbol, if no symbol with such name exists, NULL is returned.
 *
 */
tSymbolPtr htabGetSymbol(const tHashTablePtr table, dtStrPtr name);

/**
 *  \brief Executes function for each symbol in the hash table.
 *  
 *  \param [in] table tHashTablePtr, Pointer to the hashtable.
 *  \param [in] func tSymbolPtr(*)(tSymbolPtr), Pointer to a function to be executed for each symbol in the hash table.
 *  \param param [in] void*, param to be passed as second func param
 *
 *  \return bool, Returns true if for each symbol in the table, function func returns non NULL
 *
 */
bool htabForEach(tHashTablePtr table, tSymbolPtr (*func)(tSymbolPtr, void *), void *param);

/**
 *  \brief Removes symbol of the name specified from the hash table.
 *
 *  \param [in] table tHashTablePtr, Pointer to the hashtable.
 *  \param [in] name dtStrPtr, Pointer to a string structure representing name of the symbol to be removed.
 *
 */
void htabRemoveSymbol(tHashTablePtr table, dtStrPtr name);

/**
 *  \brief Removes all symbols from the hash table.
 *
 *  \param [in] table tHashTablePtr, Pointer to the hashtable.
 *
 */
void htabClear(tHashTablePtr table);

/**
 *  \brief Deallocates the hash table.
 *
 *  \param [in] table tHashTablePtr, Pointer to the hashtable.
 *
 */
void htabFree(tHashTablePtr table);

/**
 *  \brief Allocates a new Symbol.
 *
 *  \return tSymbolPtr, Returnes newly allocated symbol. If an error occures, NULL is returned.
 *
 *  \details Allocates a new symbol on heap and returnes pointer to it. Symbol is also initialized to default.
 */
tSymbolPtr symbolNew(void);


/**
 *  \brief Allocates a new symbol identical to the symbol specified.
 *
 *  \return tSymbolPtr, Returnes newly allocated and copied symbol. If an error occures, NULL is returned.
 */
tSymbolPtr symbolNewCopy(const tSymbolPtr symbol);

/**
 * @brief Registers symbol as function symbol.
 * @param symbolFunc, tSymbolPtr Pointer to the function symbol
 * @param argument, tSymbolPtr Pointer to the symbol to be registered as argument
 * @return tSymbolPtr, Returns pointer to the argument on success, else NULL is returned.
 */
tSymbolPtr symbolFuncAddArgument(tSymbolPtr symbolFunc, const tSymbolPtr argument);


/**
*  \brief Deallocates a new Symbol.
*
*	\param [in] tSymbolPtr, Pointer to the symbol to be deallocated
*/
void symbolFree(tSymbolPtr symbol);

//-------------------------------------------------------------------
//-----------------------------QuickSort-----------------------------
//-------------------------------------------------------------------

/**
 * 	Sorts characters in string s. Character with lower ordinal value
 *  is always preceded by a character with a higher ordinal value.
 *
 * @param      s     unsorted dtStr
 *
 * @return     New sorted dtStr, or NULL if something went wrong.
 */
dtStr *sort(dtStr *s);

//-------------------------------------------------------------------
//-------------------------------Find--------------------------------
//-------------------------------------------------------------------

/**
 * Searches for the first appearance of substring 'search' in string 's'.
 *
 * @param      s     	dtStr, string in which we want to find substring 'search'
 * @param      search 	dtStr, substring to be found in 's'
 *
 * @return     Starting index of found substring in 's' or -1 when 's' does not contain substring 'search'.
 */
int32_t find(dtStr* s, dtStr* search);

#endif
