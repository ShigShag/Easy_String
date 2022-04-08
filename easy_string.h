#ifndef EASY_STRING_H
#define EASY_STRING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>

#define RETURN_VOID_ON_NULL(x) if(x == NULL) return;
#define RETURN_FALSE_ON_NULL(x) if(x == NULL) return false;
#define RETURN_ZERO_ON_NULL(x) if(x == NULL) return 0;

#define UTIL_ESTR_INFINITE 0xffffffffffffffff;

#define ESTR_INVALID_INDEX (-1)

// Macro name generation
#define util_estr_concat_implementation(a, b) a##b
#define util_estr_concat(a, b) util_estr_concat_implementation(a, b)

// estr generate name
#define util_estr_gmn(x) util_estr_concat(estr_macro_name, util_estr_concat(x, __LINE__))

typedef uint64_t e_size;
typedef int64_t e_index;

void str_error_log(const char *file, const char *function, int line, char *msg){
	fprintf(stderr, "%s: In function '%s'\n%s:%d: error: %s\n", file, function, file, line, msg);
}

#define LOG(msg) str_error_log(__FILE__, __FUNCTION__, __LINE__, msg)

// allocater type ---		  n elements     sizeof element
typedef void *(* allocater_t)(uint64_t, uint64_t);

// reallocater type ---			buffer  size
typedef void *(* reallocater_t)(void *, uint64_t);

// deallocater type ---        buffer
typedef void (* deallocater_t)(void *);

// Check if string is of traditional c type
#define IS_C_STRING(x) _Generic((x), char *: 1, unsigned char *: 1, void *: 1, default: 0)

#ifdef ESTR_ALLOCATER_WRAPPER

void *estr_malloc_wrapper(size_t number_of_elements, size_t size_of_element){
	// printf("Allocating: %lu %lu\n", number_of_elements, size_of_element);
	return malloc(number_of_elements * size_of_element);
}


#endif

/* ESTR STRING TYPES */
typedef struct string{
	char *data;
	e_size length;
} str;

typedef struct string_mutable_buffer{
	char *data;
	e_size length;
	e_size capacity;
	reallocater_t reallocater;
} str_buf_mut;

typedef str_buf_mut estr_buf;

e_size str_strlen(char *str){
	uint64_t length = 0;
	while(str[length] != '\0'){length++;}
	return length;
}
/*------------- str FUNCTIONS -------------*/
// Checks if the buffer contains a str sequence
bool str_contains(str *str_, str sequence){
	RETURN_FALSE_ON_NULL(str_)

	if(sequence.length == 0 || sequence.length > str_->length) return false;

	e_size counter = 0;

	for(e_size i = 0;i < str_->length;i++){
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

// Counts a non overlapping sequence in a string buffer
e_size str_count(str *str_, str sequence){
	RETURN_ZERO_ON_NULL(str_)

	if(sequence.length == 0) return 0;

	e_size sequence_count = 0;
	e_size counter = 0;

	for(e_size i = 0;i < str_->length;i++){
		if(counter == sequence.length){
			sequence_count++;
			// counter = 0;
		}

		if(str_->data[i] == sequence.data[counter]){
			counter++;
		} else{
			// if(counter > 0){
			// 	i -= counter;
			// }
			counter = 0;
		}
	}
	if(counter) sequence_count++;

	return sequence_count;
}

// Checks if the buffer starts with a specific sequence
bool str_startswith(str *str_, str sequence){
	RETURN_FALSE_ON_NULL(str_)

	if(sequence.length == 0 || sequence.length > str_->length) return false;

	for(e_size i = 0;i < sequence.length;i++){
		if(str_->data[i] != sequence.data[i]) return false;
	}
	return true;
}

// Checks if the buffer ends with a specific sequence
bool str_endswith(str *str_, str sequence){
	RETURN_FALSE_ON_NULL(str_)

	if(sequence.length == 0 || sequence.length > str_->length) return false;

	for(e_size i = str_->length - sequence.length;i < str_->length;i++){
		if(str_->data[i] != sequence.data[i - (str_->length - sequence.length)]) return false;
	}
	return true;
}

// Finds the first occurence of a sequence in a string and returns the index
e_index str_find_first_no_pointer(str str_buf_, str sequence, e_size offset){
	if(offset >= str_buf_.length) return ESTR_INVALID_INDEX;
	e_size start_index_of_sequence;
	e_size counter = 0;

	for(e_size i = 0;i <= str_buf_.length;i++){
		if(counter == sequence.length){
			return start_index_of_sequence;
		}

		if(str_buf_.data[i] == sequence.data[counter]){
			if(counter == 0){
				start_index_of_sequence = i;
			}
			counter++;
		} else{
			if(counter > 0){
				i -= counter;
			}
			counter = 0;
		}
	}
	return -1;
}

// Finds the first occurence of a sequence in a string and returns the index
e_index str_find_first(str *str_buf_, str sequence, e_size offset){
	if(offset >= str_buf_->length) return ESTR_INVALID_INDEX;
	e_size start_index_of_sequence;
	e_size counter = 0;

	for(e_size i = offset;i <= str_buf_->length;i++){
		if(counter == sequence.length){
			return start_index_of_sequence;
		}

		if(str_buf_->data[i] == sequence.data[counter]){
			if(counter == 0){
				start_index_of_sequence = i;
			}
			counter++;
		} else{
			if(counter > 0){
				i -= counter;
			}
			counter = 0;
		}
	}
	return -1;
}

// This function will return the number split of an estr object
str str_split_number(str *str_, str split, e_size number){
	str r_str = { 0 };
	e_size split_counter = 0;
	e_size start_index_of_sequence = 0;
	e_size start_index_of_next_sequence = 0;
	e_index start_index_of_split = 0;

	while(split_counter != number){
		start_index_of_split = str_find_first(str_, split, start_index_of_next_sequence);
		start_index_of_sequence = start_index_of_next_sequence;
		if(start_index_of_split == ESTR_INVALID_INDEX){
			start_index_of_split = str_->length;
			break;
		}
		start_index_of_next_sequence = start_index_of_split + split.length; 
		split_counter++;
	}
	r_str.length = start_index_of_split - start_index_of_sequence;
	r_str.data = str_->data + start_index_of_sequence;

	return r_str;
}


/*------------- str_buf_mut FUNCTIONS -------------*/
str_buf_mut str_buf_mut_make(e_size capacity, allocater_t allocater, reallocater_t reallocater){
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

// Reallocates the current buffer to a new size
void str_buf_mut_realloc(str_buf_mut *str_buf_mut_, e_size new_capacity){
	RETURN_VOID_ON_NULL(str_buf_mut_)

	if(str_buf_mut_->reallocater == NULL){
		LOG("No reallocater was provided");
		return;
	}

	char *temporary = (char *) str_buf_mut_->reallocater(str_buf_mut_->data, new_capacity);
	if(temporary == NULL){
		LOG("Could not reallocate");
		return;
	}
	str_buf_mut_->data = temporary;
	str_buf_mut_->capacity = new_capacity; 
}

// Append a string to the end of the current buffer
void str_buf_mut_append(str_buf_mut *str_buf_mut_, str str_){
	RETURN_VOID_ON_NULL(str_buf_mut_)

	e_size bytes_left = str_buf_mut_->capacity - str_buf_mut_->length;

	// if there is not enough space
	if(bytes_left < str_.length){
		e_size new_capacity =  str_buf_mut_->capacity + (str_.length - bytes_left);
		str_buf_mut_realloc(str_buf_mut_, new_capacity);
	}

	memcpy(str_buf_mut_->data + str_buf_mut_->length, str_.data, str_.length);
	str_buf_mut_->length += str_.length;
}

// Inserts a string at specified index into the buffer
void str_buf_mut_insert(str_buf_mut *str_buf_mut_, str str_, e_size index){
	RETURN_VOID_ON_NULL(str_buf_mut_)

	// Check if index is within the length of the current string
	if(str_buf_mut_->length < index){
		LOG("Index greater than length");
		return;
	}
	
	e_size bytes_left = str_buf_mut_->capacity - str_buf_mut_->length;

	// if there is not enough space
	if(bytes_left < str_.length){
		e_size new_capacity = str_buf_mut_->capacity + (str_.length - bytes_left);
		str_buf_mut_realloc(str_buf_mut_, new_capacity);
	}

	for(e_size i = str_buf_mut_->length - 1;i >= index;i--){
		str_buf_mut_->data[i + str_.length] = str_buf_mut_->data[i];

		// To prevent overflow if index is 0 
		if(i == index) break;
	}

	for(e_size i = index;(i - index) != str_.length;i++){
		str_buf_mut_->data[i] = str_.data[i - index];
		str_buf_mut_->length++;
	}
}

// Removes a sequence specified by start and end index
void str_buf_mut_remove(str_buf_mut *str_buf_mut_, e_size start, e_size end){
	RETURN_VOID_ON_NULL(str_buf_mut_)

	if(start > end || end >= str_buf_mut_->length) return;

	e_size shorting = (end - start) + 1;

	for(e_size i = end + 1;i < str_buf_mut_->length;i++){
		str_buf_mut_->data[i - shorting] = str_buf_mut_->data[i];
	}

	str_buf_mut_->length -= shorting;
}

// Checks if the buffer contains a str sequence
bool str_buf_mut_contains(str_buf_mut *str_buf_mut_, str sequence){
	RETURN_FALSE_ON_NULL(str_buf_mut_)

	if(sequence.length == 0 || sequence.length > str_buf_mut_->length) return false;

	e_size counter = 0;

	for(e_size i = 0;i < str_buf_mut_->length;i++){
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

// Counts a non overlapping sequence in a string buffer
e_size str_buf_mut_count(str_buf_mut *str_buf_mut_, str sequence){
	RETURN_ZERO_ON_NULL(str_buf_mut_)

	if(sequence.length == 0) return 0;

	e_size sequence_count = 0;
	e_size counter = 0;

	for(e_size i = 0;i < str_buf_mut_->length;i++){
		if(counter == sequence.length){
			sequence_count++;
			counter = 0;
		}

		if(str_buf_mut_->data[i] == sequence.data[counter]){
			counter++;
		} 
		else{
			if(counter > 0){
				i -= counter;
			}
			counter = 0;
		}
	}
	if(counter) sequence_count++;

	return sequence_count;
}

// Replaces a sequence in a buffer with a new sequence
void str_buf_mut_replace(str_buf_mut *str_buf_mut_, str old_str, str new_str, e_size count){
	RETURN_VOID_ON_NULL(str_buf_mut_)
	if(count == 0) return;

	e_size old_str_count;

	old_str_count = str_buf_mut_count(str_buf_mut_, old_str);

	if(old_str_count == 0) return;

	e_size length_after_cut = str_buf_mut_->length - (old_str_count * old_str.length);
	e_size new_str_size = new_str.length * old_str_count;
	
	// if sum of new_str is greater than the capacity after subtracting the old string
	if(length_after_cut + new_str_size > str_buf_mut_->capacity){
		e_size new_capacity = new_str_size + length_after_cut;
		str_buf_mut_realloc(str_buf_mut_, new_capacity);
	}

	long size_difference = new_str.length - old_str.length;
	e_size sequence_count = 0;
	e_size counter = 0;
	e_size start_index_of_old_str = 0;

	for(e_size i = 0;i <= str_buf_mut_->capacity;i++){
		if(counter == old_str.length){
			// If string to be inserted is bigger than the original
			if(size_difference > 0){
				for(e_size n = str_buf_mut_->length - 1;n >= i;n--){
					str_buf_mut_->data[n + size_difference] = str_buf_mut_->data[n];
				}
			// If string to be inserted is smaller than the orginal
			}else if(size_difference < 0){
				for(e_size n = i - 1;n <= str_buf_mut_->length + size_difference - 1;n++){
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

// Checks if the buffer starts with a specific sequence
bool str_buf_mut_startswith(str_buf_mut *str_buf_mut_, str sequence){
	RETURN_FALSE_ON_NULL(str_buf_mut_)

	if(sequence.length == 0 || sequence.length > str_buf_mut_->length) return false;

	for(e_size i = 0;i < sequence.length;i++){
		if(str_buf_mut_->data[i] != sequence.data[i]) return false;
	}
	return true;
}

// Checks if the buffer ends with a specific sequence
bool str_buf_mut_endswith(str_buf_mut *str_buf_mut_, str sequence){
	RETURN_FALSE_ON_NULL(str_buf_mut_)

	if(sequence.length == 0 || sequence.length > str_buf_mut_->length) return false;

	for(e_size i = str_buf_mut_->length - sequence.length;i < str_buf_mut_->length;i++){
		if(str_buf_mut_->data[i] != sequence.data[i - (str_buf_mut_->length - sequence.length)]) return false;
	}
	return true;
}

// Finds the first occurence of a sequence in the string and returns the index
e_index str_buf_mut_find_first(str_buf_mut *str_buf_mut_, str sequence, e_size offset){
	if(str_buf_mut_ == NULL || offset >= str_buf_mut_->length) return ESTR_INVALID_INDEX;

	e_size start_index_of_sequence;
	e_size counter = 0;

	for(e_size i = offset;i <= str_buf_mut_->length;i++){
		if(counter == sequence.length){
			return start_index_of_sequence;
		}

		if(str_buf_mut_->data[i] == sequence.data[counter]){
			if(counter == 0){
				start_index_of_sequence = i;
			}
			counter++;
		} else{
			if(counter > 0){
				i -= counter;
			}
			counter = 0;
		}
	}
	return -1;
}

// This function will return the number split of an estr object
str str_buf_mut_split_number(str_buf_mut *str_buf_mut_, str split, e_size number){
	str r_str = { 0 };
	e_size split_counter = 0;
	e_size start_index_of_sequence = 0;
	e_size start_index_of_next_sequence = 0;
	e_index start_index_of_split = 0;

	while(split_counter != number){
		start_index_of_split = str_buf_mut_find_first(str_buf_mut_, split, start_index_of_next_sequence);
		start_index_of_sequence = start_index_of_next_sequence;
		if(start_index_of_split == ESTR_INVALID_INDEX){
			start_index_of_split = str_buf_mut_->length;
			break;
		}
		start_index_of_next_sequence = start_index_of_split + split.length; 
		split_counter++;
	}
	r_str.length = start_index_of_split - start_index_of_sequence;
	r_str.data = str_buf_mut_->data + start_index_of_sequence;

	return r_str;
}

/*str_buf_mut DECONSTRUCTOR*/
void str_buf_mut_delete(str_buf_mut *str_buf_mut_, deallocater_t deallocater){
	deallocater(str_buf_mut_->data);
}

/*-------------INTERFACE for fast and generic usage-------------*/

/*str CONSTRUCTOR*/
#define estr(str_) (IS_C_STRING(str_) ? ((str) {(char *) str_, str_strlen(str_)}) : ((str) { 0 }))

/*str_buf_mut CONSTRUCTOR*/
str_buf_mut estr_buf_mut(char *str_, e_size capacity, allocater_t allocater, reallocater_t reallocater){
	str_buf_mut buff = str_buf_mut_make(capacity, allocater, reallocater);
	str_buf_mut_append(&buff, estr(str_));
	return buff;
}

// Returns the length of an estr object
#define estr_len(str_) _Generic((str_), str: str_.length, default: str_strlen(str_))

// Prints the contents of a estr object to the screen
#define estr_print(str_buf_x) fwrite(str_buf_x.data, str_buf_x.length, sizeof(char), stdout)

// Prints the contents and length of estr object
#define estr_str_debug(str_) estr_print(str_); fprintf(stdout, "\tLength: %lu\n", str_.length) 

// Prints the contents and the length and capacity
#define estr_buf_debug_print(str_buf_x) estr_print(str_buf_x); fprintf(stdout, "\nLength: %lu --- Capacity: %lu\n", str_buf_x.length, str_buf_x.capacity)

// Returns a statement with the char * value of the object
#define estr_generic_str(str_) _Generic((str_), str: str_.data, str_buf_mut: str_.data, default: "")

// Delets a estr object with a given deconstructor
#define estr_delete(str_buf_x, deallocater) _Generic(str_buf_x, str_buf_mut *: str_buf_mut_delete)(str_buf_x, deallocater)

// Appends a string to the end of an estr object
#define estr_append(str_buf_x, str_) _Generic(str_buf_x,str_buf_mut *: str_buf_mut_append)(str_buf_x, str_)

// Inserts a sequence into an estr object at a given index
#define estr_insert(str_buf_x, str_, index) _Generic(str_buf_x,str_buf_mut *: str_buf_mut_insert)(str_buf_x, str_, index)

// Removes a sequence in an estr object especified by start and end index
#define estr_remove(str_buf_x, start, end) _Generic(str_buf_x, str_buf_mut *: str_buf_mut_remove)(str_buf_x, start, end)

// Replaces a sequence in an estr object with a new sequence
#define estr_replace(str_buf_x, old_str, new_str, count)  _Generic(str_buf_x, str_buf_mut *: str_buf_mut_replace)(str_buf_x, old_str, new_str, count)

// Checks if an estr object contains a certain str sequence
#define estr_contains(str_buf_x, sequence) _Generic(str_buf_x, str *: str_contains, str_buf_mut *: str_buf_mut_contains)(str_buf_x, sequence)

// Counts a non overlapping sequence in an estr object string buffer
#define estr_count(str_buf_x, sequence) _Generic(str_buf_x, str *: str_count, str_buf_mut *: str_buf_mut_count)(str_buf_x, sequence)

// Checks if the buffer in an estr object starts with a specific sequence
#define estr_startswith(str_buf_x, sequence) _Generic(str_buf_x, str *: str_startswith, str_buf_mut *: str_buf_mut_startswith)(str_buf_x, sequence)

// Checks if the buffer in an estr object ends with a specific sequence
#define estr_endswith(str_buf_x, sequence) _Generic(str_buf_x, str *: str_endswith, str_buf_mut *: str_buf_mut_endswith)(str_buf_x, sequence)

// Finds the first occurence of a sequence in the string and returns the index
#define estr_find_first(str_buf_x, sequence, offset) _Generic(str_buf_x, str : str_find_first_no_pointer, str *: str_find_first, str_buf_mut *: str_buf_mut_find_first)(str_buf_x, sequence, offset)

// Returns the number split of an estr object
#define estr_split_number(str_buf_x, split, number) _Generic(str_buf_x, str *: str_split_number, str_buf_mut *: str_buf_mut_split_number)(str_buf_x, split, number)

// split string
#define estr_split(str_buf_x, split, str) \
e_size util_estr_gmn(c) = estr_count(str_buf_x, split); \
e_size util_estr_gmn(i) = 0; \
str = estr_split_number(str_buf_x, split, util_estr_gmn(i) + 1); \
for(;util_estr_gmn(i) < util_estr_gmn(c) + 1;util_estr_gmn(i)++,str = estr_split_number(str_buf_x, split, util_estr_gmn(i) + 1))

#endif
