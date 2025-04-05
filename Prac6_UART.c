#include <avr/io.h>
//#include <stdlib.h>
//#include "UART.h"

#define FOSC 16000000  //Frecuencia del reloj
#define YELLOW  33 // Fixme
#define GREEN   32 // Fixme
#define BLUE    34 // Fixme



//Función que coloca el dato a enviar por el periférico.
void UART_putchar(uint8_t com, char data) {
	switch (com) {
		case 0:
		while (!(UCSR0A & (1 << UDRE0)));  //Esperar hasta que el buffer de transmisión esté vacío
		UDR0 = data;                       //Enviar el dato
		break;
		case 1:
		while (!(UCSR1A & (1 << UDRE1)));
		UDR1 = data;
		break;
		case 2:
		while (!(UCSR2A & (1 << UDRE2)));
		UDR2 = data;
		break;
		case 3:
		while (!(UCSR3A & (1 << UDRE3)));
		UDR3 = data;
		break;
	}
}
//Función que retorna 1 si existe dato disponible en el periférico.
uint8_t UART_available(uint8_t com) {
	switch (com) {
		case 0:
		return (UCSR0A & (1 << RXC0)) ? 1 : 0;   //Verifica si hay datos disponibles
		case 1:
		return (UCSR1A & (1 << RXC1)) ? 1 : 0;
		case 2:
		return (UCSR2A & (1 << RXC2)) ? 1 : 0;
		case 3:
		return (UCSR3A & (1 << RXC3)) ? 1 : 0;
		default:
		return 0;
	}
}
//Función que retorna el dato recibido por el periférico. Si no existe, entonces espera hasta recibir uno
char UART_getchar(uint8_t com) {
	switch (com) {
		case 0:
		while (!(UCSR0A & (1 << RXC0)));  //Esperar hasta que haya datos disponibles
		return UDR0;                      //Leer el dato recibido
		case 1:
		while (!(UCSR1A & (1 << RXC1)));
		return UDR1;
		case 2:
		while (!(UCSR2A & (1 << RXC2)));
		return UDR2;
		case 3:
		while (!(UCSR3A & (1 << RXC3)));
		return UDR3;
		default:
		return 0;  //Retorna 0 si el UART no es válido
	}
}
//Función que retorna una cadena haciendo uso de UART_getchar(uint8_t com), la
//cadena se retorna en el apuntador str.

void UART_gets(uint8_t com, char *str) {
	char c;
	uint8_t index = 0;      //Índice para la cadena
	//char lim[20];
	
	while (1) {
		c = UART_getchar(com);

		//Al presionar enter muestra nueva linea (fin de entrada)
		if (c == '\n' || c == '\r'){
			UART_putchar(com, '\n');
			break;
		}

		if (c == 8 || c == 127) {    //8 = Backspace, 127 = DEL
			if (index > 0) {
				index--;                          //Retrocede un índice
				UART_puts(com, "\033[D\033[K");
			}
		}
		// Si es un carácter imprimible
		else if (index < 20) {
			str[index++] = c;           //Agrega carácter a la cadena
			UART_putchar(com, c);
		}
	}
	str[index] = '\0';                 
}


//Función que imprime una cadena mediante UART_putchar(uint8_t com).
void UART_puts(uint8_t com, char *str) {
	while (*str) {                   //itera hasta encontrar el carácter nulo '\0'
		UART_putchar(com, *str++);
	}
}

//Función que convierte una numero de 16 bits a su representación ASCII en la base especificada.
void myitoa(uint16_t number, char *str, uint8_t base) {
	char buffer[17];                 //Buffer temporal para almacenar la cadena (máx. 16 bits)
	int i = 0;

	if (number == 0) {               //Si el número es 0, manejarlo como caso especial
		str[i++] = '0';
		str[i] = '\0';
		return;
	}

	// Conversión del número a la base especificada
	while (number > 0) {
		uint8_t digit = number % base;         //Obtiene el dígito menos significativo
		buffer[i++] = (digit < 10) ?           //Si es menor que 10, usa '0' a '9'
		(digit + '0') :
		(digit - 10 + 'A');     //Si es >= 10, usa 'A' a 'F'
		number /= base;
	}

	// Invierte la cadena
	buffer[i] = '\0';
	int j = 0;
	i--;
	while (i >= 0) {
		str[j++] = buffer[i--];
	}
	str[j] = '\0';
}


//Función que convierte una cadena de un valor decimal a un numero entero de 16 bits.
uint16_t atoi(char *str) {
	uint16_t result = 0;                      //Almacena el valor final
	uint8_t sign = 0;                         //manejar valores negativos

	
	if (*str == '-') {
		sign = 1;
		str++;
	}

	while (*str) {
		if (*str >= '0' && *str <= '9') {     // Si es un dígito
			result = (result * 10) + (*str - '0');
			} else {                               //Si encuentra un carácter no válido, termina
			break;
		}
		str++;
	}

	return sign ? -result : result;           //Devuelve el valor positivo o negativo
}

void UART_Init(uint8_t com, uint32_t baudrate, uint8_t size, uint8_t parity, uint8_t stop) {
	//Variables para calcular UBRR
	uint32_t ubrr16, baudReal16, errorReal16;
	uint32_t ubrr8, baudReal8, errorReal8;
	
	//Cálculo para modo normal (16X)
	ubrr16 = (FOSC / (16 * baudrate)) - 1;
	baudReal16 = (FOSC * 100) / (16 * (ubrr16 + 1));
	errorReal16 = (baudReal16 > (baudrate * 100)) ? (baudReal16 - (baudrate * 100)) : ((baudrate * 100) - baudReal16);

	//Cálculo para modo doble velocidad (8X)
	ubrr8 = (FOSC / (8 * baudrate)) - 1;
	baudReal8 = (FOSC * 100) / (8 * (ubrr8 + 1));
	errorReal8 = (baudReal8 > (baudrate * 100)) ? (baudReal8 - (baudrate * 100)) : ((baudrate * 100) - baudReal8);

	//Punteros a los registros UART
	volatile uint8_t *ucsra, *ucsrb, *ucsrc;
	volatile uint16_t *ubrr;

	switch (com) {
		case 0:
		ucsra = &UCSR0A;
		ucsrb = &UCSR0B;
		ucsrc = &UCSR0C;
		ubrr  = &UBRR0;
		break;
		case 1:
		ucsra = &UCSR1A;
		ucsrb = &UCSR1B;
		ucsrc = &UCSR1C;
		ubrr  = &UBRR1;
		break;
		case 2:
		ucsra = &UCSR2A;
		ucsrb = &UCSR2B;
		ucsrc = &UCSR2C;
		ubrr  = &UBRR2;
		break;
		case 3:
		ucsra = &UCSR3A;
		ucsrb = &UCSR3B;
		ucsrc = &UCSR3C;
		ubrr  = &UBRR3;
		break;
		default:
		return;  // UART inválido
	}

	// Selección del modo más preciso
	if (errorReal16 < errorReal8) {
		*ubrr = ubrr16;
		*ucsra &= ~(1 << U2X0);     //Modo normal (16X)
		} else {
		*ubrr = ubrr8;
		*ucsra |= (1 << U2X0);      //Modo doble velocidad (8X)
	}

	// Configuración del número de bits
	uint8_t bits_mask = 0;
	switch (size) {
		case 5: bits_mask = 0; break;
		case 6: bits_mask = (1 << UCSZ00); break;
		case 7: bits_mask = (2 << UCSZ00); break;
		case 8: bits_mask = (3 << UCSZ00); break;
		case 9: bits_mask = (3 << UCSZ00);
		*ucsrb |= (1 << UCSZ02);  //Habilitar 9 bits
		break;
		default: bits_mask = (3 << UCSZ00);
	}

	// Configuración de la paridad
	uint8_t parity_mask = 0;
	switch (parity) {
		case 0: parity_mask = 0; break;                  //Sin paridad
		case 1: parity_mask = (3 << UPM00); break;       //Impar
		case 2: parity_mask = (2 << UPM00); break;       //Par
	}

	// Configuración de los bits de parada
	uint8_t stop_mask = (stop == 2) ? (1 << USBS0) : 0;

	// Aplicar configuración
	*ucsrb = (1 << TXEN0) | (1 << RXEN0);       //Habilitar TX y RX
	*ucsrc = bits_mask | parity_mask | stop_mask;
}

//Funcion para limpiar la terminal
void UART_clrscr(uint8_t com) {
	UART_puts(com, "\033[2J");    //Limpia toda la pantalla
	UART_puts(com, "\033[H");    //mueve el cursor
}

//Establecer el color de la terminal
void UART_setColor(uint8_t com, uint8_t color) {
	char buffer[10];
	sprintf(buffer, "\033[%dm", color);
	UART_puts(com, buffer);
}

//posiciona el cursor en la terminal en la coordenada x,y que lleguen como
//parámetro, utilizando la secuencia de escape.
void UART_gotoxy(uint8_t com, uint8_t x, uint8_t y) {
	char buffer[20];
	sprintf(buffer, "\033[%d;%dH", y, x);
	UART_puts(com, buffer);               //Envía la secuencia a la terminal
}

int main( void )
{
	char cad[20];
	char cadUart3[20];
	uint16_t num;

	UART_Init(0,12345,8,1,2);
	UART_Init(2,115200,8,0,1);
	UART_Init(3,115200,8,0,1);
	while(1)
	{
		UART_getchar(0);
		UART_clrscr(0);

		UART_gotoxy(0,2,2);
		UART_setColor(0,YELLOW);
		UART_puts(0,"Introduce un numero:");
		
		UART_gotoxy(0,22,2);
		UART_setColor(0,GREEN);
		UART_gets(0,cad);
		// -------------------------------------------
		// Cycle through UART2->UART3
		UART_puts(2,cad);
		UART_puts(2,"\r");
		UART_gets(3,cadUart3);
		UART_gotoxy(0,5,3);
		UART_puts(0,cadUart3);
		// -------------------------------------------
		num = atoi(cad);
		myitoa(num,cad,16);
		
		UART_gotoxy(0,5,4);
		UART_setColor(0,BLUE);
		UART_puts(0,"Hex: ");
		UART_puts(0,cad);
		myitoa(num,cad,2);
		
		UART_gotoxy(0,5,5);
		UART_puts(0,"Bin: ");
		UART_puts(0,cad);
	}
}