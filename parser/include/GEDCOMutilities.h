/*
 * Student Name: Miriam Snow
 * Student Number: 0954174
 */

#ifndef GEDCOMUTILITIES_H
#define GEDCOMUTILITIES_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "LinkedListAPI.h"

//Represents a line from the file 
typedef struct {

	//the level number of the current line
	int level;
	
	//the information on the line, after the tag/id
	char * lineInfo;
	
	//the line number of the line
	int lineNum;
	
	//was this line number used, or should it be printed with other fields
	bool used;
    
} FileLineData;

/*
 * Function: check to see if the file is invalid (NULL, empty string, no contents or wrong file extension
 * @param: string - name of file
 * @return: true if file is valid, false if an error has occured
 */
ErrorCode checkFile(char * fileName, int * objectLine);

/*
 * Function: remove the newline and carriage return from a string of characters
 * @param: string - line to remove newline and carriage return from (\n,\r)
 * @return: void
 */
void removeNewline(char line[]);

/*
 * Function: delete the contents of a file node
 * @param: FileLineData - data of list Node to be deleted
 * @return: void
 */
void deleteFile(void* toBeDeleted);

/*
 * Function: compare contents of a FileLineData struct based on line number
 * @param: FileLineData node 1 to compare
 * @param: FileLineData node 2 to compare
 * @return: integer value - 0 if equal, -1 if first is < second, 1 if first > second
 */
int compareFile(const void* first,const void* second);

/*
 * Function: print contents of a FileLineData struct
 * @param: FileLineData struct to be printed
 * @return: allocated string of contents of the struct. must be freed after use
 */
char* printFile(void* toBePrinted);

/*
 * Function: create the Header struct of the GEDCOM file
 * @param: List containing all the lines of the file of type FileLineData
 * @param: pointer to a line number integer if an error occurs
 * @param: pointer to a string representing the source of the header
 * @param: pointer to a float indicating the GEDCOM version of the file
 * @param: pointer to the encoding value of the file. must be one of the enum values in CharSet
 * @return: error code. OK if not error. error is of enum type ErrorCode
 */
enum eCode createHeader(List * fileContents, int * lineNumber, char ** source, float * gedcVersion, CharSet * encoding);

/*
 * Function: check to make sure the version of a file is valid
 * @param: string representing the file line of the version
 * @return: integer indicating if value is okay or not. 0 if invalid, 1 if okay.
 */
int checkVersion(char * string);

/*
 * Function: get the encoding version based on a given string
 * @param: string to convert to type CharSet
 * @return: CharSet value returned
 */
CharSet getEncoding(char * string);

/*
 * Function: convert a CharSet type to a string
 * @param: CharSet encoding type to convert to a string
 * @return: string based on input of CharSet parameter
 */
char * getEncodingString(CharSet encoding);

/*
 * Function: fill in the otherFields list for the header
 * @param: List containing lines from the file
 * @param: pointer to the otherFields list in the header that will be added to
 * @return: CharSet value based on if an error occured. OK if everything is valid
 */
ErrorCode getOtherFields(List fileContents, List * otherFields, int * lineNum);

/*
 * Function: check to see if a string has an @ sign in it
 * @param: string to check 
 * @return: integer value returned - 1 if an @ sign is found, 0 if not
 */
int hasAt(char * string);

/*
 * Function: get the line number of the last line of the header
 * @param: List containing the lines of the file and it's data
 * @return: integer value equal to the line number of the last line of the header
 */
int getLastLineHeader(List * fileContents);

/*
 * Function: check to see if there is a CONC or CONT tag underneath the line currently looking at
 * @param: line number of the line to check if there is a continuation of
 * @param: List containing the lines of the file
 * @param: string which will have the values of the CONC or CONT tags added to it
 * @return: void
 */
void checkContinuation(int lineNum, List * fileContents, char * string);

/* Function get the list of individuals to put in the GEDCOM object
 * @param: List containing the lines of the file
 * @param: pointer to the List containing the individuals from the file
 * @param: pointer to an integer representing the lineNumber - used if an error is returned
 * @return: ErrorCode value if there is an error. OK if no error has occured
 */
ErrorCode getIndividualsList(List fileContents,List * individuals, int * lineNumber);

/*
 * Function: check to see if the string is a pointer (has an @ sign as the first character and the last)
 * @param: string to check if its a pointer or not
 * @return: integer value - 1 if it is a pointer, 0 if not
 */
int isPointer(char * string);

/*
 * Function: get the first and last name of an individual based on the NAME tag value, separated by /
 * @param: pointer to the string givenName
 * @param: pointer to the string surName
 * @return: void - givenName and surName values are filled, empty strings if values are not there
 */
void getName(char ** givenName, char ** surName, char * string);

/*
 * Function: get otherField for the submitter struct of the GEDCOM object
 * @param: List containing the content of each line from the file
 * @param: pointer to the list otherFields to be filled in
 * @param: integer representing the line number of the first line of the submitter
 * @return: CharSet error code - OK if no error occurs
 */
ErrorCode getSubmOtherFields(List fileContents, List * otherFields, int lineNum, int * errorLineNum);

/*
 * Function: check to see if a string is an individual event
 * @param: string to check
 * @return: integer value - 1 if it is an event, 0 if not
 */
int isEvent(char * string);

/*
 * Function: check to see if a string is a family event
 * @param: string to check
 * @return: integer value - 1 if it is an event, 0 if not
 */
int isFamEvent(char * string);

/*
 * Function: fill in the families list in the GEDCOM obj
 * @param: List containing contents of the file
 * @param: pointer to the list of Famileis in the GEDCOM object
 * @return: errorCode value - OK if valid
 */
ErrorCode getFamiliesList(List fileContents, List * families);

/*
 * Function: point the husband and wife pointers in each family struct to the correct individual
 * @param: pointer to the list of Families in the GEDCOM object
 * @param: pointer to the list of individuals in the GEDCOM object
 * @return: void
 */
void getHusbAndWife(List * families, List * individuals);

/*
 * Function: add a list of related families to each individual struct 
 * @param: pointer to the list of Families in the GEDCOM object
 * @param: pointer to the list of Individuals in the GEDCOM object
 * @return: void
 */
void getRelFams(List * families, List * individuals);

/*
 * Function: dummy delete function to delete nodes without a double free error of data. Used for shared lists like children and related families
 * @param: void data to be deleted - not used in the function
 * @return: void
 */
void dummyDelete (void * toBeDeleted);

/*
 * Function: add a list of children to each family struct
 * @param: pointer to the list of Families in the GEDCOM object
 * @param: pointer to the list of Individuals in the GECCOM object
 * @return: void
 */
void getChildren(List * families, List * individuals);

/*
 * Function: return the integer number of a month (1-12) based on a string
 * @param: first three letters of the month in lower case - string
 * @return: integer representing the month. 0 if the string does not match a month
 */
int getMonth(char * string);


/*
 * Function: help with comparing events. parses the dates of an event to see which is earlier
 * @param: date of event one
 * @param: date of event two
 * @return: 1 if string 1 is > string 2, 2 if string 2 is > string 1, 0 if strings are equal
 */
int getDateVal(char * string, char * string2);

/* 
 * Function: wrapper function of compareIndividuals to help with the FindPerson function
 * @param: first Individual to compare
 * @param: second Individual to compare
 * @return: true if two individuals are the same, false otherwise
 */
bool compareDummy(const void* first, const void* second);

/*
 * Function: helper function for getDescendants function. This one is called recurrsively
 * @param: List pointer to list to add descendants to
 * @param: Individual to get the descendants of
 * @return: void
 */
void getChildrenDes(List * desList, const Individual * person);

/*
 * Function: function to replace fgets and read in the file character by character. takes into account the possible variations of /n and /r line terminators
 * @param: buffer string which is the line that is read in
 * @param: File pointer of the file to read
 * @return: error value OK if everything is valid, INV_HEADER if a line is > 255 characters long
 */
ErrorCode newFgets(char * buffer, FILE * fptr);

Individual * createCopy(Individual * toCopy);

bool isLevelOneIndi(char * tag);

int compareEntireFams(const void * first, const void * second);

int compareEntireIndi(const void * first, const void * second);

void getChildrenDesN(List * desList, const Individual * person, int * num, int max);

char* dummyPrint(void* toBePrinted);

int dummyCompare(const void* first,const void* second);

void deleteList(void* toBeDeleted);

List * getList(List * desList, int num, int max);

int isSame(Individual * toCheck, List * theDes);

void putLNAtBack(List * desList);

void getChildrenAncestors(List * ansList, const Individual * person, int * num, int max);

void removeQuotes(char * string);

int checkForNewline(char * string);

void* deleteDataFromList2(List* list, void* toBeDeleted);

char * tableInfo(char * fileName);

char * indInfo(char * fileName);

char * createSimple(char * fileName, char * subName, char * subAddress);

char * addTheIndi(char * fileName, char * JSONstring);

char * validateFile(char * fileName);

char * desToJSON(char * fileName, char * indJSON, int max);

char * ansToJSON(char * fileName, char * indJSON, int max);

#endif
