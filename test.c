#include "hwc1.h"
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include <pthread.h>

buffer_t* buffer;
msg_t* msg;
//=msg_init_string("a");

//suite prova
int init_createBuffer(void){
	buffer=buffer_init(3);
	if(buffer!=NULL) return 0;
	else return 1;
}

int init_createBufferNotEmpty(void){
	buffer=buffer_init(3);
	msg=msg_init_string("a");
	msg=put_bloccante(buffer,msg);
	if(buffer!=NULL) return 0;
	else return 1;
}

int clean_destroyBuffer(void){
	buffer_destroy(buffer);
	if(buffer!=NULL) return 0;
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
int main(){

	CU_pSuite prova=NULL;
	
	if (CUE_SUCCESS != CU_initialize_registry())
      		return CU_get_error();
	
	prova=CU_add_suite("prova", init_createBuffer, clean_destroyBuffer);
	if(prova==NULL){
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	//adding tests
	if ((CU_add_test(prova, "non nullo", test_notNull)==NULL)||(CU_add_test(prova, "vuoto", test_empty)==NULL)||(CU_add_test(prova, "put singola", test_single_put)==NULL)||(CU_add_test(prova, "non più vuoto", test_notEmpty)==NULL)||(CU_add_test(prova, "get singola", test_single_get)==NULL)||(CU_add_test(prova, "di nuovo vuoto", test_empty)==NULL)){
      		CU_cleanup_registry();
      		return CU_get_error();
   	}



	// Run all tests using the CUnit Basic interface
   	CU_basic_set_mode(CU_BRM_VERBOSE);
   	CU_basic_run_tests();
   	CU_cleanup_registry();
   	return CU_get_error();

	/*buffer=buffer_init(3);
	int size=buffer->size;
	printf("%d\n",size);
	printf("%d\n",slotLiberi(buffer));
	return 0;*/
}
