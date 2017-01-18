#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	struct timeval tv;
	int uid;
	char *source_date_epoch;

	gettimeofday(&tv, NULL);
	uid = getuid();

	source_date_epoch = getenv("SOURCE_DATE_EPOCH");
	if (source_date_epoch) {
		printf("static int builder = 0;\n");
		printf("static uint64_t builtat = %s;\n", source_date_epoch);
		exit(0);
	}

	printf("static int builder = %d;\n", uid);
	printf("static uint64_t builtat = %d;\n", tv.tv_sec);

	exit(0);	
}
