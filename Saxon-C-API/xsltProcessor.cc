#include "saxonProcessor.h"
#include "php_saxon.h"



JNIEnv *env;
HANDLE myDllHandle;
JavaVM *jvm;


/*
 * Load dll.
 */
HANDLE loadDll(char* name)
{
    HANDLE hDll = LoadLibrary (name);

    if (!hDll) {
        printf ("Unable to load %s\n", name);
        exit(1);
    }

    printf ("%s loaded\n", name);

    return hDll;
}

extern "C" {
jint (JNICALL * JNI_GetDefaultJavaVMInitArgs_func) (void *args);
jint (JNICALL * JNI_CreateJavaVM_func) (JavaVM **pvm, void **penv, void *args);
}

/*
 * Initialize JET run-time.
 */
extern "C" void initJavaRT(HANDLE myDllHandle, JavaVM** pjvm, JNIEnv** penv)
{
    int            result;
    JavaVMInitArgs args;

    JNI_GetDefaultJavaVMInitArgs_func = 
             (jint (JNICALL *) (void *args))
             GetProcAddress (myDllHandle, "JNI_GetDefaultJavaVMInitArgs");

    JNI_CreateJavaVM_func =
             (jint (JNICALL *) (JavaVM **pvm, void **penv, void *args))
             GetProcAddress (myDllHandle, "JNI_CreateJavaVM");

    if(!JNI_GetDefaultJavaVMInitArgs_func) {
        printf ("%s doesn't contain public JNI_GetDefaultJavaVMInitArgs\n", dllname);
        exit (1);
    }

    if(!JNI_CreateJavaVM_func) {
        printf ("%s doesn't contain public JNI_CreateJavaVM\n", dllname);
        exit (1);
    }

    memset (&args, 0, sizeof(args));

    args.version = JNI_VERSION_1_2;
    result = JNI_GetDefaultJavaVMInitArgs_func(&args);
    if (result != JNI_OK) {
        printf ("JNI_GetDefaultJavaVMInitArgs() failed with result %d\n", result);
        exit(1);
    }
  
    /*
     * NOTE: no JVM is actually created
     * this call to JNI_CreateJavaVM is intended for JET RT initialization
     */
    result = JNI_CreateJavaVM_func (pjvm, (void **)penv, &args);
    if (result != JNI_OK) {
        printf ("JNI_CreateJavaVM() failed with result %d\n", result);
        exit(1);
    }

    printf ("JET RT initialized\n");
    fflush (stdout);
}

/*
 * Look for class.
 */

jclass lookForClass (JNIEnv* env, const char* name)
{
    jclass clazz = (jclass)env->FindClass (name);

    if (!clazz) {
        printf("Unable to find class %s\n", name);
        exit(1);
    }

    printf ("Class %s found\n", name);
    fflush (stdout);

    return clazz;
}

jmethodID findConstructor (JNIEnv* env, jclass myClassInDll, string arguments)
{
    jmethodID MID_init, mID;
    jobject obj;

    MID_init = (jmethodID)env->GetMethodID (myClassInDll, "<init>", arguments.c_str());
    if (!MID_init) {
        printf("Error: MyClassInDll.<init>() not found\n");
        return NULL;
    }

  return MID_init;
}

jobject createObject (JNIEnv* env, jclass myClassInDll, string arguments, bool l)
{
    jmethodID MID_init, mID;
    jobject obj;

    MID_init = (jmethodID)env->GetMethodID (myClassInDll, "<init>", arguments.c_str());
    if (!MID_init) {
        printf("Error: MyClassInDll.<init>() not found\n");
        return NULL;
    }

      obj = (jobject)env->NewObject(myClassInDll, MID_init, (jboolean)l);
      if (!obj) {
        printf("Error: failed to allocate an object\n");
        return NULL;
      }
    return obj;
}

SaxonApiException * checkForException(JNIEnv* env, jclass callingClass,  jobject callingObject){

    if (env->ExceptionCheck()) {
	string result1 = "";
	string errorCode = "";
	jthrowable exc = env->ExceptionOccurred();
	//env->ExceptionDescribe();
	 jclass exccls(env->GetObjectClass(exc));
        jclass clscls(env->FindClass("java/lang/Class"));

        jmethodID getName(env->GetMethodID(clscls, "getName", "()Ljava/lang/String;"));
        jstring name(static_cast<jstring>(env->CallObjectMethod(exccls, getName)));
        char const* utfName(env->GetStringUTFChars(name, 0));
	result1 = (string(utfName));
	//env->ReleaseStringUTFChars(name, utfName);

	 jmethodID  getMessage(env->GetMethodID(exccls, "getMessage", "()Ljava/lang/String;"));
	if(getMessage) {

		jstring message(static_cast<jstring>(env->CallObjectMethod(exc, getMessage)));
        	char const* utfMessage(env->GetStringUTFChars(message, 0));
		if(utfMessage != NULL) {
			result1 = (result1 + " : ") + utfMessage;
		}
		//cout<<result1<<endl;
		//env->ReleaseStringUTFChars(message,utfMessage);
		if(result1.compare(0,36, "net.sf.saxon.s9api.SaxonApiException", 36) == 0){
			jmethodID  getErrorCodeID(env->GetMethodID(callingClass, "getExceptions", "()[Lnet/sf/saxon/option/cpp/SaxonExceptionForCpp;"));
			jclass saxonExceptionClass(env->FindClass("net/sf/saxon/option/cpp/SaxonExceptionForCpp"));
				if(getErrorCodeID){	
					jobjectArray saxonExceptionObject((jobjectArray)(env->CallObjectMethod(callingObject, getErrorCodeID)));
					if(saxonExceptionObject) {
						jmethodID lineNumID = env->GetMethodID(saxonExceptionClass, "getLinenumber", "()I");
						jmethodID ecID = env->GetMethodID(saxonExceptionClass, "getErrorCode", "()Ljava/lang/String;");
						jmethodID emID = env->GetMethodID(saxonExceptionClass, "getErrorMessage", "()Ljava/lang/String;");
						jmethodID typeID = env->GetMethodID(saxonExceptionClass, "isTypeError", "()Z");
						jmethodID staticID = env->GetMethodID(saxonExceptionClass, "isStaticError", "()Z");
						jmethodID globalID = env->GetMethodID(saxonExceptionClass, "isGlobalError", "()Z");


						int exLength = (int)env->GetArrayLength(saxonExceptionObject);
						SaxonApiException * saxonExceptions = new SaxonApiException();
						for(int i=0; i<exLength;i++){
							jobject exObj = env->GetObjectArrayElement(saxonExceptionObject, i);

							jstring errCode = (jstring)(env->CallObjectMethod(exObj, ecID));
							jstring errMessage = (jstring)(env->CallObjectMethod(exObj, emID));
							jboolean isType = (env->CallBooleanMethod(exObj, typeID));
							jboolean isStatic = (env->CallBooleanMethod(exObj, staticID));
							jboolean isGlobal = (env->CallBooleanMethod(exObj, globalID));
							saxonExceptions->add((errCode ? env->GetStringUTFChars(errCode,0) : NULL )  ,(errMessage ? env->GetStringUTFChars(errMessage,0) : NULL),(int)(env->CallIntMethod(exObj, lineNumID)), (bool)isType, (bool)isStatic, (bool)isGlobal);
							env->ExceptionDescribe();
						}
						//env->ExceptionDescribe();
						env->ExceptionClear();
						return saxonExceptions;
					}
				}
		}
	}
	SaxonApiException * saxonExceptions = new SaxonApiException(NULL, result1.c_str());
	//env->ExceptionDescribe();
	env->ExceptionClear();
	return saxonExceptions;
     }
	return NULL;

}


XsltProcessor::XsltProcessor(bool license) {
   /*
     * First of all, load required component.
     * By the time of JET initialization, all components should be loaded.
     */
    myDllHandle = loadDll (dllname);

    /*
     * Initialize JET run-time.
     * The handle of loaded component is used to retrieve Invocation API.
     */
    initJavaRT (myDllHandle, &jvm, &env);

    /*
     * Look for class.
     */
     cppClass = lookForClass(env, "net/sf/saxon/option/cpp/XsltProcessorForCpp");
     versionClass = lookForClass(env, "net/sf/saxon/Version");

    cpp = createObject (env, cppClass, "(Z)V", license);
    jmethodID debugMID = env->GetStaticMethodID(cppClass, "setDebugMode", "(Z)V");
    if(debugMID){
	env->CallStaticVoidMethod(cppClass, debugMID, (jboolean)false);
    }
   cout<<"check 1 : "<<license<<endl;
    nodeCreated = false;
    exception = NULL;
    outputfile1 = "";

}

void XsltProcessor::close(){

}



const char * XsltProcessor::getErrorCode(int i) {
	if(exception == NULL) {return NULL;}
	return exception->getErrorCode(i);
}

const char * XsltProcessor::version() {


    jmethodID MID_foo;

    string methodName = "getProductVersion";
    string args = "()Ljava/lang/String;";
    MID_foo = (jmethodID)env->GetStaticMethodID(versionClass, methodName.c_str(), args.c_str());
    if (!MID_foo) {
	cout<<"\nError: MyClassInDll "<<methodName<<"()"<<" not found"<<endl;
        return NULL;
    }
   jstring jstr = (jstring)(env->CallStaticObjectMethod(versionClass, MID_foo));
   const char * str = env->GetStringUTFChars(jstr, NULL);
  
//    env->ReleaseStringUTFChars(jstr,str);
	return str;
}


void XsltProcessor::setSourceValue(XdmValue * node){
   xdmNode = node->getUnderlyingValue();    	
}


void XsltProcessor::setOutputfile(const char * ofile){
   outputfile1 = string(ofile); 
   setProperty("o", ofile);
}

XdmValue * XsltProcessor::parseString(string source){

    string methodName = "xmlparseStringing";
    string args = "(Ljava/lang/String;)Lnet/sf/saxon/s9api/XdmNode;";
    jmethodID mID = (jmethodID)env->GetMethodID(cppClass, methodName.c_str(), args.c_str());
    if (!mID) {
	cout<<"\nError: MyClassInDll "<<methodName<<"()"<<" not found"<<endl;
        return NULL;
    }
   jobject xdmNodei = env->CallObjectMethod(cpp, mID, env->NewStringUTF(source.c_str()));
     if(exceptionOccurred()) {
	   exception= checkForException(env, cppClass, cpp);
     } else {
	XdmValue * value = new XdmValue(xdmNodei);
	return value;
   }
   return NULL;
}

void XsltProcessor::setParameter(string namespacei, string name, XdmValue * value){
	if(value != NULL){
		parameters[name] = value;
	}
}


XdmValue* XsltProcessor::getParameter(string namespacei, string name){
	return parameters[name];
}


bool XsltProcessor::removeParameter(string namespacei, string name){
	return (bool)(parameters.erase(name));
}


void XsltProcessor::setProperty(string name, string value){
	properties[name] = value;	

}

void XsltProcessor::clearSettings(){
	properties.clear();
	parameters.clear();
        outputfile1.clear();
}

void XsltProcessor::exceptionClear(){
	if(exception != NULL) {
		delete exception;
		exception = NULL;	
	}
}

SaxonApiException* XsltProcessor::checkException(){
	if(exception == NULL) {
		exception = checkForException(env, cppClass, cpp);
	}
        return exception;
}

bool XsltProcessor::exceptionOccurred(){
	return env->ExceptionCheck();
}

int XsltProcessor::exceptionCount(){
	if(exception != NULL){
		return exception->count();
	}
	return 0;
}

const char * XsltProcessor::xsltApplyStylesheet1(string stylesheet){
 jmethodID mID = (jmethodID)env->GetMethodID (cppClass,"xsltApplyStylesheet", "(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/String;");
 if (!mID) {
        cout<<"Error: MyClassInDll."<<"xsltApplyStylesheet"<<" not found\n"<<endl;
    } else {
	jobjectArray stringArray = NULL;
	jobjectArray objectArray = NULL;
	int size = parameters.size() + properties.size();
		
	if(size >0) {
	   int i=0;
           jclass objectClass = lookForClass(env, "java/lang/Object");
	   jclass stringClass = lookForClass(env, "java/lang/String");
	   objectArray = env->NewObjectArray( (jint)size, objectClass, 0 );
	   stringArray = env->NewObjectArray( (jint)size, stringClass, 0 );
	   if((!objectArray) || (!stringArray)) { return NULL;}
	   if(xdmNode) {
		size +=1;	
	        env->SetObjectArrayElement( stringArray, i, env->NewStringUTF("s") );
     	        env->SetObjectArrayElement( objectArray, i, (jobject)(xdmNode) );
		i++;
		
	   }
	   for(map<string, XdmValue* >::iterator iter=parameters.begin(); iter!=parameters.end(); ++iter, i++) {
	     env->SetObjectArrayElement( stringArray, i, env->NewStringUTF( (string("param:")+iter->first).c_str() ) );
		bool checkCast = env->IsInstanceOf((iter->second)->getUnderlyingValue(), lookForClass(env, "net/sf/saxon/option/cpp/XdmValueForCpp") );
		if(( (bool)checkCast)==false ){
			failure = "FAILURE in  array of XdmValueForCpp";
		} 
	     env->SetObjectArrayElement( objectArray, i, (jobject)((iter->second)->getUnderlyingValue()) );
	   }
  	   for(map<string, string >::iterator iter=properties.begin(); iter!=properties.end(); ++iter, i++) {
	     env->SetObjectArrayElement( stringArray, i, env->NewStringUTF( (iter->first).c_str()  ));
	     env->SetObjectArrayElement( objectArray, i, (jobject)(env->NewStringUTF((iter->second).c_str())) );
	   }
	}

	  jstring result = (jstring)(env->CallObjectMethod(cpp, mID, NULL, env->NewStringUTF(stylesheet.c_str()), stringArray, objectArray ));
	  env->DeleteLocalRef(objectArray);
	  env->DeleteLocalRef(stringArray);
	  if(result) {
             const char * str = env->GetStringUTFChars(result, NULL);
            //return "result should be ok";            
	    return str;
	   }
  }
  return NULL;
  
}	 

void XsltProcessor::xsltSaveResultToFile(string source, string stylesheet, string outputfile) {

 jmethodID mID = (jmethodID)env->GetMethodID (cppClass,"xsltSaveResultToFile", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)V");
 exception = NULL;
 if (!mID) {
        cout<<"Error: MyClassInDll."<<"xsltApplyStylesheet"<<" not found\n"<<endl;

    } else {
 	jobjectArray stringArray = NULL;
	jobjectArray objectArray = NULL;
	int size = parameters.size() + properties.size();
	if(xdmNode) {
		size +=1;	
	}
	if(size >0) {
           jclass objectClass = lookForClass(env, "java/lang/Object");
	   jclass stringClass = lookForClass(env, "java/lang/String");
	   objectArray = env->NewObjectArray( (jint)size, objectClass, 0 );
	   stringArray = env->NewObjectArray( (jint)size, stringClass, 0 );
	   if((!objectArray) || (!stringArray)) { return;}
	   int i=0;
	   for(map<string, XdmValue* >::iterator iter=parameters.begin(); iter!=parameters.end(); ++iter, i++) {
	     env->SetObjectArrayElement( stringArray, i, env->NewStringUTF( (string("param:")+iter->first).c_str() ) );
	     env->SetObjectArrayElement( objectArray, i, (iter->second)->getUnderlyingValue() );
	   }
  	   for(map<string, string >::iterator iter=properties.begin(); iter!=properties.end(); ++iter, i++) {
	     env->SetObjectArrayElement( stringArray, i, env->NewStringUTF( (iter->first).c_str()  ));
	     env->SetObjectArrayElement( objectArray, i, env->NewStringUTF((iter->second).c_str()) );
	   }
	}
      env->CallObjectMethod(cpp, mID, env->NewStringUTF(source.c_str()), env->NewStringUTF(stylesheet.c_str()), env->NewStringUTF(outputfile.c_str()), stringArray, objectArray );     
  }
}

const char * XsltProcessor::xsltApplyStylesheet(string source, string stylesheet) {
 jmethodID mID = (jmethodID)env->GetMethodID (cppClass,"xsltApplyStylesheet", "(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/String;");
 if (!mID) {
        cout<<"Error: MyClassInDll."<<"xsltApplyStylesheet"<<" not found\n"<<endl;

    } else {
 	jobjectArray stringArray = NULL;
	jobjectArray objectArray = NULL;
	int size = parameters.size() + properties.size();

	if(size >0) {
           jclass objectClass = lookForClass(env, "java/lang/Object");
	   jclass stringClass = lookForClass(env, "java/lang/String");
	   objectArray = env->NewObjectArray( (jint)size, objectClass, 0 );
	   stringArray = env->NewObjectArray( (jint)size, stringClass, 0 );
	   if((!objectArray) || (!stringArray)) { return NULL;}
	   int i=0;
	   for(map<string, XdmValue* >::iterator iter=parameters.begin(); iter!=parameters.end(); ++iter, i++) {
	     env->SetObjectArrayElement( stringArray, i, env->NewStringUTF( (string("param:")+iter->first).c_str() ) );
	     env->SetObjectArrayElement( objectArray, i, (iter->second)->getUnderlyingValue() );
	   }
  	   for(map<string, string >::iterator iter=properties.begin(); iter!=properties.end(); ++iter, i++) {
	     env->SetObjectArrayElement( stringArray, i, env->NewStringUTF( (iter->first).c_str()  ));
	     env->SetObjectArrayElement( objectArray, i, env->NewStringUTF((iter->second).c_str()) );
	   }
	}
      jstring result = (jstring)(env->CallObjectMethod(cpp, mID, env->NewStringUTF(source.c_str()), env->NewStringUTF(stylesheet.c_str()), stringArray, objectArray ));
      if(result) {
       const char * str = env->GetStringUTFChars(result, NULL);
       //return "result should be ok";            
	return str;
     }
  }
  return NULL;
}

    const char * XsltProcessor::getErrorMessage(int i ){
	if(exception == NULL) {return NULL;}
	return exception->getErrorMessage(i);
    }




/*
* XdmNode Class implementation
*/

        XdmValue::XdmValue(){
	 if(env == NULL) {
           myDllHandle = loadDll (dllname);
          initJavaRT (myDllHandle, &jvm, &env);
	 }
	   xdmValueClass = lookForClass(env, "net/sf/saxon/option/cpp/XdmValueForCpp");
	   jmethodID MID_init = findConstructor (env, xdmValueClass, "()V");
/* 	   xdmValue = (jobject)env->NewObject(xdmValueClass, MID_init);
      		if (!xdmValue) {
	        	printf("Error: failed to allocate an object\n");
        		return;
      		}*/

	
	 xdmValue = NULL;
	}


	XdmValue::XdmValue(bool b){ 
	 if(env == NULL) {
           myDllHandle = loadDll (dllname);
          initJavaRT (myDllHandle, &jvm, &env);
	 }
		xdmValueClass = lookForClass(env, "net/sf/saxon/option/cpp/XdmValueForCpp");
	        jmethodID MID_init = findConstructor (env, xdmValueClass, "(Z)V");
 		xdmValue = (jobject)env->NewObject(xdmValueClass, MID_init, (jboolean)b);
      		if (!xdmValue) {
	        	printf("Error: failed to allocate an object\n");
        		return;
      		}
	}

	XdmValue::XdmValue(int i){ 
	 if(env == NULL) {
           myDllHandle = loadDll (dllname);
          initJavaRT (myDllHandle, &jvm, &env);
	 }
		xdmValueClass = lookForClass(env, "net/sf/saxon/option/cpp/XdmValueForCpp");
	        jmethodID MID_init = findConstructor (env, xdmValueClass, "(I)V");
 		xdmValue = (jobject)env->NewObject(xdmValueClass, MID_init, (jint)i);
      		if (!xdmValue) {
	        	printf("Error: failed to allocate an XdmValueForCpp object \n");
        		return;
      		}
	}


	XdmValue::XdmValue(string type, string str){ 
	 if(env == NULL) {
           myDllHandle = loadDll (dllname);
          initJavaRT (myDllHandle, &jvm, &env);
	 }
		xdmValueClass = lookForClass(env, "net/sf/saxon/option/cpp/XdmValueForCpp");
	        jmethodID MID_init = findConstructor (env, xdmValueClass, "(Ljava/lang/String;Ljava/lang/String;)V");
 		xdmValue = (jobject)env->NewObject(xdmValueClass, MID_init, env->NewStringUTF(type.c_str()), env->NewStringUTF(str.c_str()));
      		if (!xdmValue) {
	        	printf("Error: failed to allocate an XdmValueForCpp object \n");
        		return;
      		}
	}

	XdmValue::XdmValue(jobject xdmNode){ 
	 if(env == NULL) {
           myDllHandle = loadDll (dllname);
          initJavaRT (myDllHandle, &jvm, &env);
	 }
		xdmValueClass = lookForClass(env, "net/sf/saxon/option/cpp/XdmValueForCpp");
	        jmethodID MID_init = findConstructor (env, xdmValueClass, "(Lnet/sf/saxon/s9api/XdmNode;)V");
		if (!MID_init) {
	        	cout<<"Error: MyClassInDll."<<"XdmValueForCpp with XdmNode"<<" not found\n"<<endl;
			failure = "Error: MyClassInDll. XdmValueForCpp with XdmNode  not found\n";
			return;
		}		
 		xdmValue = (jobject)env->NewObject(xdmValueClass, MID_init, xdmNode);
      		if (!xdmValue) {
	        	printf("Error: failed to allocate an XdmValueForCpp object with XdmNode \n");
			failure = "Error: failed to allocate an XdmValueForCpp object with XdmNode \n";
        		return;
      		}
	}

    const char * XdmValue::getErrorMessage(int i){
	if(exception== NULL) return NULL;
	return exception->getErrorMessage(i);
    }

    const char * XdmValue::getErrorCode(int i) {
	if(exception== NULL) return NULL;
	return exception->getErrorCode(i);
     }

	int XdmValue::exceptionCount(){
		return exception->count();
	}

	const char * XdmValue::getStringValue(){
          jmethodID mID;

          string methodName = "getStringValue";
          string args = "()Ljava/lang/String;";
          mID = (jmethodID)(env)->GetMethodID(xdmValueClass, methodName.c_str(), args.c_str());
          if (!mID) {
	    cout<<"\nError: MyClassInDll "<<methodName<<"()"<<" not found"<<endl;
            return string("ERROR: Method Not Found: "+methodName).c_str();
         }
	   jstring valueStr = (jstring)(env)->CallObjectMethod(xdmValue, mID);
	   if(valueStr){
		  const char * str = env->GetStringUTFChars(valueStr, NULL);
	       //return "result should be ok";            
		return str;
	   }
		return NULL;
	}



int main(int argc, char *argv[]) {
	SaxonProcessor *processor = new SaxonProcessor();
	XsltProcessor * test = processor->newTransformer();
	cout<<"Hello World version:"<<test->version()<<endl;
	SaxonApiException * ex = test->checkException();
	if(ex != NULL){
		cout<<ex->count()<<endl;	
	}
	//test->setSourceAsString("<node>test</node>");
	const char * result = test->xsltApplyStylesheet("xmark64.xml","q8.xsl");//test->xsltApplyStylesheet("cat.xml","test.xsl");
	if(result != NULL){ //2m 25sec
		cout<<"Output: "<<result<<endl;
	} else {
	cout<<"result is null"<<endl;	
	}
//	cout<<"Test output: "<<test->xsltApplyStylesheet("cat.xml","test.xsl")<<endl;
	//cout<<"Test output: "<<test->xsltApplyStylesheet("xmark100.xml","q8.xsl")<<endl;
//	cout<<"ErrorCode:"<<test->getErrorCode()<<endl;
SaxonApiException * ex1 = test->checkException();
	if(ex != NULL){
		cout<<ex->count()<<endl;	
	}
	cout<<"Hello World"<<endl;
}

