#include "hwc1.h"
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include <pthread.h>

buffer_t* buffer_u;
msg_t* msg;

//Creazione e distruzione buffer unitario
int init_createBuffer_Unitary(void){
	buffer_u=buffer_init(1);
	if(buffer_u!=NULL) return 0;
	else return 1;
}

int clean_destroyBuffer_Unitary(void){
	buffer_destroy(buffer_u);
	if(buffer_u!=NULL) return 0;
	else return 1;
}
/*==============================================*/
/*Test*/

/*========(P=1, C=0, N=1)========*/
/*Produzione di un solo messaggio in un buffer vuoto*/
void test_single_put_unitary(void){
	msg_t* msg=msg_init_string("a");
	msg_t* expected;

	arg_t args;
	args.buffer=buffer_u;
	args.msg=msg;

	if(buffer_u!=NULL) {
		pthread_t thread;

		CU_ASSERT(buffer_u->numM==0);//Verifico che il buffer non sia pieno

		pthread_create(&thread, NULL, args_put_bloccante, &args);
		pthread_join(thread, (void*)&expected);

		CU_ASSERT_EQUAL(msg, expected);//Verifica del messaggio prodotto
		CU_ASSERT(buffer_u->numM==1);//Verifico che il messaggio prodotto sia nel buffer
	}
}

/*========(P=0, C=1, N=1)========*/
/*Consumazione  di  un  solo  messaggio  da  un  buffer pieno*/
void test_single_get_unitary(void){
	if(buffer_u!=NULL){
		pthread_t thread;

		CU_ASSERT(buffer_u->numM==1);//Verifico la presenza di un messaggio nel buffer

		pthread_create(&thread, NULL, args_get_bloccante, buffer_u);
		pthread_join(thread, (void*)&msg);

		CU_ASSERT(buffer_u->numM==0);//Verifico che il messaggio sia stato consumato
	}
}

/*========(P=1, C=0, N=1)========*/
/*Produzione in un buffer pieno*/
void test_single_put_unitary_full(void){

	msg_t* msg=msg_init_string("a");
	msg_t* expected;

	arg_t args;
	args.buffer=buffer_u;
	args.msg=msg;

	if(buffer_u!=NULL) {
		pthread_t thread;

		CU_ASSERT(buffer_u->numM==1);//Verifico ch il buffer sia inizialmente pieno

		pthread_create(&thread, NULL, args_put_non_bloccante, &args);
		pthread_join(thread, (void*)&expected);

		CU_ASSERT_EQUAL(BUFFER_ERROR, expected);//Verifica che il processo torni un errore
		CU_ASSERT(buffer_u->numM==1);//Verifico ch il buffer sia ancora pieno
	}
}

/*========(P=0, C=1, N=0)========*/
/*Consumazione  di  un  solo  messaggio  da  un  buffer vuoto*/
void test_single_get_unitary_empty(void){
	pthread_t thread;

	CU_ASSERT(buffer_u->numM==0); //Verifico che il buffer sia inizialmente vuoto

	pthread_create(&thread, NULL, args_get_non_bloccante, buffer_u);
	pthread_join(thread, (void*)&msg);

	CU_ASSERT_EQUAL(BUFFER_ERROR, msg); //Verifica che il processo torni un errore
	CU_ASSERT(buffer_u->numM==0); //Verifico ch il buffer sia ancora vuoto
	
}

/*========(P=1,C=1,N=1)========*/
/*Consumazione  e  produzione  concorrente  di  un  messaggio da un buffer unitario; prima il consumatore*/
void test_single_put_get_C(void){
	msg_t* msg=msg_init_string("a");
	msg_t* expectedP;
	msg_t* expectedG;

	arg_t args;
	args.buffer=buffer_u;
	args.msg=msg;

	if(buffer_u!=NULL) {
		pthread_t prod;
		pthread_t cons;
		int size=buffer_u->size;	

		pthread_create(&cons, NULL, args_get_bloccante, buffer_u);
		pthread_create(&prod, NULL, args_put_non_bloccante, &args);

		pthread_join(cons, (void*)&expectedG);
		pthread_join(prod, (void*)&expectedP);

		CU_ASSERT(buffer_u->numM==0); //Verifico che è vuoto
		CU_ASSERT(0==strcmp(expectedG->content, expectedP->content)); //Verifico d'aver consumato e prodotto lo stesso messaggio
	}
}

/*========(P=1,C=1,N=1)========*/
/*Consumazione  e  produzione  concorrente  di  un  messaggio da un buffer unitario; prima il produttore*/
void test_single_put_get_P(void){
	msg_t* msg=msg_init_string("a");
	msg_t* expectedP;
	msg_t* expectedG;

	arg_t args;
	args.buffer=buffer_u;
	args.msg=msg;

	if(buffer_u!=NULL){
		pthread_t prod;
		pthread_t cons;

		CU_ASSERT(buffer_u->numM==0); //Verifico che è vuoto
		pthread_create(&prod, NULL, args_put_non_bloccante, &args);
		pthread_create(&cons, NULL, args_get_bloccante, buffer_u);
		
		pthread_join(prod, (void*)&expectedP);
		pthread_join(cons, (void*)&expectedG);
		CU_ASSERT(buffer_u->numM==0); //Verifico che sia ancora vuoto
		CU_ASSERT(0==strcmp(expectedG->content, expectedP->content)); //Verifico d'aver consumato e prodotto lo stesso messaggio
	}
}

/*========(P>1,C=0,N=1)========*/
/*Produzione concorrente di molteplici messaggi in un buffer unitario vuoto*/
void test_multiple_prod_u(void){
	msg_t* msg1=msg_init_string("a");
	msg_t* msg2=msg_init_string("a");
	msg_t* expected_a;
	msg_t* expected_b;

	arg_t args1;
	arg_t args2;

	args1.buffer=buffer_u;
	args1.msg=msg1;

	args2.buffer=buffer_u;
	args2.msg=msg2;

	if(buffer_u!=NULL){
		pthread_t process_a;
		pthread_t process_b;
		
		CU_ASSERT(buffer_u->numM==0);//Verifico che il buffer sia vuoto
		pthread_create(&process_a, NULL, args_put_non_bloccante, &args1);
		pthread_create(&process_b, NULL, args_put_non_bloccante, &args2);

		pthread_join(process_a, (void*)&expected_a);
		pthread_join(process_b, (void*)&expected_b);

		CU_ASSERT(buffer_u->numM==1);//Verifico che il buffer sia pieno dopo le produzioni
		
		//Verifico che solo una produzione va a buon fine
		CU_ASSERT_EQUAL(msg2, expected_b);
		CU_ASSERT_EQUAL(BUFFER_ERROR, expected_a);
	}
}

/*========(P=0,C>1,N=1)========*/
/*Consumazione concorrente di molteplici messaggi in un buffer unitario pieno*/
void test_multiple_cons_u(void){
	msg_t* msg=msg_init_string("a");
	msg_t* expected_a;
	msg_t* expected_b;

	if(buffer_u!=NULL){
		pthread_t process_a;
		pthread_t process_b;
		
		CU_ASSERT(buffer_u->numM==1);//Verifico che il buffer sia pieno
		pthread_create(&process_a, NULL, args_get_non_bloccante, buffer_u);
		pthread_create(&process_b, NULL, args_get_non_bloccante, buffer_u);

		pthread_join(process_a, (void*)&expected_a);
		pthread_join(process_b, (void*)&expected_b);

		CU_ASSERT(buffer_u->numM==0);//Verifico che il buffer sia vuoto dopo le consumazioni
		
		//Verifico che solo una consumazione va a buon fine
		CU_ASSERT(0==strcmp(msg->content, expected_b->content));
		CU_ASSERT_EQUAL(BUFFER_ERROR, expected_a);
	}
}
/*==============================================*/
int main(){

	CU_pSuite primi=NULL;
	CU_pSuite concorrentiU=NULL;
	CU_pSuite multiple=NULL;
	
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

	multiple=CU_add_suite("Produzione e consumazione di multipli elementi nei vari casi su un buffer unitario", init_createBuffer_Unitary, clean_destroyBuffer_Unitary);
	if(multiple==NULL){
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	//Aggiunta test a primi
	if((CU_add_test(primi,"(P=1, C=0, N=1):Produzione di un solo messaggio in un buffer vuoto:", test_single_put_unitary)==NULL)
||(CU_add_test(primi,"(P=1, C=0, N=1):Produzione in un buffer pieno:", test_single_put_unitary_full)==NULL)
||(CU_add_test(primi,"(P=0, C=1, N=1):Consumazione  di  un  solo  messaggio  da  un  buffer pieno:", test_single_get_unitary)==NULL)
||(CU_add_test(primi,"(P=0, C=1, N=0):Consumazione  di  un  solo  messaggio  da  un  buffer vuoto:", test_single_get_unitary_empty)==NULL)){
      		CU_cleanup_registry();
      		return CU_get_error();		
	}
	
	//Aggiunta test a concurrentU
	if((CU_add_test(concorrentiU,"(P=1, C=1, N=1):Consumazione  e  produzione  concorrente  di  un  messaggio da un buffer unitario; prima il consumatore:", test_single_put_get_C)==NULL)
||(CU_add_test(concorrentiU,"(P=1, C=1, N=1):Consumazione  e  produzione  concorrente  di  un  messaggio da un buffer unitario; prima il produttore:", test_single_put_get_P)==NULL)){
      		CU_cleanup_registry();
      		return CU_get_error();		
	}
	
	//Aggiunta test a multiple
	if((CU_add_test(multiple,"(P>1, C=0, N=1):Produzione concorrente di molteplici messaggi in un buffer unitario vuoto:", test_multiple_prod_u)==NULL)
||(CU_add_test(multiple,"(P=0, C>1, N=1):Consumazione concorrente di molteplici messaggi in un buffer unitario pieno:", test_multiple_cons_u)==NULL)){
      		CU_cleanup_registry();
      		return CU_get_error();		
	}

   	CU_basic_set_mode(CU_BRM_VERBOSE);
   	CU_basic_run_tests();
   	CU_cleanup_registry();
   	return CU_get_error();
}
