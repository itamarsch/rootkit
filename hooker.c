
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>


struct callback {
	void* data_before_callback;
	size_t size_of_data;
	void* address_of_callback;
	

};


struct callback register_callback( void(*callback)(void), void(*targeted)(void), size_t targeted_size) {

	size_t pagesize = sysconf(_SC_PAGESIZE);
	void *page_start = (void*)((uintptr_t) targeted & ~(pagesize -1));
	if(mprotect(page_start, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
		perror("mprotect");
		exit(EXIT_FAILURE);
	}

	unsigned char* targeted_ptr = (unsigned char*) targeted;

	struct callback target_callback = {0};

	// Selected by random until it worked :)	
	size_t offset = 5;

	target_callback.size_of_data = targeted_size + offset;

	target_callback.data_before_callback = malloc(target_callback.size_of_data );
	memcpy(target_callback.data_before_callback, targeted, target_callback.size_of_data);

	target_callback.address_of_callback = targeted;




	memmove(targeted_ptr + offset, targeted_ptr, targeted_size);

	int inserted = 0;

	targeted_ptr[0] = 0xE8;
	
	inserted++;

	*((int*) (targeted_ptr + inserted)) = callback - targeted - 5; 

	inserted += 4;
	

	for(int i = inserted; i<offset; i++) {
		targeted_ptr[i] = 0x90;
	}
	return target_callback;

}


void unregister(struct callback clbck) {
	memcpy(clbck.address_of_callback, clbck.data_before_callback, clbck.size_of_data);

	free(clbck.data_before_callback);
}




void modified(void) {
	printf("world\n");
}


void foo() {
	printf("foo\n");
}


void hello(void) {
	printf("Hello\n");
}

int main (void) {
	struct callback hello_callback = register_callback(&hello, &modified,0x19);

	modified();


	unregister(hello_callback);
	printf("-------------Unregister-------------\n");


	modified();

	foo();
}



