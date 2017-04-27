/*
 *	Embedded version of "printf" function
 *
 *	Based in this link:
 *	http://e2e.ti.com/support/development_tools/code_composer_studio/f/81/p/30479/107146
 *
 *	Updated by: Haroldo Amaral - agaelema@globo.com
 *	Include v 0.4
 *	11/04/2016
 */

#ifndef		__EMBEDDED_PRINTF_H
#define		__EMBEDDED_PRINTF_H
#if OS_CONSOLE_CONFIG_VALID > 0


#define		ENABLE_PAD								// enable padding in functions
#ifdef	ENABLE_PAD
#define	ENABLE_PAD_
#endif

#define		BINARY			2						// number identificator
#define		DECIMAL			10
#define		HEXADEC			16

#define		SIGNED 			1						// if signed (+/-)
#define		NON_SIGNED		0						// just (+)
#define		NON_PAD			0						// withou padding
#define		UPPER_CASE		'A'
#define		LOWER_CASE		'a'

#define		SCI				1						// scientific notation "xx.yyezz"
#define		NON_SCI			0						// standard notation "xx.yy"

#define 	PAD_RIGHT		0x01					// right justified
#define 	PAD_ZERO		0x02					// padding with zero "0"

#define		PRINT_BUF_LEN 	128						// size of internl buffer to the conversion

/*
 * Embedded version of the "printf()" function - use the same parameters
 * 		"u" (unsigned long), "d" (signed long), "x/X" (hexadecimal)
 * 		"b" (binary), "f" (float), "e" (float in scientific notation)
 * Limitations:		integer numbers need be casted with (long) and float/double with (float - standard)
 * 						or (double - precision_float)
 * 					integer numbers limited by 32bits signed size (long)
 * 					float variables limited to 4 decimal places in standard (reduce size and performance)
 * 					float variables limited to 9 decimal places in precision (reduce size and performance)
 * Input:		char *format			(like standard "printf( )")
 * return:		int return_value		(number of characters printed - like standard printf)
 */
unsigned int embedded_printf(char *format, ...);
#define uLipePrintk	 embedded_printf

#endif
#endif
