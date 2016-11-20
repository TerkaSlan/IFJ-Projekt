#ifndef ERROR_H
#define ERROR_H
/**
 *  \brief Typedef enum of possible errors
 */
typedef enum {
	ERR_OK        = 0,
	ERR_LEX,
	ERR_SYNTAX,
	ERR_SEM,
	ERR_SEM_TYPE,
	ERR_SEM_OTHER = 6,
	ERR_RUN_INPUT,
	ERR_RUN_UNINITIALIZED,
	ERR_RUN_ZERODIV,
	ERR_OTHER,
	ERR_INTERN    = 99,
} eError;

/**
 *  \brief Prints to stderr error message with appropriate error code representation and a custom formated string specified.
 *  
 *  \param [in] errType eError, Error code that occured.
 *  \param [in] fmt const char*, Custom formatted string to be printed
 *  \param [in] ... other arguments matching the formating string
 *  
 */
void printError(eError errType, const char *fmt, ...);

#endif