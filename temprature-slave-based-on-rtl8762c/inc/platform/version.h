#define VERSION_MAJOR            1
#define VERSION_MINOR            2
#define VERSION_REVISION         2
#define VERSION_BUILDNUM         1
#define VERSION_GCID             0xab12567c
#define CUSTOMER_NAME            sdk
#define CN_1                     'r'
#define CN_2                     'c'
#define CN_3                     'u'
#define CN_4                     's'
#define CN_5                     'd'
#define CN_6                     'k'
#define CN_7                     '#'
#define CN_8                     '#'
#define BUILDING_TIME            "Tue Sep 24 10:57:13 2019"
#define NAME2STR(a)              #a
#define CUSTOMER_NAME_S          #NAME2STR(CUSTOMER_NAME)
#define NUM4STR(a,b,c,d)         #a "." #b "." #c "." #d
#define VERSIONBUILDSTR(a,b,c,d) NUM4STR(a,b,c,d)
#define VERSION_BUILD_STR        VERSIONBUILDSTR(VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION,VERSION_BUILDNUM)
