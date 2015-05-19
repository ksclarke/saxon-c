#ifndef PHP_SAXON_H
#define PHP_SAXON_H

#define PHP_SAXON_EXTNAME  "saxon"
#define PHP_SAXON_EXTVER   "9.5"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif 

extern "C" {
#include "php.h"
#include "zend_exceptions.h"
}

extern zend_module_entry saxon_module_entry;
#define phpext_saxon_ptr &saxon_module_entry;

zend_object_handlers XsltProcessor_object_handlers;
zend_object_handlers xdmValue_object_handlers;



struct XsltProcessor_object {
    zend_object std;
    XsltProcessor *xsltProcessor;
};

struct xdmValue_object {
    zend_object std;
    XdmValue * xdmValue;
};


#endif /* PHP_SAXON_H */


zend_class_entry *xsltProcessor_ce;
zend_class_entry *xdmValue_ce;

void XsltProcessor_free_storage(void *object TSRMLS_DC)
{
    XsltProcessor_object *obj = (XsltProcessor_object *)object;
    delete obj->xsltProcessor; 

    zend_hash_destroy(obj->std.properties);
    FREE_HASHTABLE(obj->std.properties);

    efree(obj);
}


zend_object_value XsltProcessor_create_handler(zend_class_entry *type TSRMLS_DC)
{
    zval *tmp;
    zend_object_value retval;

    XsltProcessor_object *obj = (XsltProcessor_object *)emalloc(sizeof(XsltProcessor_object));
    memset(obj, 0, sizeof(XsltProcessor_object));
    obj->std.ce = type;

    ALLOC_HASHTABLE(obj->std.properties);
    zend_hash_init(obj->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
//#if PHP_VERSION_ID < 50399
  //  zend_hash_copy(obj->std.properties, &type->default_properties,
  //      (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
//#else
    object_properties_init(&obj->std, type);
//#endif
// zend_hash_copy(obj->std.properties, &type->default_properties,
  //      (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));

    retval.handle = zend_objects_store_put(obj, NULL,
        XsltProcessor_free_storage, NULL TSRMLS_CC);
    retval.handlers = &XsltProcessor_object_handlers;

    return retval;
}






PHP_METHOD(XsltProcessor, __construct)
{
 	
    XsltProcessor *xsltProcessor = NULL;
    zval *object = getThis();


    xsltProcessor = new XsltProcessor(false);
    XsltProcessor_object *obj = (XsltProcessor_object *)zend_object_store_get_object(object TSRMLS_CC);
    obj->xsltProcessor = xsltProcessor;

}



PHP_METHOD(XsltProcessor, xsltSaveResultToFile)
{
   XsltProcessor *xsltProcessor;
   char * sourcefile;
   char * stylesheet;
   char * outputfile;
   int len1, len2, len3;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &sourcefile, &len1, &stylesheet, &len2, &outputfile, &len3) == FAILURE) {
        RETURN_NULL();
    }

    XsltProcessor_object *obj = (XsltProcessor_object *)zend_object_store_get_object(
        getThis() TSRMLS_CC);
    xsltProcessor = obj->xsltProcessor;
    if (xsltProcessor != NULL) {
     xsltProcessor->xsltSaveResultToFile(sourcefile, stylesheet, outputfile);
     if(xsltProcessor->exceptionOccurred()) {
     	//throw exception
     } 	
    }

}
PHP_METHOD(XsltProcessor, xsltApplyStylesheet)
{

   XsltProcessor *xsltProcessor;
   char * sourcefile;
   char * stylesheet;
   int len1, len2, myint;	

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &sourcefile, &len1, &stylesheet, &len2) == FAILURE) {
        RETURN_NULL();
    }
    XsltProcessor_object *obj = (XsltProcessor_object *)zend_object_store_get_object(
        getThis() TSRMLS_CC);
    xsltProcessor = obj->xsltProcessor;
    if (xsltProcessor != NULL) {

	php_error(E_WARNING, "before xsltApplyStylesheet");

	const char * result = xsltProcessor->xsltApplyStylesheet(sourcefile, stylesheet);
	php_error(E_WARNING, "after xsltApplyStylesheet");

	xsltProcessor->checkException();
	if(result != NULL ) {
	php_error(E_WARNING, result);
   	  char *str = estrdup(result);
          RETURN_STRING(str, 0);
       } else {
	php_error(E_WARNING, "Error message");
		const char * errStr  = xsltProcessor->getErrorCode(0);
		php_error(E_WARNING, errStr);
	}
    }
   RETURN_NULL();
}
PHP_METHOD(XsltProcessor, xsltApplyStylesheet1)
{

   XsltProcessor *xsltProcessor;
   char * stylesheet;
   int len1;	

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &stylesheet, &len1) == FAILURE) {
        RETURN_NULL();
    }
    XsltProcessor_object *obj = (XsltProcessor_object *)zend_object_store_get_object(
        getThis() TSRMLS_CC);
    xsltProcessor = obj->xsltProcessor;
    if (xsltProcessor != NULL) {
	/*string failures = "xsltApplyStylesheet failures:="+string(xsltProcessor->checkFailures());
	php_error(E_WARNING,failures.c_str());*/
	const char * result = xsltProcessor->xsltApplyStylesheet1(stylesheet);
	if(result == NULL ) {
		php_error(E_WARNING, "result is null");
	} else {
	php_error(E_WARNING, result);
	}
	if(result != NULL) {
   	  char *str = estrdup(result);
          RETURN_STRING(str, 0);
       } else {
	  xsltProcessor->checkException();
	  const char * errStr = xsltProcessor->getErrorMessage(0);
	  if(errStr != NULL) {
	php_error(E_WARNING, errStr);
	      const char * errorCode = xsltProcessor->getErrorCode(0);	
	      if(errorCode!=NULL){
		php_error(E_WARNING, errorCode);
//		RETURN_STRING(estrdup(errorCode),0);	
	      }	
//	     RETURN_STRING(estrdup(errStr),0);
          } 	
	}
    }
   RETURN_NULL();
}


PHP_METHOD(XsltProcessor, setProperty)
{

   XsltProcessor *xsltProcessor;
   char * name;
   char * value;
   int len1, len2, myint;	

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &name, &len1, &value, &len2) == FAILURE) {
        RETURN_NULL();
    }
    XsltProcessor_object *obj = (XsltProcessor_object *)zend_object_store_get_object(
        getThis() TSRMLS_CC);
    xsltProcessor = obj->xsltProcessor;
    if (xsltProcessor != NULL) {
	xsltProcessor->setProperty(name, value);
	if(xsltProcessor->exceptionOccurred()) {
	//throw exception
   	}
    }
}

PHP_METHOD(XsltProcessor, setParameter)
{

   XsltProcessor *xsltProcessor;
   char * namespacei;
   char * name;
   zval* oth;
   int len1, len2, len3, myint;	

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssO", &namespacei, &len1, &name, &len2, &oth, xdmValue_ce) == FAILURE) {
        RETURN_NULL();
    }
    XsltProcessor_object *obj = (XsltProcessor_object *)zend_object_store_get_object(
        getThis() TSRMLS_CC);
    xsltProcessor = obj->xsltProcessor;
    if (xsltProcessor != NULL) {
	xdmValue_object* ooth = (xdmValue_object*)zend_object_store_get_object(oth TSRMLS_CC);
	if(ooth != NULL){
		XdmValue * value = ooth->xdmValue;
		if(value != NULL)
		xsltProcessor->setParameter("", string(name), ooth->xdmValue);
	}
	
	/*const char * errStr = xsltProcessor->getErrorMessage();
	if(errStr!=NULL) {
	//throw exception
   	}*/
    }
}


PHP_METHOD(XsltProcessor, setSourceValue)
{

   XsltProcessor *xsltProcessor;
   zval* oth;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", xdmValue_ce) == FAILURE) {
        RETURN_NULL();
    }
    XsltProcessor_object *obj = (XsltProcessor_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    xsltProcessor = obj->xsltProcessor;
    if (xsltProcessor != NULL) {
	xdmValue_object* ooth = (xdmValue_object*)zend_object_store_get_object(oth TSRMLS_CC);
	if(ooth != NULL){
	  XdmValue * value = ooth->xdmValue;
	  if(value != NULL)
	    xsltProcessor->setSourceValue(value);
	} 
    }
}

PHP_METHOD(XsltProcessor, parseString)
{

   XsltProcessor *xsltProcessor;
   char * source;
   int len1;	

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &source, &len1) == FAILURE) {
        RETURN_NULL();
    }
    XsltProcessor_object *obj = (XsltProcessor_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    assert (obj != NULL);
    xsltProcessor = obj->xsltProcessor;
    if (xsltProcessor != NULL) {
	XdmValue* node = xsltProcessor->parseString(source);

//	php_error(E_WARNING, " create XdmValue");E_ERROR
       if(node != NULL) {
	php_error(E_WARNING,"parseString not null");
	  if (object_init_ex(return_value, xdmValue_ce) != SUCCESS) {
		php_error(E_WARNING,"parseString reported as null");
              	RETURN_NULL();
	  } else {
	    struct xdmValue_object* vobj = (struct xdmValue_object *)zend_object_store_get_object(return_value TSRMLS_CC);
            assert (vobj != NULL); 
            vobj->xdmValue = node;
	 }
      } else {
		php_error(E_WARNING,"parseString is NULL");
	xsltProcessor->checkException();
	}
    } else {
	php_error(E_WARNING,"parseString reported as null");
    RETURN_NULL();
}
	
    
}

PHP_METHOD(XsltProcessor, version)
{

   XsltProcessor *xsltProcessor;

    XsltProcessor_object *obj = (XsltProcessor_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    xsltProcessor = obj->xsltProcessor;
    if (xsltProcessor != NULL) {
	char *str = estrdup(xsltProcessor->version());
	RETURN_STRING(str, 0);
    }
   RETURN_NULL();
}

PHP_METHOD(XsltProcessor, getExceptionCount)
{

   XsltProcessor *xsltProcessor;

    XsltProcessor_object *obj = (XsltProcessor_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    xsltProcessor = obj->xsltProcessor;
    if (xsltProcessor != NULL) {
	int count = xsltProcessor->exceptionCount();
	   RETURN_LONG(count);
   }
   RETURN_LONG(0);
}

PHP_METHOD(XsltProcessor, getErrorCode)
{

   XsltProcessor *xsltProcessor;
   long index;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
        RETURN_NULL();
    }
    XsltProcessor_object *obj = (XsltProcessor_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    xsltProcessor = obj->xsltProcessor;
    if (xsltProcessor != NULL) {
	const char * errCode = xsltProcessor->getErrorCode((int)index);
	if(errCode != NULL) {
	  char *str = estrdup(errCode);
	  RETURN_STRING(str, 0);	
	}
    }
   RETURN_NULL();
}

PHP_METHOD(XsltProcessor, getErrorMessage)
{

   XsltProcessor *xsltProcessor;
   long index;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
        RETURN_NULL();
    }
    XsltProcessor_object *obj = (XsltProcessor_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    xsltProcessor = obj->xsltProcessor;
    if (xsltProcessor != NULL) {
	const char * errStr = xsltProcessor->getErrorMessage((int)index);
	if(errStr != NULL) {
	  char *str = estrdup(errStr);
	  RETURN_STRING(str, 0);	
	}
    }
   RETURN_NULL();
}


PHP_METHOD(XsltProcessor, close)
{

   XsltProcessor *xsltProcessor;

    XsltProcessor_object *obj = (XsltProcessor_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    xsltProcessor = obj->xsltProcessor;
    if (xsltProcessor != NULL) {
	xsltProcessor->close();
    }
}

PHP_METHOD(XsltProcessor, clear)
{

   XsltProcessor *xsltProcessor;

    XsltProcessor_object *obj = (XsltProcessor_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    xsltProcessor = obj->xsltProcessor;
    if (xsltProcessor != NULL) {
	xsltProcessor->clearSettings();
    }
}

PHP_METHOD(XsltProcessor, exceptionClear)
{

   XsltProcessor *xsltProcessor;

    XsltProcessor_object *obj = (XsltProcessor_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    xsltProcessor = obj->xsltProcessor;
    if (xsltProcessor != NULL) {
	xsltProcessor->exceptionClear();
    }
}



zend_function_entry XsltProcessor_methods[] = {
    PHP_ME(XsltProcessor,  __construct,     NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(XsltProcessor,  xsltSaveResultToFile,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(XsltProcessor,  xsltApplyStylesheet,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(XsltProcessor,  xsltApplyStylesheet1,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(XsltProcessor,  setSourceValue,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(XsltProcessor,  parseString,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(XsltProcessor,  setParameter,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(XsltProcessor,  setProperty,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(XsltProcessor,  clear,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(XsltProcessor,  exceptionClear,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(XsltProcessor,  close,      NULL, ZEND_ACC_PUBLIC)
//    PHP_ME(XsltProcessor,  checkException,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(XsltProcessor,  getErrorCode,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(XsltProcessor,  getErrorMessage,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(XsltProcessor,  getExceptionCount,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(XsltProcessor,  version,      NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};




/*     ============== PHP Interface of   XdmValue =============== */

void xdmValue_free_storage(void *object TSRMLS_DC)
{
    xdmValue_object *obj = (xdmValue_object *)object;
    delete obj->xdmValue; 

    zend_hash_destroy(obj->std.properties);
    FREE_HASHTABLE(obj->std.properties);

    efree(obj);
}

zend_object_value xdmValue_create_handler(zend_class_entry *type TSRMLS_DC)
{
    zval *tmp;
    zend_object_value retval;

    xdmValue_object *obj = (xdmValue_object *)emalloc(sizeof(xdmValue_object));
    memset(obj, 0, sizeof(xdmValue_object));
    obj->std.ce = type;

    ALLOC_HASHTABLE(obj->std.properties);
    zend_hash_init(obj->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
    object_properties_init(&obj->std, type);


    retval.handle = zend_objects_store_put(obj, NULL,
        xdmValue_free_storage, NULL TSRMLS_CC);
    retval.handlers = &xdmValue_object_handlers;

    return retval;
}

PHP_METHOD(XdmValue, __construct)
{
 	
    XdmValue *xdmValue = NULL;
    zval *object = getThis();
    bool bVal;
    char * sVal;
    int len;
    long iVal;
    double dVal;
     zval *zvalue; 
    xdmValue_object *obj;

 if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",&zvalue) == SUCCESS) { 
switch (Z_TYPE_P(zvalue)) {
       
        case IS_BOOL:
	  bVal = Z_BVAL_P(zvalue);
         xdmValue = new XdmValue(bVal);
         obj = (xdmValue_object *)zend_object_store_get_object(object TSRMLS_CC);
         obj->xdmValue = xdmValue;
	 break;
        case IS_LONG:
	     iVal = Z_LVAL_P(zvalue);
             xdmValue = new XdmValue((int)iVal);
             obj = (xdmValue_object *)zend_object_store_get_object(object TSRMLS_CC);
             obj->xdmValue = xdmValue;
	  break;
        case IS_STRING:
            sVal = Z_STRVAL_P(zvalue);
            len = Z_STRLEN_P(zvalue);
	    xdmValue = new XdmValue("string", string(sVal));
	    obj = (xdmValue_object *)zend_object_store_get_object(object TSRMLS_CC);
            obj->xdmValue = xdmValue;
            break;
	 case IS_NULL:
	    xdmValue = new XdmValue();
            obj = (xdmValue_object *)zend_object_store_get_object(object TSRMLS_CC);
       	    obj->xdmValue = xdmValue;
	    break;
        case IS_DOUBLE:
           //index = (long)Z_DVAL_P(zvalue);
            //break;
        case IS_ARRAY:
            //break;
        case IS_OBJECT:
            //break;
        default:
	  obj = NULL;
          zend_throw_exception(zend_exception_get_default(TSRMLS_C), "unknown type specified in XdmValue", 0 TSRMLS_CC);
    }

   } 
/*     if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &sVal, &len, &valuei) == SUCCESS) { 

      
php_error(E_WARNING,"STRING Value");
   } else if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &iVal) == SUCCESS) { 

php_error(E_WARNING,"Int value");
   }else if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &bVal) == SUCCESS) {
 
    } else {*/
       /*xdmValue = new XdmValue();
       xdmValue_object *obj = (xdmValue_object *)zend_object_store_get_object(object TSRMLS_CC);
       obj->xdmValue = xdmValue;*/
//zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 1, TSRMLS_C, "unknown field \"%s\"", key)

 //  }


}


PHP_METHOD(XdmValue, getErrorCode)
{

   XdmValue *xdmValue;
   long index;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
        RETURN_NULL();
    }
    xdmValue_object *obj = (xdmValue_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    xdmValue = obj->xdmValue;
    if (xdmValue != NULL) {
	const char * errCode = xdmValue->getErrorCode(index);
	if(errCode != NULL) {
	  char *str = estrdup(errCode);
	  RETURN_STRING(str, 0);	
	}
    }
   RETURN_NULL();
}



PHP_METHOD(XdmValue, getStringValue)
{

   XdmValue *xdmValue;

    xdmValue_object *obj = (xdmValue_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    xdmValue = obj->xdmValue;
    if (xdmValue != NULL) {
	const char * valueStr = xdmValue->getStringValue();
	if(valueStr != NULL) {
	php_error(E_WARNING,"getStringValue:");
	php_error(E_WARNING,valueStr);
	  char *str = estrdup(valueStr);
	  RETURN_STRING(str, 0);	
	}
    }
php_error(E_WARNING,"getStringValue is NULL");
   RETURN_NULL();
}



zend_function_entry xdmValue_methods[] = {
    PHP_ME(XdmValue,  __construct,     NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(XdmValue,  getErrorCode,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(XdmValue,  getStringValue,      NULL, ZEND_ACC_PUBLIC)
//    PHP_ME(XdmValue,  getSaxonErrorMessage,      NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

PHP_MINIT_FUNCTION(saxon)
{
zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Saxon\\XsltProcessor", XsltProcessor_methods);
    xsltProcessor_ce = zend_register_internal_class(&ce TSRMLS_CC);
    xsltProcessor_ce->create_object = XsltProcessor_create_handler;
    memcpy(&XsltProcessor_object_handlers,
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    XsltProcessor_object_handlers.clone_obj = NULL;


    INIT_CLASS_ENTRY(ce, "Saxon\\XdmValue", xdmValue_methods);
    xdmValue_ce = zend_register_internal_class(&ce TSRMLS_CC);
    xdmValue_ce->create_object = xdmValue_create_handler;
    memcpy(&xdmValue_object_handlers,
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    xdmValue_object_handlers.clone_obj = NULL;
    return SUCCESS;
    
}

zend_module_entry saxon_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_SAXON_EXTNAME,
    NULL,        /* Functions */
    PHP_MINIT(saxon),        /* MINIT */
    NULL,        /* MSHUTDOWN */
    NULL,        /* RINIT */
    NULL,        /* RSHUTDOWN */
    NULL,        /* MINFO */
#if ZEND_MODULE_API_NO >= 20010901
    PHP_SAXON_EXTVER,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SAXON
extern "C" {
ZEND_GET_MODULE(saxon)
}
#endif
