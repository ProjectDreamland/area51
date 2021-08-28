struct tm {
        int tm_sec;     /* seconds after the minute - [0,59] */
        int tm_min;     /* minutes after the hour - [0,59] */
        int tm_hour;    /* hours since midnight - [0,23] */
        int tm_mday;    /* day of the month - [1,31] */
        int tm_mon;     /* months since January - [0,11] */
        int tm_year;    /* years since 1900 */
        int tm_wday;    /* days since Sunday - [0,6] */
        int tm_yday;    /* days since January 1 - [0,365] */
        int tm_isdst;   /* daylight savings time flag */
        };

#define CLOCKS_PER_SEC  1000

/* Function prototypes */

char * asctime(const struct tm *);
char * ctime(const time_t *);
clock_t clock(void);
double difftime(time_t, time_t);
struct tm * gmtime(const time_t *);
struct tm * localtime(const time_t *);
time_t  mktime(struct tm *);
size_t strftime(char *, size_t, const char *,const struct tm *);
/* not implemented under iso-c 
 *     char * _strdate(char *);
 *     char * _strtime(char *);
 */
time_t  time(time_t *);

