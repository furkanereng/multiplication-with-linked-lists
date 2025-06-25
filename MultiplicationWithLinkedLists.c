/* CSE2225 - Fall 2024 - Project #1
 * This program multiplies positive two infinitely large numbers and prints the result to the output file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Defining number struct to hold numbers in a linked list. */
typedef struct number {
    int digit;
    struct number *next;
} Number;
typedef struct number *NumberPtr;

/* Function prototypes. */
void insert_head(NumberPtr *num_head, NumberPtr *num_tail, int digit);
void insert_tail(NumberPtr *num_head, NumberPtr *num_tail, int digit);
void delete_number(NumberPtr *num_head);
NumberPtr multiply(NumberPtr num1_head, NumberPtr num2_head);
void multiply_digit(NumberPtr num2_head, int digit, int carryout, NumberPtr *result_head, NumberPtr *result_tail);
NumberPtr add(NumberPtr num1_head, NumberPtr num2_head, int carryout);
void print_num(NumberPtr num_head, NumberPtr num_frac, FILE *outputf);
void print_result(NumberPtr result_head, int fraction, FILE *outputf);
int find_fraction(NumberPtr num_head, NumberPtr num_frac);
void trim(NumberPtr *num_head, int *frac);

int main() {
    FILE *inputf = fopen("input.txt", "r");
    if (inputf == NULL) { 
        printf("Input file can not be found.\n");
        return 1;
    }

    /* Pointers to heads, tails and starting points of fraction parts. */
    NumberPtr num1_head = NULL, num1_tail = NULL, num1_frac = NULL;
    NumberPtr num2_head = NULL, num2_tail = NULL, num2_frac = NULL;

    /* Reading file and initializing multiplier and multiplicand. */
    char buffer;
    while ((buffer = fgetc(inputf)) != '\n') {
        if (buffer == '.')
            num1_frac = num1_head;
        else if ('0' <= buffer && buffer <= '9')
            insert_head(&num1_head, &num1_tail, buffer - '0');
    }
    while ((buffer = fgetc(inputf)) != EOF) {
        if (buffer == '.')
            num2_frac = num2_head;
        else if ('0' <= buffer && buffer <= '9') 
            insert_head(&num2_head, &num2_tail, buffer - '0');
    }
    fclose(inputf);

    /* Perfoming multiplication and recording the execution time. */
    clock_t start = clock();
	NumberPtr result_head = multiply(num1_head, num2_head);
	clock_t end = clock();
	double execution_time = (double)(end - start) / CLOCKS_PER_SEC;
	int result_frac = find_fraction(num1_head, num1_frac) + find_fraction(num2_head, num2_frac);
	trim(&result_head, &result_frac);
	if (result_frac == 0) result_frac = -1;

    /* Printing the output to the file. */
	FILE *outputf = fopen("output.txt", "w");
	fprintf(outputf, "Multiplier: ");
	print_num(num1_head, num1_frac ? num1_frac : NULL, outputf);
	fprintf(outputf, "\nMultiplicand: ");
	print_num(num2_head, num2_frac ? num2_frac : NULL, outputf);
	fprintf(outputf, "\nResult: ");
    print_result(result_head, result_frac, outputf);
    fprintf(outputf, "\nExecution time of multiply function: %f milliseconds", execution_time * 1000);
    fclose(outputf);

    /* Freeing the allocated memory for multiplicand, multiplier and the result. */    
    delete_number(&result_head);
    result_head = NULL;
    delete_number(&num1_head);
    num1_head = NULL; num1_tail = NULL; num1_frac = NULL;
    delete_number(&num2_head);
    num2_head = NULL; num2_tail = NULL; num2_frac = NULL;
    return 0;
}

/* Inserts a digit to the head part (least significant part) and updates the head. */
void insert_head(NumberPtr *num_head, NumberPtr *num_tail, int digit) {
    NumberPtr new_num = (NumberPtr) malloc(sizeof(Number));
    if (new_num != NULL) {
        new_num->digit = digit;
        if (*num_head == NULL) {
            new_num->next = NULL;
            *num_head = new_num;
            *num_tail = new_num;
        }
        else {
            new_num->next = *num_head;
            *num_head = new_num;
        }
    }
    else printf("Memory can not be allocated.\n");
}

/* Inserts a digit to the tail part (most significant part) and updates the tail. */
void insert_tail(NumberPtr *num_head, NumberPtr *num_tail, int digit) {
    NumberPtr new_num = (NumberPtr) malloc(sizeof(Number));
    if (new_num != NULL) {
        new_num->digit = digit;
        if (*num_tail == NULL) {
            new_num->next = NULL;
            *num_tail = new_num;
            *num_head = new_num;
        }
        else {
        	new_num->next = NULL;
            (*num_tail)->next = new_num;
            *num_tail = new_num;
        }
    }
    else printf("Memory can not be allocated.\n");
}

/* Frees the whole memory that allocated for the linked list. */
void delete_number(NumberPtr *num_head) {
    /* Base Case: Reaching the end of the list. */
    if (*num_head == NULL)
        return;
    delete_number(&((*num_head)->next));
    free(*num_head);
    *num_head = NULL;
}

/* Performing the multiplication. */
NumberPtr multiply(NumberPtr num1_head, NumberPtr num2_head) {
    /* Base Case: Completion of traversing all digits of num1. */ 
    if (num1_head == NULL)
        return NULL;
    /* Multiplies num2 with a single digit of num1, and updates product1 list. */
    NumberPtr product1_head = NULL, product1_tail = NULL;
    multiply_digit(num2_head, num1_head->digit, 0, &product1_head, &product1_tail);
    /* Traverses num1 until no digits left, meanwhile calculates all single digit multiplications. */
    NumberPtr product2_head = multiply(num1_head->next ? num1_head->next : NULL, num2_head);
    if (product2_head != NULL) /* Shifting the result with using 0. */
    	insert_head(&product2_head, NULL, 0);
    /* Adds shifted result and the other result. */
    return add(product1_head ? product1_head : NULL, product2_head ? product2_head : NULL, 0);
} 

/* Multiplies num2 with a single digit of num1. */
void multiply_digit(NumberPtr num2_head, int digit, int carryout, NumberPtr *result_head, NumberPtr *result_tail) {
    /* Base Case: Completion of traversing all digits of num2 without having any carry out left. */
    if (num2_head == NULL && carryout == 0)
        return;
    /* Calculates the product of the num1's digit and a single digit of num2. */
    int product = carryout;
    if (num2_head != NULL)
    	product += num2_head->digit * digit;
    /* Inserts the product to result list and performs multiplication to the other digits. */
	insert_tail(result_head, result_tail, product % 10);
    multiply_digit(num2_head ? num2_head->next : NULL, digit, product / 10, result_head, result_tail);
}

/* Calculates the addition of two numbers. */
NumberPtr add(NumberPtr num1_head, NumberPtr num2_head, int carryout) {
    /* Base Case: Completion of traversing the both numbers without having any carry out left. */
    if (num1_head == NULL && num2_head == NULL && carryout == 0)
        return NULL;
    /* Calculates the addition result. */
    int sum = carryout;
    if (num1_head != NULL) {
        sum += num1_head->digit;
        num1_head = num1_head->next;
    }
    if (num2_head != NULL) {
        sum += num2_head->digit;
        num2_head = num2_head->next;
    }
    /* Allocates the memory for the digit and adds the next digits. */
    NumberPtr result = (NumberPtr) malloc(sizeof(Number));
    if (result != NULL) {
        result->digit = sum % 10;
        result->next = add(num1_head ? num1_head : NULL, num2_head ? num2_head : NULL, sum / 10);
    }
    else printf("Memory can not be allocated.\n");
    return result;
}

/* Finds the number of digits that exist after decimal point. */
int find_fraction(NumberPtr num_head, NumberPtr num_frac) {
	if (num_frac == NULL) /* If no exist. */
		return 0;
    /* Traverses the list until find the pointer to the fraction part. */
	int frac = 0;
	while (num_head != NULL) {
		if (num_head != num_frac) {
			frac++;
			num_head = num_head->next;
		}
		else break;
	}
	return frac;
}

/* Trims the unnecessary 0s in the fraction part. */
void trim(NumberPtr *num_head, int *frac) {
	NumberPtr current = *num_head, temp = *num_head;
	while (*frac > 0 && current->digit == 0) {
		current = current->next;
		*frac = *frac - 1;
	}
	while (temp != current) {
		*num_head = temp->next;
		free(temp);
		temp = *num_head;
	}
} 

/* Prints multiplicand and multiplier to the output file. */
void print_num(NumberPtr num_head, NumberPtr num_frac, FILE *outputf) {
    if (num_head == NULL)
        return;
    print_num(num_head->next ? num_head->next : NULL, num_frac, outputf);
    fprintf(outputf, "%d", num_head->digit);
    if (num_head == num_frac)
        fprintf(outputf, ".");
}

/* Prints result to the output file. */
void print_result(NumberPtr result_head, int fraction, FILE *outputf) {
	if (result_head == NULL)
        return;
    print_result(result_head->next ? result_head->next : NULL, fraction - 1, outputf);
    fprintf(outputf, "%d", result_head->digit);
    if (fraction == 0)
        fprintf(outputf, ".");
}

