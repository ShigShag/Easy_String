#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RETURN_ON_NULL(x) if(x == NULL) return;

typedef unsigned long s_size;


void str_error_log(const char *file, const char *function, int line, char *msg){
	fprintf(stderr, "%s: In function '%s'\n%s:%d: error: %s\n", file, function, file, line, msg);
}

#define LOG(msg) str_error_log(__FILE__, __FUNCTION__, __LINE__, msg)

#ifdef WI32
// allocater type ---		  n elements     sizeof element flags
typedef void *(* allocater_t)(unsigned long, unsigned long, unsigned long);

// TODO
typedef void(* deallocater_t)(void *, unsigned long) 
#endif
// allocater type ---		  n elements     sizeof element
typedef void *(* allocater_t)(unsigned long, unsigned long);

// reallocater type ---			buffer  size
typedef void *(* reallocater_t)(void *, unsigned long);

// deallocater type ---               buffer
typedef void (* deallocater_t)(void *);

#ifdef ALLOCATER_WRAPPER
void *malloc_wrapper(size_t number_of_elements, size_t size_of_element){
	// printf("Allocating: %lu %lu\n", number_of_elements, size_of_element);
	return malloc(number_of_elements * size_of_element);
}

// TODO
#ifdef WIN32
#include <heapapi.h>

void *heap_alloc(size_t number_of_elements)

#endif
#endif

typedef struct string{
	char *data;
	s_size length;
} str;

typedef struct string_buffer{
	char *data;
	s_size length;
	s_size capacity;
} str_buf;

typedef struct string_mutable_buffer{
	char *data;
	s_size length;
	s_size capacity;
	reallocater_t reallocater;
} str_buf_mut;

unsigned long str_strlen(char *str){
	unsigned long length = 0;
	while(str[length] != '\0'){length++;}
	return length;
}

// Check if string is of traditional c type
#define IS_C_STRING(x) _Generic((x), char *: 1, unsigned char *: 1, void *: 1, default: 0)

// Creates a str struct from char * --- eg: str s = cstr("Hello");
#define cstr(str_) (IS_C_STRING(str_) ? ((str) {(char *) str_, str_strlen(str_)}) : ((str) { 0 }))

// Strlen for norrmal c string and new data type
#define STRLEN(str_) _Generic((str_), str: str_.length, str_buf: str_.length, default: 0)
#define STRLEN_GENERIC(str_) (str_strlen(str_))

// Get generic string
#define generic_str(str_) _Generic((str_), str: str_.data, str_buf: str_.data, str_buf_mut: str_.data, default: "")

// Create a string buffer
str_buf str_buf_make(s_size capacity, allocater_t allocater){
	str_buf buff = 
	{
		.data = (char *) allocater(capacity, sizeof(char)),
		.length = 0,
		.capacity = capacity,
	};
	if(buff.data == NULL){
		LOG("Allocation failed");
	}
	return buff;
}

void str_buf_append(str_buf *str_buf_, str str_){
	RETURN_ON_NULL(str_buf_)

	s_size bytes_left = str_buf_->capacity - str_buf_->length;
	s_size bytes_to_copy = (bytes_left >= str_.length) ? str_.length : str_.length - (str_.length - bytes_left);

	memcpy(str_buf_->data + str_buf_->length, str_.data, bytes_to_copy);

	str_buf_->length += bytes_to_copy;
}

void str_buf_insert(str_buf *str_buf_, str str_, s_size index){
	RETURN_ON_NULL(str_buf_)

	// Check if index is within the length of the current string
	if(str_buf_->length <= index){
		LOG("Index greater than length");
		return;
	}
	
	s_size bytes_left = str_buf_->capacity - str_buf_->length;

	// if there is not enough space
	if(bytes_left < str_.length){
		
	}

	for(s_size i = index;(i - index) != str_.length;i++){
		// index + length_of_string_to_be_inserted = new_index
		str_buf_->data[i + str_.length] = str_buf_->data[i];
		str_buf_->data[i] = str_.data[i - index];
		str_buf_->length++;
	}
}

// initialize a string buffer
str_buf cstr_buf(char *str_, s_size capacity, allocater_t allocater){
	str_buf buff = str_buf_make(capacity, allocater);
	str_buf_append(&buff, cstr(str_));
	return buff;
}

void str_buf_delete(str_buf *str_buf_, deallocater_t deallocater){
	deallocater(str_buf_->data);
}

// Create a mutable string buffer
str_buf_mut str_buf_mut_make(s_size capacity, allocater_t allocater, reallocater_t reallocater){
	str_buf_mut buff = 
	{
		.data = (char *) allocater(capacity, sizeof(char)),
		.length = 0,
		.capacity = capacity,
		.reallocater = reallocater,
	};
	if(buff.data == NULL){
		LOG("Allocation failed");
	}
	return buff;
}

void str_buf_mut_realloc(str_buf_mut *str_buf_mut_, s_size new_capacity){
	RETURN_ON_NULL(str_buf_mut_)

	char *temporary = str_buf_mut_->reallocater(str_buf_mut_->data, new_capacity);
	if(temporary == NULL){
		LOG("Could not reallocate");
		return;
	}
	str_buf_mut_->data = temporary;
	str_buf_mut_->capacity = new_capacity; 
}

void str_buf_mut_append(str_buf_mut *str_buf_mut_, str str_){
	RETURN_ON_NULL(str_buf_mut_)

	s_size bytes_left = str_buf_mut_->capacity - str_buf_mut_->length;

	// if there is not enough space
	if(bytes_left < str_.length){
		s_size new_capacity =  str_buf_mut_->capacity + (str_.length - bytes_left);
		str_buf_mut_realloc(str_buf_mut_, new_capacity);
	}

	memcpy(str_buf_mut_->data + str_buf_mut_->length, str_.data, str_.length);
	str_buf_mut_->length += str_.length;
}

void str_buf_mut_insert(str_buf_mut *str_buf_mut_, str str_, s_size index){
	RETURN_ON_NULL(str_buf_mut_)

	// Check if index is within the length of the current string
	if(str_buf_mut_->length <= index){
		LOG("Index greater than length");
		return;
	}
	
	s_size bytes_left = str_buf_mut_->capacity - str_buf_mut_->length;

	// if there is not enough space
	if(bytes_left < str_.length){
		s_size new_capacity =  str_buf_mut_->capacity + (str_.length - bytes_left);
		str_buf_mut_realloc(str_buf_mut_, new_capacity);
	}

	for(s_size i = index;(i - index) != str_.length;i++){
		// index + length_of_string_to_be_inserted = new_index
		str_buf_mut_->data[i + str_.length] = str_buf_mut_->data[i];
		str_buf_mut_->data[i] = str_.data[i - index];
		str_buf_mut_->length++;
	}
}

// Initialize a string buffer
str_buf_mut cstr_buf_mut(char *str_, s_size capacity, allocater_t allocater, reallocater_t reallocater){
	str_buf_mut buff = str_buf_mut_make(capacity, allocater, reallocater);
	str_buf_mut_append(&buff, cstr(str_));
	return buff;
}

void str_buf_mut_delete(str_buf_mut *str_buf_mut_, deallocater_t deallocater){
	deallocater(str_buf_mut_->data);
}

// Template for str_buf and str_buf_mut
#define cstr_append(str_buf_x, str_) _Generic(str_buf_x, str_buf *: str_buf_append, str_buf_mut *: str_buf_mut_append)(str_buf_x, str_)

int main (int argc, char *argv[])
{
	// str s = cstr("1asdsadas23");

	// printf("%s --- %lu\n", s.data, s.length);

	// printf("strlen: %lu\n", STRLEN(s));
	// printf("strlen: %lu\n", STRLEN_GENERIC("Hello"));

	// str_buf s = cstr_buf("Hello", 10, calloc);
	// str b = cstr("World");
	// cstr_append(&s, b);
	// printf("%s\n", s.data);
	// str_buf_delete(&s, free);
	// str_buf_append(&s, cstr("Hello World"));
	// printf("%s\n", s.data);
	// str_buf_append(&s, cstr("Hello World"));

	// str_buf s = cstr_buf("Hello World", 5, calloc);
	// printf("%s\n", s.data);

	// str_buf_delete(&s, free);
	// str_buf_mut b = str_buf_mut_make(50, calloc, realloc); 
	// str_buf_mut_append(&b, cstr("Hello"));
	// cstr_append(&b, cstr("fgeogjwerogjerogjerogjerogj"));

	str_buf_mut b = cstr_buf_mut("Hello", 5, calloc, realloc);

	printf("%s --- %lu --- %lu\n", b.data, b.capacity, b.length);
	printf("%s\n", generic_str(b));

	str_buf_mut_insert(&b, cstr("World"), 55);
	printf("%s --- %lu --- %lu\n", b.data, b.capacity, b.length);
	printf("%s\n", generic_str(b));
	str_buf_mut_delete(&b, free);

  	return 0;
}