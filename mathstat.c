/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Eugene Kurilov <eugenekurilov@gmail.com>                     |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_mathstat.h"
//#include <curses.h>
/* If you declare any globals in php_mathstat.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(mathstat)
*/

/* True global resources - no need for thread safety here */
static int le_mathstat;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("mathstat.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_mathstat_globals, mathstat_globals)
    STD_PHP_INI_ENTRY("mathstat.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_mathstat_globals, mathstat_globals)
PHP_INI_END()
*/
/* }}} */

static long calculate(long number) 
{
  if(number == 0) {
    return 1;
  }

  return number * calculate(number - 1);
}

static HashTable *sort(HashTable *ht)
{
   HashTable *result;
   zval *value;

   uint32_t cnt =  zend_array_count(ht);
   if(cnt <= 1) {
      return ht;
   }

   ALLOC_HASHTABLE(result);
   zend_hash_init(result, cnt, NULL, ZVAL_PTR_DTOR, 0);

   double array[cnt];
   int iter = 0;

   //php_printf("size hashtable: %d used %d ", ht->nTableSize, ht->nNumUsed);
   //php_printf("count: %d ", ht->nNumOfElements);
 
   while ((value = zend_hash_get_current_data(ht)) != NULL) {
      array[iter] = zval_get_double(value);
      zend_hash_move_forward(ht);
      iter+=1;
   }


   /* bubble sort  */
   int i,j;

   for(i=0; i < cnt; i++) {
     for(j=0; j < cnt; j++) {
        if((j+1) < cnt && array[j+1] < array[j] ) {
           array[j+1] = array[j+1] + array[j];
           array[j] = array[j+1] - array[j];
           array[j+1] = array[j+1] - array[j];   
        }
     }
   }
   
   for(i=0; i < cnt; i++) {
      zval temp;   
      ZVAL_LONG(&temp, array[i]);
      zend_hash_next_index_insert(result, &temp);     
   }

   return result;
}


PHP_FUNCTION(ms_sort)
{
   int argc = ZEND_NUM_ARGS(), key = 0;

   zval *array,
	*value;

   if (zend_parse_parameters(argc, "a", &array) == FAILURE) {
        RETURN_FALSE;
   }
   
   HashTable *ht = sort(Z_ARRVAL_P(array));
   array_init(return_value);

   while ((value = zend_hash_get_current_data(ht)) != NULL) {
      add_index_long(return_value, key,  zval_get_double(value)); 
      zend_hash_move_forward(ht);
      key += 1;
   }
}

PHP_FUNCTION(ms_factorial) 
{
   //initscr();
   //raw();
   //printw("test");
   //getch();
   //endwin();   

   int argc = ZEND_NUM_ARGS();
   long number = 0;

   if (zend_parse_parameters(argc, "l", &number) == FAILURE) {
	RETURN_LONG(0);
   }

   number = calculate(number);

   RETURN_LONG(number);
}

PHP_FUNCTION(ms_minimum)
{
   int argc = ZEND_NUM_ARGS();
   double minimum = ZEND_LONG_MAX, cur_value = 0;
   zend_long cnt;
   zval *array,
        *value;

   if (zend_parse_parameters(argc, "a", &array) == FAILURE) {
        RETURN_DOUBLE(minimum);
   }

   cnt = zend_array_count(Z_ARRVAL_P(array));
   if(!cnt) {
	RETURN_DOUBLE(0);
   }

   ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(array), value) {

        if (Z_TYPE_P(value) == IS_ARRAY || Z_TYPE_P(value) == IS_OBJECT) {
      	    continue;
        }

        cur_value = zval_get_double(value);
        if(cur_value < minimum) {
           minimum = cur_value;
        }

   } ZEND_HASH_FOREACH_END();

   RETURN_DOUBLE(minimum);
}

PHP_FUNCTION(ms_maximal)
{
   int argc = ZEND_NUM_ARGS();
   double maximal = ZEND_LONG_MIN, cur_value = 0;
   zend_long cnt;
   zval *array,
        *value;

   if (zend_parse_parameters(argc, "a", &array) == FAILURE) {
        RETURN_DOUBLE(maximal);
   }

   cnt = zend_array_count(Z_ARRVAL_P(array));
   if(!cnt) {
        RETURN_DOUBLE(0);
   }

   ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(array), value) {

        if (Z_TYPE_P(value) == IS_ARRAY || Z_TYPE_P(value) == IS_OBJECT) {
            continue;
        }

        cur_value = zval_get_double(value);
        if(cur_value > maximal) {
           maximal = cur_value;
        }

   } ZEND_HASH_FOREACH_END();

   RETURN_DOUBLE(maximal);
}


PHP_FUNCTION(ms_median)
{
   int argc = ZEND_NUM_ARGS();
   double total = 0;
   int count = 0;
   zval *array, 
        *value;

   if (zend_parse_parameters(argc, "a", &array) == FAILURE) {
	RETURN_DOUBLE(0);
   }
  
   ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(array), value) {
	total = total + zval_get_double (value);
        count += 1;
   } ZEND_HASH_FOREACH_END();

   if (count == 0 || total == 0) {
   	RETURN_DOUBLE(0);
   }
 
   RETURN_DOUBLE(total/count);
}

PHP_FUNCTION(ms_third_quartile)
{
   int argc = ZEND_NUM_ARGS();

   zval *array,
        *value;

   if (zend_parse_parameters(argc, "a", &array) == FAILURE) {
        RETURN_FALSE;
   }

   HashTable *ht = sort(Z_ARRVAL_P(array));
   uint32_t cnt = zend_array_count(ht); 

   if (cnt == 0) {
      RETURN_DOUBLE(0);
   } else if (cnt >= 1 && cnt <= 3) {
 
      zend_string *string_key;
      zend_ulong num_key;
 
      ZEND_HASH_REVERSE_FOREACH_KEY_VAL(ht, num_key, string_key, value) {
         RETURN_DOUBLE(zval_get_double(value));
      } ZEND_HASH_FOREACH_END(); 
   }

   uint32_t cnt75 = ceil(cnt*0.75); 

   int iter = 1;
   while ((value = zend_hash_get_current_data(ht)) != NULL) {
      if (iter == cnt75) {
        RETURN_DOUBLE(zval_get_double(value));  
      }
      zend_hash_move_forward(ht);
      iter += 1;
   }

   RETURN_DOUBLE(0);
}

PHP_FUNCTION(ms_first_quartile) 
{
   int argc = ZEND_NUM_ARGS();

   zval *array,
        *value;

   if (zend_parse_parameters(argc, "a", &array) == FAILURE) {
        RETURN_FALSE;
   }

   HashTable *ht = sort(Z_ARRVAL_P(array));
   uint32_t cnt = zend_array_count(ht); 

   if (cnt == 0) {
      RETURN_DOUBLE(0);
   } else if (cnt >= 1 && cnt <= 3) {
      value = zend_hash_get_current_data(ht);
      RETURN_DOUBLE(zval_get_double(value));
   }
  
   uint32_t cnt25 = ceil(cnt*0.25); 

   int iter = 1;
   while ((value = zend_hash_get_current_data(ht)) != NULL) {
      if (iter == cnt25) {
        RETURN_DOUBLE(zval_get_double(value));  
      }
      zend_hash_move_forward(ht);
      iter += 1;
   }

   RETURN_DOUBLE(0);
}


static HashTable *unique(HashTable *ht)
{
   zval *value;
   double dvalue;

   if(zend_array_count(ht) <= 1) {
      return ht;
   }

   double *array = malloc(zend_array_count(ht)*sizeof(double)); 
   zend_string *str_index = NULL;
   zend_ulong num_index;
   double dbl_value;

   int cnt = 0, i = 0;
 
   while ((value = zend_hash_get_current_data(ht)) != NULL) {

      zend_hash_get_current_key(ht, &str_index, &num_index);
      if(dbl_value == zval_get_double(value)) {
         array[cnt] = num_index; 
         cnt +=1;
      } else {
        dbl_value = zval_get_double(value);
      }

      zend_hash_move_forward(ht);
   }
 
   for(i = 0; i < cnt; i++) {
      zend_hash_index_del(ht, array[i]);
   }
 
   free(array);
   zend_hash_internal_pointer_reset(ht);

   return ht;
}

PHP_FUNCTION(ms_unique) 
{
   int argc = ZEND_NUM_ARGS(), key = 0;

   zval *array,
	*value;

   if (zend_parse_parameters(argc, "a", &array) == FAILURE) {
        RETURN_FALSE;
   }

   HashTable *ht = unique(sort(Z_ARRVAL_P(array)));
   array_init(return_value);

   while ((value = zend_hash_get_current_data(ht)) != NULL) {
      add_index_long(return_value, key, zval_get_double(value)); 
      zend_hash_move_forward(ht);
      key += 1;
   }

}

/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_mathstat_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_mathstat_init_globals(zend_mathstat_globals *mathstat_globals)
{
	mathstat_globals->global_value = 0;
	mathstat_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(mathstat)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(mathstat)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(mathstat)
{
#if defined(COMPILE_DL_MATHSTAT) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(mathstat)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(mathstat)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "mathstat support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


ZEND_BEGIN_ARG_INFO(arginfo_ms_factorial, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()


/* {{{ mathstat_functions[]
 *
 * Every user visible function must have an entry in mathstat_functions[].
 */
const zend_function_entry mathstat_functions[] = {
        PHP_FE(ms_factorial, 	arginfo_ms_factorial)
        PHP_FE(ms_median,   NULL)
	PHP_FE(ms_minimum,  NULL)
        PHP_FE(ms_maximal,   NULL)
        PHP_FE(ms_sort, NULL)
        PHP_FE(ms_unique, NULL)
        PHP_FE(ms_first_quartile, NULL)
        PHP_FE(ms_third_quartile, NULL)
	PHP_FE_END	/* Must be the last line in mathstat_functions[] */
};
/* }}} */

/* {{{ mathstat_module_entry
 */
zend_module_entry mathstat_module_entry = {
	STANDARD_MODULE_HEADER,
	"mathstat",  
	mathstat_functions,
	PHP_MINIT(mathstat),
	PHP_MSHUTDOWN(mathstat),
	PHP_RINIT(mathstat),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(mathstat),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(mathstat),
	PHP_MATHSTAT_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_MATHSTAT
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(mathstat)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
