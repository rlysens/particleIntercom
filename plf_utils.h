#ifndef PLF_UTILS_H
#define PLF_UTILS_H

#define PLF_ASSERT_ENABLE

#ifdef PLF_ASSERT_ENABLE
#define plf_assert(error_string, condition) do { if (!(condition)) {Log.error(error_string); while(1);}} while (0)
#else
#define plf_assert(error_string, condition)
#endif /*PLF_ASSERT_ENABLE*/

#define PLF_PRINT(...) {Log.info(__VA_ARGS__);}

#ifndef MIN
#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#endif /* ifndef MIN */

#ifndef MAX
#define MAX(x,y)  ((x) > (y) ? (x) : (y))
#endif /* ifndef MAX */

#endif /*PLF_UTILS_H*/
