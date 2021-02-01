#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>

int main(int argc, char *argv[])
{
	struct timeval tv;
	int uid;
	char *source_date_epoch;

	gettimeofday(&tv, NULL);
	uid = getuid();

	source_date_epoch = getenv("SOURCE_DATE_EPOCH");
	if (source_date_epoch) {
		int n;
		uint64_t value;

		n = sscanf(source_date_epoch, "%" PRIu64, &value);
		if (n != 1) {
			fprintf(stderr, "SOURCE_DATE_EPOCH environment variable value '%s' is not convertible to a uint64_t\n",
				source_date_epoch);
			fprintf(stderr, "Either make sure it is unset, or set it to seconds since midnight, Jan 1, 1970.\n");
			exit(1);
		}
		printf("static int builder = 0;\n");
		printf("static uint64_t builtat = %" PRIu64 ";\n", value);
		exit(0);
	}

	printf("static int builder = %d;\n", uid);
	printf("static uint64_t builtat = %ld;\n", tv.tv_sec);

	exit(0);	
}
