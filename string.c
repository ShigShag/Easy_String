#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define RETURN_VOID_ON_NULL(x) if(x == NULL) return;
#define RETURN_FALSE_ON_NULL(x) if(x == NULL) return false;
#define RETURN_ZERO_ON_NULL(x) if(x == NULL) return 0;

typedef unsigned long s_size;

// #ifndef TRUE
// #define TRUE (1)
// #endif

// #ifndef FALSE
// #define FALSE (0)
// #endif

void str_error_log(const char *file, const char *function, int line, char *msg){
	fprintf(stderr, "%s: In function '%s'\n%s:%d: error: %s\n", file, function, file, line, msg);
}

#define LOG(msg) str_error_log(__FILE__, __FUNCTION__, __LINE__, msg)

#ifdef WIN32
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

s_size str_strlen(char *str){
	unsigned long length = 0;
	while(str[length] != '\0'){length++;}
	return length;
}

#define cstr_print(str_buf) fwrite(str_buf.data, str_buf.length, sizeof(char), stdout)
#define cstr_debug_print(str_buf) cstr_print(str_buf); fprintf(stdout, "\nLength: %lu --- Capacity: %lu\n", str_buf.length, str_buf.capacity)

// Check if string is of traditional c type
#define IS_C_STRING(x) _Generic((x), char *: 1, unsigned char *: 1, void *: 1, default: 0)

// Creates a str struct from char * --- eg: str s = cstr("Hello");
#define cstr(str_) (IS_C_STRING(str_) ? ((str) {(char *) str_, str_strlen(str_)}) : ((str) { 0 }))

// Strlen for norrmal c string and new data type
#define STRLEN(str_) _Generic((str_), str: str_.length, str_buf: str_.length, default: str_strlen(str_))

bool str_contains(str *str_, str sequence){
	RETURN_FALSE_ON_NULL(str_)

	if(sequence.length == 0 || sequence.length > str_->length) return false;

	s_size counter = 0;

	for(s_size i = 0;i < str_->length;i++){
		if(counter == sequence.length){
			break;
		}

		if(str_->data[i] == sequence.data[counter]){
			counter++;
		} else{
			if(counter > 0){
				i -= counter;
			}
			counter = 0;
		}
	}
	return counter < sequence.length ? false : true;
}

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
	RETURN_VOID_ON_NULL(str_buf_)

	s_size bytes_left = str_buf_->capacity - str_buf_->length;
	s_size bytes_to_copy = (bytes_left >= str_.length) ? str_.length : str_.length - (str_.length - bytes_left);

	memcpy(str_buf_->data + str_buf_->length, str_.data, bytes_to_copy);

	str_buf_->length += bytes_to_copy;
}

void str_buf_insert(str_buf *str_buf_, str str_, s_size index){
	RETURN_VOID_ON_NULL(str_buf_)

	// Check if index is within the length of the current string
	if(str_buf_->length < index){
		LOG("Index greater than length");
		return;
	}
	
	s_size bytes_left = str_buf_->capacity - str_buf_->length;

	// if there is not enough space
	if(bytes_left < str_.length){
		LOG("Not enough space left to copy complete string");
		return;
	}

	for(s_size i = str_buf_->length - 1;i >= index;i--){
		str_buf_->data[i + str_.length] = str_buf_->data[i];

		// To prevent overflow if index is 0 
		if(i == index) break;
	}

	for(s_size i = index;(i - index) != str_.length;i++){
		str_buf_->data[i] = str_.data[i - index];
		str_buf_->length++;
	}
}

void str_buf_remove(str_buf *str_buf, s_size start, s_size end){
	RETURN_VOID_ON_NULL(str_buf)

	if(start > end || end >= str_buf->length) return;

	s_size shorting = (end - start) + 1;

	for(s_size i = end + 1;i < str_buf->length;i++){
		str_buf->data[i - shorting] = str_buf->data[i];
	}

	str_buf->length -= shorting;
}

bool str_buf_contains(str_buf *str_buf_, str sequence){
	RETURN_FALSE_ON_NULL(str_buf_)

	if(sequence.length == 0 || sequence.length > str_buf_->length) return false;

	s_size counter = 0;

	for(s_size i = 0;i < str_buf_->length;i++){
		if(counter == sequence.length){
			break;
		}

		if(str_buf_->data[i] == sequence.data[counter]){
			counter++;
		} else{
			if(counter > 0){
				i -= counter;
			}
			counter = 0;
		}
	}
	return counter < sequence.length ? false : true;
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
	RETURN_VOID_ON_NULL(str_buf_mut_)

	char *temporary = str_buf_mut_->reallocater(str_buf_mut_->data, new_capacity);
	if(temporary == NULL){
		LOG("Could not reallocate");
		return;
	}
	str_buf_mut_->data = temporary;
	str_buf_mut_->capacity = new_capacity; 
}

void str_buf_mut_append(str_buf_mut *str_buf_mut_, str str_){
	RETURN_VOID_ON_NULL(str_buf_mut_)

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
	RETURN_VOID_ON_NULL(str_buf_mut_)

	// Check if index is within the length of the current string
	if(str_buf_mut_->length < index){
		LOG("Index greater than length");
		return;
	}
	
	s_size bytes_left = str_buf_mut_->capacity - str_buf_mut_->length;

	// if there is not enough space
	if(bytes_left < str_.length){
		s_size new_capacity = str_buf_mut_->capacity + (str_.length - bytes_left);
		str_buf_mut_realloc(str_buf_mut_, new_capacity);
	}

	for(s_size i = str_buf_mut_->length - 1;i >= index;i--){
		str_buf_mut_->data[i + str_.length] = str_buf_mut_->data[i];

		// To prevent overflow if index is 0 
		if(i == index) break;
	}

	for(s_size i = index;(i - index) != str_.length;i++){
		str_buf_mut_->data[i] = str_.data[i - index];
		str_buf_mut_->length++;
	}
}

s_size str_buf_mut_count(str_buf_mut *str_buf_mut_, str sequence){
	RETURN_ZERO_ON_NULL(str_buf_mut_)

	if(sequence.length == 0) return 0;

	s_size sequence_count = 0;
	s_size counter = 0;

	for(s_size i = 0;i < str_buf_mut_->length;i++){
		if(counter == sequence.length){
			sequence_count++;
			counter = 0;
		}

		if(str_buf_mut_->data[i] == sequence.data[counter]){
			counter++;
		} else{
			if(counter > 0){
				i -= counter;
			}
			counter = 0;
		}
	}
	if(counter) sequence_count++;

	return sequence_count;
}

void str_buf_mut_remove(str_buf_mut *str_buf_mut_, s_size start, s_size end){
	RETURN_VOID_ON_NULL(str_buf_mut_)

	if(start > end || end >= str_buf_mut_->length) return;

	s_size shorting = (end - start) + 1;

	for(s_size i = end + 1;i < str_buf_mut_->length;i++){
		str_buf_mut_->data[i - shorting] = str_buf_mut_->data[i];
	}

	str_buf_mut_->length -= shorting;
}

// Count -> How often the sequence is to be replaced starting from the beginning
void str_buf_mut_replace(str_buf_mut *str_buf_mut_, str old_str, str new_str, s_size count){
	RETURN_VOID_ON_NULL(str_buf_mut_)
	if(count == 0) return;

	s_size old_str_count;

	old_str_count = str_buf_mut_count(str_buf_mut_, old_str);

	if(old_str_count == 0) return;

	s_size length_after_cut = str_buf_mut_->length - (old_str_count * old_str.length);
	s_size new_str_size = new_str.length * old_str_count;
	
	// if sum of new_str is greater than the capacity after subtracting the old string
	if(length_after_cut + new_str_size > str_buf_mut_->capacity){
		s_size new_capacity = new_str_size+ length_after_cut;
		str_buf_mut_realloc(str_buf_mut_, new_capacity);
	}

	long size_difference = new_str.length - old_str.length;
	s_size sequence_count = 0;
	s_size counter = 0;
	s_size start_index_of_old_str = 0;

	for(s_size i = 0;i <= str_buf_mut_->capacity;i++){
		if(counter == old_str.length){
			// If string to be inserted is bigger than the original
			if(size_difference > 0){
				for(s_size n = str_buf_mut_->length - 1;n >= i;n--){
					str_buf_mut_->data[n + size_difference] = str_buf_mut_->data[n];
				}
			// 1234567890
			// 12abcd77890
			// If string to be inserted is smaller than the orginal
			}else if(size_difference < 0){
				for(s_size n = i - 1;n <= str_buf_mut_->length + size_difference - 1;n++){
					str_buf_mut_->data[n] = str_buf_mut_->data[n - size_difference];
				}
			}
			str_buf_mut_->length += size_difference;

			memcpy(str_buf_mut_->data + start_index_of_old_str, new_str.data, new_str.length);

			sequence_count++;
			if(sequence_count == old_str_count) break;
			
			counter = 0;
			i += size_difference;
			if(i >= str_buf_mut_->capacity) break;
		}
		if(str_buf_mut_->data[i] == old_str.data[counter]){
			if(counter == 0){
				start_index_of_old_str = i;
			}
			counter++;
		} else{
			if(counter > 0){
				i -= counter;
			}
			counter = 0;
		}
	}
}

bool str_buf_mut_contains(str_buf_mut *str_buf_mut_, str sequence){
	RETURN_FALSE_ON_NULL(str_buf_mut_)

	if(sequence.length == 0 || sequence.length > str_buf_mut_->length) return false;

	s_size counter = 0;

	for(s_size i = 0;i < str_buf_mut_->length;i++){
		if(counter == sequence.length){
			break;
		}

		if(str_buf_mut_->data[i] == sequence.data[counter]){
			counter++;
		} else{
			if(counter > 0){
				i -= counter;
			}
			counter = 0;
		}
	}
	return counter < sequence.length ? false : true;
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
#define generic_str(str_) _Generic((str_), str: str_.data, str_buf: str_.data, str_buf_mut: str_.data, default: "")
#define cstr_delete(str_buf_x, deallocater) _Generic(str_buf_x, str_buf *: str_buf_delete, str_buf_mut *: str_buf_mut_delete)(str_buf_x, deallocater)
#define cstr_append(str_buf_x, str_) _Generic(str_buf_x, str_buf *: str_buf_append, str_buf_mut *: str_buf_mut_append)(str_buf_x, str_)
#define cstr_insert(str_buf_x, str_, index) _Generic(str_buf_x, str_buf *: str_buf_insert, str_buf_mut *: str_buf_mut_insert)(str_buf_x, str_, index)
#define cstr_remove(str_buf_x, start, end) _Generic(str_buf_x, str_buf *: str_buf_remove, str_buf_mut *: str_buf_mut_remove)(str_buf_x, start, end)
#define cstr_contains(str_buf_x, sequence) _Generic(str_buf_x, str *: str_contains, str_buf *: str_buf_contains, str_buf_mut *: str_buf_mut_contains)(str_buf_x, sequence)


int main (int argc, char *argv[])
{
	// str s = cstr("1asdsadas23");

	// printf("%s --- %lu\n", s.data, s.length);

	// printf("strlen: %lu\n", STRLEN(s));
	// printf("strlen: %lu\n", STRLEN_GENERIC("Hello"));

	// str_buf s = cstr_buf("Hello", 10, calloc);
	// cstr_remove(&s, 2, 5);
	// fwrite(s.data, s.length, 1, stdout);
	// printf("\n");
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

	// str_buf c = cstr_buf("Hello World", 30, calloc);
	// printf("%s --- %lu --- %lu\n", c.data, c.capacity, c.length);
	// cstr_insert(&c, cstr("my name issssssssss"), 7);
	// printf("%s --- %lu --- %lu\n", c.data, c.capacity, c.length);

	// cstr_delete(&c, free);

	str_buf_mut b = cstr_buf_mut("sss1234sss567890sssssx", 0, calloc, realloc);
	cstr_debug_print(b);

	str_buf_mut_replace(&b, cstr("x"), cstr("zzzz"), 4);

	// cstr_print(b);
	cstr_debug_print(b);
	printf("\n");
	// printf("%d\n%d\n%d\n", cstr_contains(&b, cstr("Hello")), cstr_contains(&a, cstr("cool")), cstr_contains(&c, cstr("happy")));
  	return 0;
}