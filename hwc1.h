#ifndef HWC1_H_
#define HWC1_H_
#define BUFFER_ERROR (msg_t *) NULL
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct msg {

    	void* content; // generico contenuto del messaggio

    	struct msg * (*msg_init)(void*); // creazione msg
    	void (*msg_destroy)(struct msg *); // deallocazione msg
    	struct msg * (*msg_copy)(struct msg *); // creazione/copia msg

} msg_t;

typedef struct buffer {
 	int size; //dimensione
  	int indP; //indice per i produttori
   	int indC; //indice per i consumatori
	int numM; //numero messaggi nel buffer
    	msg_t* message; //contenuto del blocco

    	pthread_mutex_t mutexP; //Mutex produttori
    	pthread_mutex_t mutexC; //Mutex consumatori
    	pthread_cond_t notEmpty; //Buffer non vuoto
    	pthread_cond_t notFull; //Buffer non pieno

    	struct buffer* (*buffer_init)(unsigned int); //creazione buffer
    	void (*buffer_destroy)(struct buffer*); //deallocazione buffer

} buffer_t;

typedef struct args {
    	buffer_t* buffer;
    	msg_t* msg;
} arg_t;

msg_t* msg_init_string(void* content);

void msg_destroy_string(msg_t* msg);

msg_t* msg_copy_string(msg_t* msg);

/* allocazione / deallocazione buffer */
// creazione di un buffer vuoto di dim. max nota
buffer_t* buffer_init(unsigned int maxsize);

// deallocazione di un buffer
void buffer_destroy(buffer_t* buffer);

/* operazioni sul buffer */
// inserimento bloccante: sospende se pieno, quindi
// effettua l’inserimento non appena si libera dello spazio
// restituisce il messaggio inserito; N.B.: msg!=null
msg_t* put_bloccante(buffer_t* buffer, msg_t* msg);
void* args_put_bloccante(void* arguments);

// inserimento non bloccante: restituisce BUFFER_ERROR se pieno,
// altrimenti effettua l’inserimento e restituisce il messaggio
// inserito; N.B.: msg!=null
msg_t* put_non_bloccante(buffer_t* buffer, msg_t* msg);
void* args_put_non_bloccante(void* arguments);

// estrazione bloccante: sospende se vuoto, quindi
// restituisce il valore estratto non appena disponibile
msg_t* get_bloccante(buffer_t* buffer);
void* args_get_bloccante(void* buffer);

// estrazione non bloccante: restituisce BUFFER_ERROR se vuoto
// ed il valore estratto in caso contrario
msg_t* get_non_bloccante(buffer_t* buffer);
void* args_get_non_bloccante(void* buffer);

#endif
