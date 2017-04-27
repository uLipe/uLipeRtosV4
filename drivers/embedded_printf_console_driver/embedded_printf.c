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

#include    "uLipeRtos4.h"


#if OS_CONSOLE_CONFIG_VALID > 0


/*
 * Os resources
 */

static bool consoleReady = false;
static Device_t *consoleUart;
static Device_t *consolePmux;
static char print_buf[PRINT_BUF_LEN];

/*!
 *  ConsoleInit()
 *
 *  \brief inits the IO device used by console
 *
 */
static void console_init(void)
{
	OsStatus_t err;
	consoleUart = uLipeDeviceOpen(OS_CONSOLE_DRIVER, NULL);
	uLipeAssert(consoleUart != NULL);

	consolePmux = uLipeDeviceOpen(OS_CONSOLE_DRIVER_PINMUX_NAME, NULL);
	uLipeAssert(consolePmux != NULL);


	/* setup the uart pins PTA1:2 as UART0 */
	err = uLipeDriverPinMuxSetRowToAlt(consolePmux, OS_CONSOLE_DRIVER_PIN_OFFSET,
			OS_CONSOLE_DRIVER_PINS_LEN, OS_CONSOLE_ACCEPT_MASK, OS_CONSOLE_ALTERNATE);
	uLipeAssert(err == kStatusOk);

	/* sets the console as 115200 8N1 */
	err = uLipeDriverUartInit(consoleUart, 115200,0);
	uLipeAssert(err == kStatusOk);
	uLipeDriverUartEnable(consoleUart);
	uLipeAssert(err == kStatusOk);

}

/*
 * Call the specific function to send data
 * Input:	unsigned char	byte (byte to be transmited)
 * Output:	none
 */
static void put_char(uint8_t byte)
{
	uLipeDriverUartPollOut(consoleUart, byte);
}


/*
 * Print a String in the serial port
 * input:	char *string		(string to be printed)
 * 			unsigned int width	(limit the pad size)
 * 			unsigned int pad	(control the padding)
 * return:	number of characters
 */
//int embedded_prints(char *string, unsigned int width, unsigned int pad)
static unsigned int embedded_prints(char *string, unsigned int width, unsigned int pad)
{
	unsigned int return_value = 0;
	unsigned char padchar = ' ';

	if (width > 0)
	{
		unsigned int len = 0;						// string length
		char *ptr;

		for (ptr = string; *ptr; ++ptr) {			// Calculate string length
			++len;
		}

		if (len >= width) width = 0;    			// If string is longer than width
		else width -= len;

#ifdef	ENABLE_PAD_
		if (pad & PAD_ZERO) padchar = '0';
#endif
	}

#ifdef	ENABLE_PAD_
	if (!(pad & PAD_RIGHT))  	                   	// If not right padding - left justification
	{
		for (; width > 0; --width)					// if padding is possible - put the char
		{
			put_char(padchar);
			++return_value;
		}
	}
#endif

	while (*string)
	{
		put_char(*string);
		++return_value;
//		*++string;
		++string;
	}

#ifdef	ENABLE_PAD_
	for (; width > 0; --width) {
		put_char(padchar);
		++return_value;
	}
#endif

	return return_value;                        	// Return the number of characters printed
}


/*
 *	Print an integer number (signed or unsigned) with max size of 32bits (long)
 *	input:	char *print_buf			(pointer to the buffer when the string will be saved)
 *			signed long input		(number to be printed - use the casting "(long)" before the number
 *			unsigned int base		(base of number - decimal, hexa, binary)
 *			unsigned int sg			(1 = signed, 0 = unsigned)
 *			unsigned int width		(mas size of padding)
 *			unsigned int pad		(control the padding)
 *			unsigned char letbase	(base to select the character in the ASCII table)
 *	return:	int return_value		(number of characters printed - like standard printf)
 */
//int embedded_ltoa(char *print_buf, signed long input, unsigned int base, unsigned int sg, unsigned int width, unsigned int pad, unsigned char letbase)
static unsigned int embedded_ltoa(char *print_buf, int32_t input, unsigned int base, unsigned int sg, unsigned int width, unsigned int pad, unsigned char letbase)
{
	char *s;
	char neg = 0;
	uint32_t t;
	uint32_t u = input;
	unsigned int return_value = 0;

//	if (input == 0)
//	{
//		if (base != 2)
//		{
//			print_buf[0] = '0';
//			print_buf[1] = '\0';            // Always remenber to put string end
//			return 2;
////			print_buf[(2 * label) + 0] = '0';
////			print_buf[(2 * label) + 1] = '\0';            // Always remenber to put string end
////			if ((label) && (base == HEXADEC))
////			{
////				print_buf[0] = '0';
////				print_buf[1] = 'x';
////			}
////			return ((2 * label) + 2);
//		}
//		else
//		{
//			unsigned int xx;
//			for (xx = 0; xx < width; xx++)
//			{
//				print_buf[xx] = '0';
////				print_buf[(2 * label) + xx] = '0';
//			}
//			print_buf[width] = '\0';
////			print_buf[(2 * label) + width] = '\0';
////			if (label)
////			{
////				print_buf[0] = '0';
////				print_buf[1] = 'x';
////			}
//			return (width + 1);
//		}
//	}

	if (sg && (base == 10) && (input < 0))                 // If it is a negative number in decimal base
	{
		neg = 1;
		u = -input;
	}

	s = print_buf + PRINT_BUF_LEN - 1;				// go to the end of buffer
	*s = '\0';										// print the string terminator "\0"

	do
	{
		if (base != 2)								// if decimal or hexa
		{
			t = u % base;
			if (t >= 10)
				t += letbase - '0' - 10;
			*--s = t + '0';
			u /= base;
		}
#ifdef	ENABLE_BINARY_
		else
		{											// if binary

			while (width)
			{
				*--s = (u & 0x01) ? '1' : '0';
				u >>= 1;
				width--;
			}
			u = 0;
		}
#endif
	}while(u);

//	if (label) {									// place the label... not implemented (some bugs)
//		if (base != DECIMAL)
//		{
//			*--s = (base == HEXADEC ? 'x' : 'b');
//			*--s = '0';
//		}
//	}

	if (neg)										// if negative, put the "-" signal
	{

////	 	old version of code
//		if (width && (pad & PAD_ZERO))
//		{     // If there is width, right justified and pad with zero, output negative sign.
//			put_char('-');
//			++return_value;
//			--width;
//		}
//		else *--s = '-';                  // Otherwise put the '-' to string buffer.

		if (!(width && (pad & PAD_ZERO)))
		{
			*--s = '-';                  // Otherwise put the '-' to string buffer.
		}
#ifdef	ENABLE_PAD_
		else							// If there is width, right just. and pad with zero
		{
			put_char('-');
			++return_value;
			--width;
		}
#endif
	}

	char *buffer_end = print_buf + PRINT_BUF_LEN - 1;
	int count;;
	count = (int)buffer_end - (int)s;

	int xx;
	for (xx = 0; xx < count + 1; xx++) {
		print_buf[xx] = *s;
		s++;
	}

	return return_value;
}



unsigned int embedded_printf(char *format, ...)
{
	unsigned int width, pad;
	unsigned int return_value = 0;
	unsigned int dp = 0;


	if(consoleReady != true ){
		/* first time use, prepare the uart driver first */
		uint32_t sReg = 0;
		console_init();

		OS_CRITICAL_IN();
		consoleReady = true;
		OS_CRITICAL_OUT();
	}

	va_list args;
	va_start(args, format);

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;

			if (*format == '-') {
				++format;
#ifdef	ENABLE_PAD_
				pad = PAD_RIGHT;
#endif
			}
			while (*format == '0') {
				++format;
#ifdef	ENABLE_PAD_
				pad |= PAD_ZERO;
#endif
			}
			for (; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}

			if (*format == '.') {
				++format;
				for (; *format >= '0' && *format <= '9'; ++format) {
					dp *= 10;
					dp += *format - '0';
				}
			}

			if (*format == 's') {				// if string - call the respective function
				char *s = (char *)va_arg(args, int);
				return_value += embedded_prints(s ? s : "(null)", width, pad);
				continue;
			}
			if (*format == 'd') {				// if signed long - call the respective function
				return_value += embedded_ltoa(print_buf, va_arg(args, int32_t), DECIMAL, SIGNED, width, pad, LOWER_CASE);
				return_value += embedded_prints(print_buf, width, pad);
				continue;
			}
			if (*format == 'x') {				// if hexadecimal (lowercase) - call the respective function
				return_value += embedded_ltoa(print_buf, va_arg(args, int32_t), HEXADEC, NON_SIGNED, width, pad, LOWER_CASE);
				return_value += embedded_prints(print_buf, width, pad);
				continue;
			}
			if (*format == 'X') {				// if hexadecimal (uppercase) - call the respective function
				return_value += embedded_ltoa(print_buf, va_arg(args, int32_t), HEXADEC, NON_SIGNED, width, pad, UPPER_CASE);
				return_value += embedded_prints(print_buf, width, pad);
				continue;
			}
			if (*format == 'u') {				// if unsigned long - call the respective function
				return_value += embedded_ltoa(print_buf, va_arg(args, int32_t), DECIMAL, NON_SIGNED, width, pad, LOWER_CASE);
				return_value += embedded_prints(print_buf, width, pad);
				continue;
			}
			if (*format == 'c') {				// if a char - direct put in the serial
				char scr[2];
				scr[0] = (char)va_arg(args, int);
				scr[1] = '\0';
				return_value += embedded_prints(scr, width, pad);
				continue;
			}
#ifdef	ENABLE_FLOAT_
			if (*format == 'f') {				// if float/double - call the respective function
#ifdef PRECISION_FLOAT_
				return_value += embedded_ftoa(print_buf, va_arg(args, double), dp, NON_SCI);
#else
				return_value += embedded_ftoa(print_buf, (float)va_arg(args, double), dp, NON_SCI);
#endif
				return_value += embedded_prints(print_buf, 0, 0);
				continue;
			}
			if (*format == 'e') {				// if scientific notation - call the respective function
#ifdef PRECISION_FLOAT_
				return_value += embedded_ftoa(print_buf, va_arg(args, double), dp, SCI);
#else
				return_value += embedded_ftoa(print_buf, (float)va_arg(args, double), dp, SCI);
#endif
				return_value += embedded_prints(print_buf, 0, 0);
				continue;
			}
#endif
#ifdef	ENABLE_BINARY_
			if (*format == 'b') {				// if binary - call the respective function
				return_value += embedded_ltoa(print_buf, va_arg(args, int32_t), BINARY, NON_SIGNED, width, pad, LOWER_CASE);
				return_value += embedded_prints(print_buf, width, 0);
				continue;
			}
#endif
		}
		else {
		out:
			put_char(*format);
			++return_value;
		}
	}
	va_end(args);
	return return_value;
}

#endif
