#define VERSION_MAJOR            1
#define VERSION_MINOR            2
#define VERSION_REVISION         1
#define VERSION_BUILDNUM         10
#define VERSION_GCID             0x0fa97d16
#define CUSTOMER_NAME            sdk
#define CN_1                     '#'
#define CN_2                     'M'
#define CN_3                     'a'
#define CN_4                     's'
#define CN_5                     't'
#define CN_6                     'e'
#define CN_7                     'r'
#define CN_8                     '#'
#define BUILDING_TIME            "Tue Dec 24 18:52:20 2019"
#define NAME2STR(a)              #a
#define CUSTOMER_NAME_S          #NAME2STR(CUSTOMER_NAME)
#define NUM4STR(a,b,c,d)         #a "." #b "." #c "." #d
#define VERSIONBUILDSTR(a,b,c,d) NUM4STR(a,b,c,d)
#define VERSION_BUILD_STR        VERSIONBUILDSTR(VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION,VERSION_BUILDNUM)
