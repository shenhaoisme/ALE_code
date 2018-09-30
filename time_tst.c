#include <time.h>
#include <stdio.h>

/****

1: Both the ctime function and the gmtime and other...when incoming parameters are pointers. Please pay attention + "&".

**/


/*

char *ctime(const time_t * timep);
time_t is calendar_time.also called GMT(return seconds)
Convert from time_t(GMT) to Format B(formated string of time)
*/

void use_ctime()
{
	char ret_time[100];
	time_t tm_GMT = time(NULL); //no matter,anyway,It is NULL
	//ret_time = ctime(&tm_GMT);
	//printf("%s\n",ret_time);
	printf("%s\n", ctime(&tm_GMT));
}

/*
struct tm * gmtime(const time_t * timep)
struct tm * localtime(const time_t* timep)

struct tm{
	int tm_sec;
	int tm_min;
	...
};

The tm structure is calculated from GMT to how long from now, split into years and months.
*/


void use_gmtiem()
{
	time_t tm_GMT = time(NULL); //no matter,anyway,It is NULL
	struct tm tm1,*tmp;
	tmp = gmtime(&tm_GMT);
	tm1 = *tmp;
	printf("%d %d %d %d %d %d\n",tm1.tm_sec,tm1.tm_min,tm1.tm_hour,
tm1.tm_mday,tm1.tm_mon,tm1.tm_year);

	tmp = localtime(&tm_GMT);
	tm1 = *tmp;
	printf("%d %d %d %d %d %d\n",tm1.tm_sec,tm1.tm_min,tm1.tm_hour,
tm1.tm_mday,tm1.tm_mon,tm1.tm_year);
}
void main()
{
	//use_ctime();
	use_gmtiem();

}
