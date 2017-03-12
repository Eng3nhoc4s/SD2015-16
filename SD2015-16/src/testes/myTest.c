#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "data.h"
#include "entry.h"
#include "table-private.h"


void printKeys (char **c) {
	int i = 0;
	while(c[i] != NULL) {
		printf("%s |", c[i]);
		i++;	
	}
	printf("\n");
}

int main() {
	int score = 0;
	
	printf(" o hash de antonio = %d\n", hash("antonio", 10) );
	
	printf(" o hash de antnio = %d\n", hash("antnio", 10) );
	
	printf(" SOMA = %d\n", ('a'+'n'+'t'+'n'+'i'+'o'));
	
	printf(" o hash de jose = %d\n", hash("jose", 10) );

	printf(" o hash de esoj = %d\n", hash("esoj", 10) );

	printf(" o hash de aaaaaa = %d\n", hash("aaaaaa", 10) );
	
	printf(" o hash de a = %d\n", hash("a", 10) );

	struct table_t *t = table_create(10);
	table_put(t, "antonio", data_create(5));
	table_put(t, "antnio", data_create(5));
	table_put(t, "jose", data_create(5));
	table_put(t, "esoj", data_create(5));
	table_put(t, "aaaaaa", data_create(5));
	table_put(t, "a", data_create(5));
	
	char ** list = table_get_keys(t);
	printKeys(list);
	
	table_del(t,"a");
	printKeys(table_get_keys(t));
	table_put(t,"jose");
	printf("XXX%d\n",table_del(t,"a"));
	printKeys(table_get_keys(t));

	return score;
}
