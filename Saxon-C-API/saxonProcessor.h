#ifndef SAXON_XSLT_H
#define SAXON_XSLT_H
#include <jni.h>

	
#ifdef __linux__
    #include <stdlib.h>
    #include <string.h>
    #include <dlfcn.h>

    #define HANDLE void*
    #define LoadLibrary(x) dlopen(x, RTLD_LAZY)
    #define GetProcAddress(x,y) dlsym(x,y)
#else
    #include <windows.h>
#endif


char dllname[] =
    #ifdef __linux__
        "/usr/lib/libsaxon.so";
    #else
        "Saxon-hec.dll";
    #endif

#include <string>
#include <iostream>
#include <sstream>  
#include <map>	
#include <vector>
using namespace std;
// The Saxon XSLT interface class


/*! <code>MyException</code>. This struct captures details of the Java exception thrown from Saxon s9api API (Java).
 * <p/>
 */
typedef struct {
		string errorCode;
		string errorMessage;
		int linenumber;
	    	bool isType;
	    	bool isStatic;
	    	bool isGlobal;
	}MyException;



/*! <code>SaxonApiException</code>. An exception thrown by the Saxon s9api API (Java). This is always a C++ wrapper for some other underlying exception in Java
 * <p/>
 */
class SaxonApiException {

public:

    /**
     * A default Constructor. Create a SaxonApiException
     */
     SaxonApiException(){}

    /**
     * A Copy constructor. Create a SaxonApiException
     * @param ex - The exception object to copy
     */
	SaxonApiException(const SaxonApiException &ex){
		exceptions = ex.exceptions;
	}

    /**
     * A constructor. Create a SaxonApiException
     * @param ec - The error code of the underlying exception thrown, if known
     * @param exM - The error message of the underlying exception thrown, if known
     */
	SaxonApiException(const char * ec, const char * exM){
		MyException newEx;	
		if(ec != NULL){
			newEx.errorCode =   string(ec);
		} else {
			newEx.errorCode ="Unknown";	
		}
		if(exM != NULL){
			newEx.errorMessage =  string(exM);
		} else {
			newEx.errorMessage="Unkown";		
		}
		newEx.isType = false;
	    	newEx.isStatic = false;
	    	newEx.isGlobal = false;
		newEx.linenumber = 0;
		exceptions.push_back(newEx);
	}

    /**
     * A constructor. Create a SaxonApiException
     * @param ec - The error code of the underlying exception thrown, if known
     * @param exM - The error message of the underlying exception thrown, if known
     * @param typeErr - Flag indicating if the error is a type error
     * @param stat - Flag indicating a static error
     * @param glob - Flag for if the error is global
     * @param l - Line number information of where the error occurred
     */
	SaxonApiException(const char * ec, const char * exM, bool typeErr, bool stat, bool glob, int l){
		MyException newEx;
		if(ec != NULL){
			newEx.errorCode =   string(ec);
		} else {
			newEx.errorCode ="ERROR1";	
		}
		if(exM != NULL){
			newEx.errorMessage =  string(exM);
		} else {
			newEx.errorMessage="ERROR2";		
		}
		newEx.isType = typeErr;
	    	newEx.isStatic = stat;
	    	newEx.isGlobal = glob;
		newEx.linenumber = l;
		exceptions.push_back(newEx);
	}

    /**
     * Creates a SaxonApiException and adds it to a vector of exceptions
     * @param ec - The error code of the underlying exception thrown, if known
     * @param exM - The error message of the underlying exception thrown, if known
     * @param typeErr - Flag indicating if the error is a type error
     * @param stat - Flag indicating a static error
     * @param glob - Flag for if the error is global
     * @param l - Line number information of where the error occurred
     */
	void add(const char * ec, const char * exM, bool typeErr, bool stat, bool glob, int l){
		MyException newEx;
		if(ec != NULL){
			newEx.errorCode =   string(ec);
		} else {
			newEx.errorCode ="ERROR1";	
		}
		if(exM != NULL){
			newEx.errorMessage =  string(exM);
		} else {
			newEx.errorMessage="ERROR2";		
		}
		newEx.isType = typeErr;
	    	newEx.isStatic = stat;
	    	newEx.isGlobal = glob;
		newEx.linenumber = l;
		exceptions.push_back(newEx);
	}


    /**
     * A destructor.
     */
	~SaxonApiException(){ 
	  exceptions.clear();
	}

    /**
     * Get the error code associated with the ith exception in the vector, if there is one
     * @param i - ith exception in the vector
     * @return the associated error code, or null if no error code is available
     */
	const char * getErrorCode(int i){
		if(i <= exceptions.size()){
			return exceptions[i].errorCode.c_str();
		}
		return NULL;
	}


	int getLineNumber(int i){
		if(i <= exceptions.size()){
			return exceptions[i].linenumber;	
		}
		return 0;
	}

	bool isGlobalError(int i){
		if(i <= exceptions.size()){
			return exceptions[i].isGlobal;
		}
		return false;
	}

	bool isStaticError(int i){
		if(i <= exceptions.size()){
			return exceptions[i].isStatic;
		}
		return false;
	}

	bool isTypeError(int i){
		if(i <= exceptions.size()){
			return exceptions[i].isType;
		}
		return NULL;
	}

	void clear(){
	  for(int i =0; i< exceptions.size();i++) {
		exceptions[i].errorCode.clear();
		exceptions[i].errorMessage.clear();	
	  }
	  exceptions.clear();
	}

	int count(){
		return exceptions.size();	
	}

    /**
     * Returns the detail message string of the ith throwable, if there is one
     * @param i - ith exception in the vector
     * @return the detail message string of this <tt>Throwable</tt> instance
     *         (which may be <tt>null</tt>).
     */
	const char * getErrorMessage(int i){
		if(i <= exceptions.size()){
			return exceptions[i].errorMessage.c_str();
		}
		return NULL;
	}

    /**
     * Returns the ith Exception added, if there is one
     * @param i - ith exception in the vector
     * @return MyException
     */
	MyException getException(int i){
		if(i <= exceptions.size()){
			return exceptions[i];	
		}
		throw 0;
	}

private:
	vector<MyException> exceptions; /*!< Capture exceptions in a std:vector */
};




//===============================================================================================

/*! <code>XdmValue</code>. Value in the XDM data model. A value is a sequence of zero or more items,
 * each item being either an atomic value or a node. This class is a wrapper of the the XdmValue object created in Java.
 * <p/>
 */

class XdmValue {
public:
/**
     * A default Constructor. Create a empty value
     */
	XdmValue();

    /**
     * A Constructor. Create an xs:boolean value
     * @param val - boolean value
     */
	XdmValue(bool val); 

    /**
     * A Constructor. Create an xs:string value
     * @param val - string value
     */
	XdmValue(string val);

    /**
     * A Constructor. Create an xs:int value
     * @param val - int value
     */
	XdmValue(int val); 

	//TODO XdmValue with constructor of sequence of values

    /**
     * A Constructor. Wrap an Java XdmValue object.
     * @param val - Java XdmValue object
     */
	XdmValue(jobject val);

    /**
     * A Constructor. Create a XdmValue based on the target type. Conversion is applied if possible
     * @param type - specify target type of the value  
     * @param val - Value to convert
     */
   	XdmValue(string type, string val); 	

    /**
     * Get Java XdmValue object.
     * @return jobject - The Java object of the XdmValue in its JNI representation
     */
	jobject getUnderlyingValue(){ return xdmValue;}

    /**
     * Get the string representation of the XdmValue.
     * @return char array
     */
	const char * getStringValue();
        const char * getErrorMessage(int i);
        const char * getErrorCode(int i);
	int exceptionCount();
	string checkFailures(){return failure;}

private:
	jobject xdmValue; /*!< Underlying Java object of the XdmValue */
	string valueStr;  /*!< String representation of the XdmValue, if available */
	jclass  xdmValueClass;
	SaxonApiException * exception;
	string failure;

};




/*! An <code>XsltProcessor</code> represents factory to compile, load and execute stylesheet.
 * IT is possible to cache the context and the stylesheet in the <code>XsltProcessor</code>.
 * <p/>
 */
class XsltProcessor {
public:

    //! A constructor.
    /*!
      license flag not required in Saxon-HE product
    */
    XsltProcessor(bool license=false);
   

    /**
     * Parse a lexical representation of the source document and return it as an XdmValue
    */
    XdmValue * parseString(string source);

    /**
     * Parse a source document file and return it as an XdmValue.
     * Not implemented yet. Available in next release
    */
    XdmValue * parseFile(string source);

    //XdmValue * parseURI(string source);

    /**
     * Set the source document from a XdmValue for the transformation.
    */
    void setSourceValue(XdmValue * value);

    /**
     * Set the source document from a XdmValue for the transformation.
    */
    void setOutputfile(const char* outfile);

    /**
     * Set the value of a stylesheet parameter
     *
     * @param namespacei currently not used
     * @param name  the name of the stylesheet parameter, as a string
     * @param value the value of the stylesheet parameter, or null to clear a previously set value
     */
    void setParameter(string namespacei, string name, XdmValue*value);

    /**
     * Get the value of a stylesheet parameter
     *
     * @param namespacei currently not used
     * @param name  the name of the stylesheet parameter, as a string
     */
    XdmValue * getParameter(string namespacei, string name);

    /**
     * Remove a parameter (name, value) pair from a stylesheet
     *
     * @param namespacei currently not used
     * @param name  the name of the stylesheet parameter
     * @return bool - outcome of the romoval
     */
    bool removeParameter(string namespacei, string name);

    /**
     * Set a property.
     *
     * @param name of the property
     * @param value of the property
     */
    void setProperty(string name, string value);

    /**
     * Clear parameter and property values set
     */
    void clearSettings();

    /**
     * Get the Saxon version
     * @return char array
     */	
    const char * version();

    /**
     * Perform a one shot transformation. The result is stored in the supplied outputfile.
     *
     * @param sourcefile - The file name of the source document
     * @param stylesheetfile - The file name of the stylesheet document
     * @param outputfile - The file name where results will be stored
     */
    void xsltSaveResultToFile(string sourcefile, string stylesheetfile, string outputfile); 

    /**
     * Perform a one shot transformation. The result is returned as a string
     *
     * @param sourcefile - The file name of the source document
     * @param stylesheetfile - The file name of the stylesheet document
     * @return char array - result of the transformation
     */
    const char * xsltApplyStylesheet(string sourcefile, string stylesheetfile);

    /**
     * Perform a one shot transformation. The result is returned as a string
     *
     * @param stylesheetfile - The file name of the stylesheet document
     * @return char array - result of the transformation
     */
    const char * xsltApplyStylesheet1(string stylesheet);

    /**
     * compile a stylesheet received as a file.
     *
     * Not implemented yet. Available in next release
     * @param stylesheetfile - name of the file
     */
    void compileFile(string stylesheetfile);

    /**
     * compile a stylesheet received as a string.
     *
     * Not implemented yet. Available in next release
     * @param stylesheet as a lexical string representation
     */
    void compileString(string stylesheet);

    /**
     * Perform the transformation returned as a string.
     * Not implemented yet. Available in next release
     *
     */
    const char * transformToString();

    /**
     * Perform the transformation returned as a XdmValue.
     * Not implemented yet. Available in next release
     *
     */
    XdmValue * transformToValue();

     /**
     * Set the initial context of the stylesheet.
     * Not implemented yet. Available in next release
    */
    void setIntialContext(XdmValue value);

    /**
    * close method not currently used. Available in next release
    */
    void close();

    bool exceptionOccurred();
    SaxonApiException* checkException();
    void exceptionClear();
    int exceptionCount();
    const char * getErrorMessage(int i);
    const char * getErrorCode(int i);
    int parameterSize(){return parameters.size();}	
    int propertiesSize(){return properties.size();}

    /**
    * Method for testing purposes. Available only in alpha release
    */
    string sizes(int number){
	std::ostringstream ostr; //output string stream
    	ostr << number;
	return ostr.str();
	}

    /**
    * Method for testing purposes. Available only in alpha release
    */
     string checkFailures(){return failure;}	//for testing

private:

	jclass  cppClass;
	jclass  versionClass;
	jobject cpp;
	jobject xdmNode; /*!< Underlying XdmValue Java object of the source document */
	string outputfile1; /*!< output file where result will be saved */
	string failure; //for testing
	SaxonApiException* exception; /*!< Pointer to any potential exception thrown */
	bool nodeCreated;
	std::map<string,XdmValue*> parameters; /*!< map of paramaters used for the transformation as (string, value) pairs */
	std::map<string,string> properties; /*!< map of properties used for the transformation as (string, string) pairs */
};



/*! An <code>SaxonProcessor</code> acts as a factory for generating XQuery, XPath, Schema and XSLT compilers
 * In this alpha release only the XSLT compiler is available
 * <p/>
 */
class SaxonProcessor {
public:
   //! A constructor.
    /*!
      * Create Saxon Processor.
      * @param l - Flag that a license is to be used. Default is false.	
    */
    SaxonProcessor(bool l=false){license = l;}
	
    XsltProcessor * newTransformer(){ return (new XsltProcessor(license));}
//	XPathEngine
//	XQueryEngine
//	SchemaManager

private:

	bool license;
	

};



#endif /* SAXON_XSLT_H */
