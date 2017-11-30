#include "hwc1.h"
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include <pthread.h>

buffer_t* buffer;
buffer_t* bufferU;
msg_t* msg;
//=msg_init_string("a");

//suite prova
int init_createBuffer(void){
	buffer=buffer_init(3);
	if(buffer!=NULL) return 0;
	else return 1;
}

int init_createBuffer_Unitary(void){
	bufferU=buffer_init(1);
	if(bufferU!=NULL) return 0;
	else return 1;
}

int clean_destroyBuffer(void){
	buffer_destroy(buffer);
	if(buffer!=NULL) return 0;
	else return 1;
}

int clean_destroyBuffer_Unitary(void){
	buffer_destroy(bufferU);
	if(bufferU!=NULL) return 0;
	else return 1;
}
/*==============================================*/
void test_notNull(void){
	CU_ASSERT(buffer!=NULL);
	CU_ASSERT(buffer->size==3);
}

void test_empty(void){
	if(buffer!=NULL){
		CU_ASSERT(buffer->numM==0);
	}
}

/*void test_empty(void){
	if(buffer!=NULL){
		CU_ASSERT(slotLiberi(buffer)==buffer->size);
	}
}*/

void test_notEmpty(void){
	if(buffer!=NULL){
		CU_ASSERT(buffer->numM>0);
	}
}

void test_single_put(void){
	msg_t* msg = msg_init_string("a");
	msg_t* expected;

	arg_t args;
	args.buffer=buffer;
	args.msg=msg;

	if(buffer!=NULL) {
		pthread_t thread;
		int size=buffer->size;	

		CU_ASSERT(buffer->numM==0);

		pthread_create(&thread, NULL, args_put_bloccante, &args);
		pthread_join(thread, (void*)&expected);

		CU_ASSERT_EQUAL(msg, expected);
		CU_ASSERT(buffer->numM==1);
	}
}

void test_single_get(void){
	pthread_t thread;
	CU_ASSERT(buffer->numM==1);
	pthread_create(&thread, NULL, args_get_bloccante, buffer);
	pthread_join(thread, (void*)&msg);
	CU_ASSERT(buffer->numM==0);
	
}
/*==============================================*/
/*Test di HW1*/

/*========(P=1, C=0, N=1)========*/
/*Produzione di un solo messaggio in un buffer vuoto*/
void test_single_put_unitary(void){
	msg_t* msg = msg_init_string("a");
	msg_t* expected;

	arg_t args;
	args.buffer=bufferU;
	args.msg=msg;

	if(bufferU!=NULL) {
		pthread_t thread;
		int size=bufferU->size;	

		CU_ASSERT(bufferU->numM==0);

		pthread_create(&thread, NULL, args_put_bloccante, &args);
		pthread_join(thread, (void*)&expected);

		CU_ASSERT_EQUAL(msg, expected);
		CU_ASSERT(bufferU->numM==1);
	}
}

/*========(P=0, C=1, N=1)========*/
/*Consumazione  di  un  solo  messaggio  da  un  buffer pieno*/
void test_single_get_unitary(void){
	pthread_t thread;
	CU_ASSERT(bufferU->numM==1);
	pthread_create(&thread, NULL, args_get_bloccante, bufferU);
	pthread_join(thread, (void*)&msg);
	CU_ASSERT(bufferU->numM==0);
	
}

/*========(P=1, C=0, N=1)========*/
/*Produzione in un buffer pieno*/
void test_single_put_unitary_full(void){

	msg_t* msg = msg_init_string("a");
	msg_t* expected;

	arg_t args;
	args.buffer=bufferU;
	args.msg=msg;

	if(bufferU!=NULL) {
		pthread_t thread;

		CU_ASSERT(bufferU->numM==1);

		pthread_create(&thread, NULL, args_put_non_bloccante, &args);
		pthread_join(thread, (void*)&expected);

		CU_ASSERT_EQUAL(BUFFER_ERROR, expected);
		CU_ASSERT(bufferU->numM==1);
	}
}

/*========(P=0, C=1, N=0)========*/
/*Consumazione  di  un  solo  messaggio  da  un  buffer vuoto*/
void test_single_get_unitary_empty(void){
	pthread_t thread;
	CU_ASSERT(bufferU->numM==0);
	pthread_create(&thread, NULL, args_get_non_bloccante, bufferU);
	pthread_join(thread, (void*)&msg);
	CU_ASSERT_EQUAL(BUFFER_ERROR, msg);
	CU_ASSERT(bufferU->numM==0);
	
}

/*========(P=1,C=1,N=1)========*/
/*Consumazione  e  produzione  concorrente  di  un  messaggio da un buffer unitario; prima il consumatore*/
void test_single_put_get_C(void){
	msg_t* msg = msg_init_string("a");
	msg_t* expectedP;
	msg_t* expectedG;

	arg_t args;
	args.buffer=bufferU;
	args.msg=msg;

	if(bufferU!=NULL) {
		pthread_t prod;
		pthread_t cons;
		int size=bufferU->size;	

		pthread_create(&cons, NULL, args_get_bloccante, bufferU);
		pthread_create(&prod, NULL, args_put_non_bloccante, &args);

		pthread_join(cons, (void*)&expectedG);
		pthread_join(prod, (void*)&expectedP);

		CU_ASSERT(bufferU->numM==0); //Verifico che è vuoto
		CU_ASSERT(0==strcmp(expectedG->content, expectedP->content)); //Verifico d'aver consumato e prodotto lo stesso messaggio
	}
}

/*========(P=1,C=1,N=1)========*/
/*Consumazione  e  produzione  concorrente  di  un  messaggio da un buffer unitario; prima il produttore*/
void test_single_put_get_P(void){
	msg_t* msg = msg_init_string("a");
	msg_t* expectedP;
	msg_t* expectedG;

	arg_t args;
	args.buffer = bufferU;
	args.msg = msg;

	if(bufferU != NULL) {
		pthread_t prod;
		pthread_t cons;

		CU_ASSERT(bufferU->numM==0); //Verifico che è vuoto
		pthread_create(&prod, NULL, args_put_non_bloccante, &args);
		pthread_join(prod, (void*)&expectedP);
		CU_ASSERT(bufferU->numM==1); //Verifico che è pieno

		pthread_create(&cons, NULL, args_get_bloccante, bufferU);
		pthread_join(cons, (void*)&expectedG);
		CU_ASSERT(bufferU->numM==0); //Verifico che è vuoto
		CU_ASSERT(0==strcmp(expectedG->content, expectedP->content)); //Verifico d'aver consumato e prodotto lo stesso messaggio
	}
}
/*==============================================*/
int main(){

	CU_pSuite primi=NULL;
	CU_pSuite concorrentiU=NULL;
	
	if (CUE_SUCCESS != CU_initialize_registry())
      		return CU_get_error();
	
	primi=CU_add_suite("Produzione e consumazione singola", init_createBuffer_Unitary, clean_destroyBuffer_Unitary);
	if(primi==NULL){
		CU_cleanup_registry();
		return CU_get_error();
	}

	concorrentiU=CU_add_suite("Produzioni e consumazioni concorrenti su buffer unitario", init_createBuffer_Unitary, clean_destroyBuffer_Unitary);
	if(concorrentiU==NULL){
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	//adding tests to primi
	if((CU_add_test(primi,"(P=1, C=0, N=1):Produzione di un solo messaggio in un buffer vuoto:", test_single_put_unitary)==NULL)
||(CU_add_test(primi,"(P=1, C=0, N=1):Produzione in un buffer pieno:", test_single_put_unitary_full)==NULL)
||(CU_add_test(primi,"(P=0, C=1, N=1):Consumazione  di  un  solo  messaggio  da  un  buffer pieno:", test_single_get_unitary)==NULL)
||(CU_add_test(primi,"(P=0, C=1, N=0):Consumazione  di  un  solo  messaggio  da  un  buffer vuoto:", test_single_get_unitary_empty)==NULL)){
      		CU_cleanup_registry();
      		return CU_get_error();		
	}

	//adding tests to concurrentU
	if((CU_add_test(concorrentiU,"(P=1, C=1, N=1):Consumazione  e  produzione  concorrente  di  un  messaggio da un buffer unitario; prima il consumatore:", test_single_put_get_C)==NULL)
||(CU_add_test(concorrentiU,"(P=1, C=1, N=1):Consumazione  e  produzione  concorrente  di  un  messaggio da un buffer unitario; prima il produttore:", test_single_put_get_P)==NULL)){
      		CU_cleanup_registry();
      		return CU_get_error();		
	}


	// Run all tests using the CUnit Basic interface
   	CU_basic_set_mode(CU_BRM_VERBOSE);
   	CU_basic_run_tests();
   	CU_cleanup_registry();
   	return CU_get_error();
}
