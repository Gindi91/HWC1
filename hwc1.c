#include "hwc1.h"
#include <pthread.h>

msg_t * msg_init_string(void* content){
	msg_t * msg=(msg_t*)malloc(sizeof(msg_t));
    	char* string=(char*)content;
    	char* new_content=(char*)malloc(strlen(string)+1); //+1 per lo /0 finale
    	strcpy(new_content, string);

    	msg->content=new_content;
    	msg->msg_init=msg_init_string;
    	msg->msg_copy=msg_copy_string;
    	msg->msg_destroy=msg_destroy_string;
    	return msg;
}

void msg_destroy_string(msg_t* msg) {
   	free(msg->content);
    	free(msg);
}

msg_t * msg_copy_string(msg_t* msg) {
    	return msg->msg_init(msg->content);
}

/* allocazione / deallocazione  buffer */
// creazione di un buffer vuoto di dim. max nota
buffer_t* buffer_init(unsigned int maxsize){
	buffer_t* buffer=(buffer_t*)malloc(sizeof(buffer_t));
	buffer->message=(msg_t*)calloc(maxsize, sizeof(msg_t));

	buffer->indP=0;
	buffer->indC=0;
	buffer->size=maxsize;
	buffer->numM=0;

	buffer->buffer_init=buffer_init;
	buffer->buffer_destroy=buffer_destroy;

	pthread_mutex_init(&(buffer->mutexP), NULL);
	pthread_mutex_init(&(buffer->mutexC), NULL);
	pthread_cond_init(&(buffer->notFull), NULL);
	pthread_cond_init(&(buffer->notEmpty), NULL);

return buffer;
}

// deallocazione di un buffer
void buffer_destroy(buffer_t* buffer){
	pthread_mutex_destroy(&(buffer->mutexP));
	pthread_mutex_destroy(&(buffer->mutexC));
	pthread_cond_destroy(&(buffer->notFull));
	pthread_cond_destroy(&(buffer->notEmpty));
	int i=0;

	while(i<buffer->size) {
		buffer->message[i].msg_destroy;
		i++;
	}

	free(buffer->message);
	free(buffer);
}

/* operazioni sul buffer */
// inserimento bloccante: sospende se pieno, quindi
// effettua l’inserimento non appena si libera dello spazio
// restituisce il messaggio inserito; N.B.: msg!=null
msg_t* put_bloccante(buffer_t* buffer, msg_t* msg){
	if(msg!=NULL){
		pthread_mutex_lock(&(buffer->mutexP));
		/*if(slotLiberi(buffer)==0){
			pthread_cond_wait(&(buffer->notFull),&(buffer->mutexP));
		}*/
		if(buffer->numM==buffer->size){
			pthread_cond_wait(&(buffer->notFull),&(buffer->mutexP));
		}
		int index=buffer->indP;
		int size=buffer->size;
		buffer->message[index]=*msg;
		buffer->numM++;
		buffer->indP=(index+1)%size;
		pthread_cond_signal(&(buffer->notEmpty));
		pthread_mutex_unlock(&(buffer->mutexP));
		return msg;
			
	}
	return BUFFER_ERROR;
}

void* args_put_bloccante(void* arguments) {
    arg_t *args = arguments;
    msg_t* msg = put_bloccante(args->buffer, args->msg);
    pthread_exit(msg);
}

// inserimento non bloccante: restituisce BUFFER_ERROR se pieno,
// altrimenti effettua l’inserimento e restituisce il messaggio
// inserito; N.B.: msg!=null
msg_t* put_non_bloccante(buffer_t* buffer, msg_t* msg){
	if(msg!=NULL){
		pthread_mutex_lock(&(buffer->mutexP));

		if(buffer->numM==buffer->size){
			pthread_mutex_unlock(&(buffer->mutexP));
			return BUFFER_ERROR;
		}

		int index=buffer->indP;
		int size=buffer->size;
		buffer->message[index]=*msg;
		buffer->numM++;
		buffer->indP=(index+1)%size;
		pthread_cond_signal(&(buffer->notEmpty));
		pthread_mutex_unlock(&(buffer->mutexP));
		return msg;
			
	}
	return BUFFER_ERROR;
}

void* args_put_non_bloccante(void* arguments) {
    arg_t *args = arguments;
    msg_t* msg = put_bloccante(args->buffer, args->msg);
    pthread_exit(msg);
}

// estrazione bloccante: sospende se vuoto, quindi
// restituisce il valore estratto non appena disponibile
msg_t* get_bloccante(buffer_t* buffer){
	pthread_mutex_lock(&(buffer->mutexC));
	int size=buffer->size;
	printf("%d\n",size);
	printf("eccomi1\n");

	if(buffer->numM==0){
		pthread_cond_wait(&(buffer->notEmpty),&(buffer->mutexC));
		printf("eccomi ma è pieno\n");
	}

	int index=buffer->indC;
	msg_t* msg=(msg_t*)malloc(sizeof(msg_t));
	msg=&buffer->message[index];
	printf("eccomi che ho inserito\n");
	buffer->message[index].msg_destroy;
	buffer->numM--;
	buffer->indC=(index+1)%size;
	pthread_cond_signal(&(buffer->notFull));
	pthread_mutex_unlock(&(buffer->mutexC));
	return msg;
}

void* args_get_bloccante(void* buffer){
    msg_t* msg = get_bloccante((buffer_t*) buffer);
    pthread_exit(msg);
}

// estrazione non bloccante: restituisce BUFFER_ERROR se vuoto
// ed il valore estratto in caso contrario
msg_t* get_non_bloccante(buffer_t* buffer){
	pthread_mutex_lock(&(buffer->mutexC));
	int size=buffer->size;
	if(slotLiberi(buffer)==size){
		pthread_mutex_unlock(&(buffer->mutexC));
		return BUFFER_ERROR;
	}
	int index=buffer->indC;
	msg_t* msg=(msg_t*)malloc(sizeof(msg_t));
	msg=(msg_t*)buffer->message[index].msg_copy;
	buffer->message[index].msg_destroy;
	buffer->numM--;
	buffer->indC=(index+1)%size;
	pthread_cond_signal(&(buffer->notFull));
	pthread_mutex_unlock(&(buffer->mutexC));
	return msg;
}

void* args_get_non_bloccante(void* buffer){
    msg_t* msg = get_bloccante((buffer_t*) buffer);
    pthread_exit(msg);
}

/*int slotLiberi(buffer_t* buffer) {
	int c=0;
	int size=buffer->size;
	for (int i=0;i<size;i++){
		msg_t* msg=(msg_t*)buffer->message[i].msg_copy;
		if((msg)==NULL) c++;
	}
	return c;
}

int main(){
	return 0;
}*/
