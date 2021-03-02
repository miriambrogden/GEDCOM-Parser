#include "GEDCOMparser.h"
#include "GEDCOMutilities.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

GEDCOMerror createGEDCOM(char* fileName, GEDCOMobject** obj){
	*obj = NULL;
	GEDCOMerror errorVal;
	errorVal.type = 0;
	errorVal.line = 0;
	int objectLine = -1;
	
	if (fileName == NULL){
		errorVal.type = INV_FILE;
		errorVal.line = -1;
		return errorVal;
	}
	
	//check if file is valid
	ErrorCode validFile = checkFile(fileName, &objectLine);
	if (validFile == INV_FILE){
		errorVal.type = INV_FILE;
		errorVal.line = -1;
		return errorVal;
	} else if (validFile == INV_GEDCOM){
		errorVal.type = INV_GEDCOM;
		errorVal.line = -1;
		return errorVal;
	} else if (validFile == INV_RECORD){
		errorVal.type = INV_RECORD;
		errorVal.line = objectLine;
		return errorVal;
	} else {		
		//put contents of file in a list
		FILE * fptr = fopen(fileName, "r");
		char * buffer = malloc(sizeof(char) * 1000);
		strcpy(buffer, "");
		int theLevel = -1;
		List fileContents = initializeList(&printFile,&deleteFile,&compareFile);
        char * token;
        FileLineData * fileData = NULL;
        int theLineNumber = 1;
        newFgets(buffer, fptr);
        while (strcmp(buffer, "") != 0){
			if (strcmp(buffer,"\n") != 0 && strcmp(buffer," ") != 0 && strcmp(buffer,"") != 0 && strcmp(buffer,"\r") != 0 && strcmp(buffer,"\n\r") != 0 && strcmp(buffer,"\r\n") != 0){
				removeNewline(buffer);
				fileData = malloc(sizeof(FileLineData));
				token = strtok(buffer, " ");
				theLevel = atoi(token);
				fileData->level = theLevel;
				token = strtok(NULL, "");
				fileData->lineInfo = malloc(sizeof(char) * 256);
				strcpy(fileData->lineInfo, "");
				if (token != NULL){
					removeNewline(token);
					strncpy(fileData->lineInfo, token, strlen(token)+1);
				}
					fileData->lineNum = theLineNumber;
					theLineNumber++; 
					fileData->used = false;
					insertBack(&fileContents,fileData);
			}
			free(buffer);
			buffer = malloc(sizeof(char) * 1000);
			strcpy(buffer, "");
			newFgets(buffer, fptr);
        }
		free(buffer);
        fclose(fptr);
        
        //check for 0 TRLR at end of file
        Node * currVal = fileContents.head;
        int trlrExists = 0;
        while (currVal != NULL){
			FileLineData * fileInfoVal = (FileLineData*)(currVal->data);
			if (strcmp(fileInfoVal->lineInfo, "TRLR") == 0 && fileInfoVal->level == 0){
				trlrExists = 1;
			}
			currVal = currVal->next;
		}
		
		if (trlrExists == 0){
			errorVal.type = INV_GEDCOM;
			errorVal.line = -1;
			clearList(&fileContents);
			return errorVal;
		}
			
		//check for wrongly incremented lines
		currVal = NULL;
		currVal = fileContents.head;
		while (currVal != NULL){
			FileLineData * testLevel = currVal->data;
			if (currVal->next != NULL){
				FileLineData* testLevelTwo = currVal->next->data;
				int one = testLevel->level + 1;
				int two = testLevelTwo->level;
				if (two > one){
					errorVal.type = INV_RECORD;
					errorVal.line = testLevelTwo->lineNum;
					clearList(&fileContents);
					return errorVal;
				}
			}
			currVal = currVal->next;
		}	
			
        //get header
        Node * curr = fileContents.head;
        int headerExists = 0;
        int headerExists2 = 0;
        while (curr != NULL){
			FileLineData* fileInfo = (FileLineData*)(curr->data);
			if (fileInfo->level != 0 && (strcmp(fileInfo->lineInfo, "HEAD") == 0)){
					headerExists2++;
				} else if (fileInfo->level == 0 && (strcmp(fileInfo->lineInfo, "HEAD") == 0)){
					headerExists++;	
					//check for multiple headers
					/*if (headerExists == 2){
						errorVal.type = INV_GEDCOM;
						errorVal.line = -1;
						clearList(&fileContents);
						return errorVal;					
					}*/
				}
				
				curr = curr->next;
			}
			
			//check for header with wrong line number
			if (headerExists2 != 0){
				errorVal.type = INV_HEADER;
				errorVal.line = getLastLineHeader(&fileContents);
				clearList(&fileContents);
				return errorVal;
			}
			
			//check for no header
			if (headerExists == 0){
				errorVal.type = INV_GEDCOM;
				errorVal.line = -1;
				clearList(&fileContents);
				return errorVal;
			}
		
			int lineNumber = 0;
			char * source = malloc(sizeof(char) * 250);
			strcpy(source,"");
			float gedcVersion = -1;
			CharSet encoding = -1;
			*obj = malloc(sizeof(GEDCOMobject));
			(*obj)->header = malloc(sizeof(Header));
			
			
			//When there are no errors with the header, proceed to creating it
			enum eCode headerError = createHeader(&fileContents, &lineNumber, &source, &gedcVersion, &encoding);	
			
			//check to make sure the gedcversion and encoding values are not missing
			if (gedcVersion == -1){
				lineNumber = getLastLineHeader(&fileContents);
				if (lineNumber == -1){
					lineNumber = fileData->lineNum;
				}
				headerError = INV_HEADER;
			}
			if (encoding == -1){
				lineNumber = getLastLineHeader(&fileContents);
				if (lineNumber == -1){
					lineNumber = fileData->lineNum;
				}
				headerError = INV_HEADER;
			}
			
			//something wrong with the header
			if (headerError != OK){
				errorVal.type = INV_HEADER;
				errorVal.line = lineNumber;
				clearList(&fileContents);
				free((*obj)->header);
				(*obj)->header = NULL;
				free(source);
				source = NULL;
				free(*obj);
				*obj=NULL;
				return errorVal;
			} 
			
			
			//get all the header fields to put in "Other" if they exist
			lineNumber = 0;
			List otherFields = initializeList(&printField,&deleteField,&compareFields);
			ErrorCode other = getOtherFields(fileContents,&otherFields, &lineNumber);
			if (other != OK){
				clearList(&fileContents);
				clearList(&otherFields);
				free((*obj)->header);
				(*obj)->header = NULL;
				free(source);
				source = NULL;
				free(*obj);
				(*obj) = NULL;
				errorVal.type = INV_RECORD;
				errorVal.line = lineNumber;
				return errorVal;
			}
		
			
			//add things to the header
			strncpy((*obj)->header->source,source,strlen(source) +1); 	 //add source to header	
			(*obj)->header->gedcVersion = gedcVersion;
			(*obj)->header->encoding = encoding;
			(*obj)->header->otherFields = otherFields;	
			
			free(source);
			source = NULL;

			int pointerLineNum = -1;
			
			//create submitter
			FileLineData * fileInfo = NULL;
			curr = fileContents.head;
			char * pointer = malloc(sizeof(char) * 256);
			strcpy(pointer, "");
			int endOfHead = getLastLineHeader(&fileContents);
			fileInfo = curr->data;
			char * temporary = malloc(sizeof(char) * 256);
			while (fileInfo->lineNum <= endOfHead){
				fileInfo = curr->data;
				strcpy(temporary, fileInfo->lineInfo);
				token = strtok(temporary, " ");
				if (token != NULL && strcmp(token, "SUBM") == 0 && fileInfo->level == 1){
					token = strtok(NULL, "");
					pointerLineNum = fileInfo->lineNum;
					if (token != NULL){
						if (hasAt(token) == 1){
							strncpy(pointer, token, strlen(token) +1);
						}
					}
				}
				curr = curr->next;
			}
			
			//there is no SUBM line
			if (pointerLineNum == -1){
				errorVal.type = INV_HEADER;
				errorVal.line = getLastLineHeader(&fileContents);
				free(pointer);
				pointer = NULL;
				free(temporary);
				temporary = NULL;
				clearList(&((*obj)->header->otherFields));
				free((*obj)->header);
				free(*obj);
				*obj = NULL;
				clearList(&fileContents); 	
				return errorVal;
			}
				
			strcpy(temporary, "");
			// go to pointer and get submitter values
			if (strcmp(pointer, "") != 0){
				curr = fileContents.head;
				while(curr != NULL){
					fileInfo = curr->data;
					strcpy(temporary, fileInfo->lineInfo);
					token = strtok(temporary, " ");
					if (token != NULL && strcmp(pointer, token) == 0){
						pointerLineNum = fileInfo->lineNum;
					}
					curr = curr->next;
				}
			}
			
			//there is no @ subm reference
			if (pointerLineNum == -1){
				errorVal.type = INV_GEDCOM;
				errorVal.line = -1;
				free(pointer);
				pointer = NULL;
				free(temporary);
				temporary = NULL;
				clearList(&fileContents); 	
				return errorVal;
			}
			
			
			// go to the pointer and get info on it
			curr = fileContents.head;
			Node * temp = NULL;
			while (curr != NULL){ 
				fileInfo = curr->data;
				if (fileInfo->lineNum == pointerLineNum){
					temp = curr;
					curr = NULL;
				} else {
					curr = curr->next;
				}
			}
			
			//int errorNum = 0;
			strcpy(temporary, "");
			char * name = malloc(sizeof(char) * 256);
			strcpy(name, "");
			curr = temp;
			temp = temp->next;
			FileLineData * foundInfo = curr->data;
			fileInfo = temp->data;
			
			//errorNum = foundInfo->lineNum;
			while (fileInfo->level > foundInfo->level){
				fileInfo = temp->data;
				strncpy(temporary, fileInfo->lineInfo, strlen(fileInfo->lineInfo)+1);
				token = strtok(temporary, " ");
				//errorNum = fileInfo->lineNum;
				if (token != NULL && strcmp(token, "NAME") == 0){
					token = strtok(NULL, "");
					if (token != NULL){
						strcpy(name, token);
						checkContinuation(fileInfo->lineNum, &fileContents, name);
					}
				}
				temp = temp->next;
			}
			
			
			//no name found
			if (strcmp(name, "") == 0){
				errorVal.type = INV_GEDCOM;
				errorVal.line = -1;		
				free(pointer);
				free(name);
				name = NULL;
				pointer = NULL;
				free(temporary);
				temporary = NULL;
				clearList(&fileContents);
				clearList(&((*obj)->header->otherFields));
				free((*obj)->header);
				free(*obj);
				*obj = NULL;
				return errorVal;
			}
	
			if (strlen(name) > 61 || strlen(name)< 1){
				errorVal.type = INV_GEDCOM;
				errorVal.line = -1;
				free(name);
				name = NULL;
				free(pointer);
				pointer = NULL;
				free(temporary);
				temporary = NULL;
				clearList(&fileContents); 	
				return errorVal;
			} /*else {
				errorNum = 0;
			}*/
			
			//allocate submitter (add additional space for flexible array - address member)
			(*obj)->submitter = malloc(sizeof(Submitter) + 256 * sizeof(char));	
			strcpy((*obj)->submitter->submitterName, name);
			free(name);
			name = NULL;

			//find the address
			//go to the pointer
			curr = fileContents.head;
			temp = NULL;
			while (curr != NULL){ 
				fileInfo = curr->data;
				if (fileInfo->lineNum == pointerLineNum){
					temp = curr;
					curr = NULL;
				} else {
					curr = curr->next;
				}
			}
			
			strcpy(temporary, "");
			char * address = malloc(sizeof(char) * 256);
			strcpy(address, "");
			curr = temp;
			//errorNum = 0;
			temp = temp->next;
			foundInfo = curr->data;
			fileInfo = temp->data;
			
			//find ADDR under the pointer
			while (fileInfo->level > foundInfo->level){
				fileInfo = temp->data;
				strncpy(temporary, fileInfo->lineInfo, strlen(fileInfo->lineInfo)+1);
				token = strtok(temporary, " ");
				//errorNum = fileInfo->lineNum;
				if (token != NULL && strcmp(token, "ADDR") == 0 && fileInfo->level == 1){
					token = strtok(NULL, "");
					if (token != NULL){
						strcpy(address, token);
						checkContinuation(fileInfo->lineNum, &fileContents, address);
					}
				}
				temp = temp->next;
			}
			
			strcpy((*obj)->submitter->address, address);
			
			free(address);
			address = NULL;			
			free(pointer);
			pointer = NULL;
			free(temporary);
			temporary = NULL;
			
			//other fields in the submitter
			List submOtherFields = initializeList(&printField,&deleteField,&compareFields);
			lineNumber = 0;
			ErrorCode submOther = getSubmOtherFields(fileContents, &submOtherFields, pointerLineNum, &lineNumber);
			if (submOther != OK){
				errorVal.type = INV_RECORD;
				errorVal.line = lineNumber;
				clearList(&fileContents);
				clearList(&submOtherFields);
				free((*obj)->submitter);
				clearList(&((*obj)->header->otherFields));
				free((*obj)->header);
				free(*obj);
				*obj = NULL;
				return errorVal;
			}
			
			(*obj)->submitter->otherFields = submOtherFields;
			
			
			(*obj)->header->submitter = (*obj)->submitter;
			
			//parse individuals
			lineNumber = 0;
			List individuals = initializeList(&printIndividual,&deleteIndividual,&compareIndividuals);
			getIndividualsList(fileContents,&individuals, &lineNumber);
			(*obj)->individuals = individuals;
			
			//parse families
			lineNumber = 0;
			List families = initializeList(&printFamily,&deleteFamily,&compareFamilies);
			getFamiliesList(fileContents,&families);
			(*obj)->families = families;
			
			//link husband and wife
			getHusbAndWife(&families, &individuals);
			
			//get related families in individuals list 
			getRelFams(&families, &individuals);
			
			//get children in families list
			getChildren(&families, &individuals);

			//delete XREFs from individuals
			Node * xrefCurr = (*obj)->individuals.head;
			while (xrefCurr != NULL){
				Individual * xrefIndi = xrefCurr->data;
				Node * xrefCurr2 = xrefIndi->otherFields.head;
				while (xrefCurr2 != NULL){
					Field * xrefField = xrefCurr2->data;
					xrefCurr2 = xrefCurr2->next;
					if (isPointer(xrefField->tag) == 1){
						deleteDataFromList(&xrefIndi->otherFields, xrefField);
						deleteField(xrefField);	
					} else if (isPointer(xrefField->value) == 1){
						deleteDataFromList(&xrefIndi->otherFields, xrefField);
						deleteField(xrefField);	
					} /*else if ((strcmp(xrefField->tag, "GIVN") == 0) || (strcmp(xrefField->tag, "SURN") == 0)){
						deleteDataFromList(&xrefIndi->otherFields, xrefField);
						deleteField(xrefField);	
					}*/
				}
				xrefCurr = xrefCurr->next;
			}
			
			
			//delete XREFs from families
			Node * xrefCurr3 = (*obj)->families.head;
			while (xrefCurr3 != NULL){
				Family * xrefFam = xrefCurr3->data;
				Node * xrefCurr4 = xrefFam->otherFields.head;
				while (xrefCurr4 != NULL){
					Field * xrefField2 = xrefCurr4->data;
					xrefCurr4 = xrefCurr4->next;
					if (isPointer(xrefField2->tag) == 1){
						deleteDataFromList(&xrefFam->otherFields, xrefField2);
						deleteField(xrefField2);	
					} else if (isPointer(xrefField2->value) == 1){
						deleteDataFromList(&xrefFam->otherFields, xrefField2);
						deleteField(xrefField2);	
					}
				}
				xrefCurr3 = xrefCurr3->next;
			}
			
			//delete XREF from submitter
			xrefCurr3 = NULL;
			xrefCurr3 = (*obj)->submitter->otherFields.head;
			while (xrefCurr3 != NULL){
				Field * xrefField = xrefCurr3->data;
				xrefCurr3 = xrefCurr3->next;
				if (isPointer(xrefField->tag) == 1){
					deleteDataFromList(&((*obj)->submitter->otherFields), xrefField);
					deleteField(xrefField);	
				} else if (isPointer(xrefField->value) == 1){
					deleteDataFromList(&((*obj)->submitter->otherFields), xrefField);
					deleteField(xrefField);	
				}
			}
			
			//delete XREF from header
			xrefCurr3 = NULL;
			xrefCurr3 = (*obj)->header->otherFields.head;
			while (xrefCurr3 != NULL){
				Field * xrefField = xrefCurr3->data;
				xrefCurr3 = xrefCurr3->next;
				if (isPointer(xrefField->tag) == 1){
					deleteDataFromList(&((*obj)->header->otherFields), xrefField);
					deleteField(xrefField);	
				} else if (isPointer(xrefField->value) == 1){
					deleteDataFromList(&((*obj)->header->otherFields), xrefField);
					deleteField(xrefField);	
				}
			}
			
        clearList(&fileContents); 
		return errorVal;
	}

}

char* printGEDCOM(const GEDCOMobject* obj){
	char * string = malloc(sizeof(char) * 100000);
	if (obj == NULL){
		strcpy(string, "Error Printing File\n");
	} else {
	char * temp = malloc(sizeof(char) * 100);
	Header * theHead = (Header*)(obj->header);
	strcpy(string, "\n***Header***\nSource: ");
	strcat(string,theHead->source);
	strcat(string,"\nGEDC Version: ");
	sprintf(temp,"%f",theHead->gedcVersion);
	strcat(string,temp);
	strcat(string,"\nEncoding: ");
	char * temp2 = getEncodingString(theHead->encoding);
	strcat(string,temp2);
	strcat(string,"\n--Other Fields--\n");
	int x = getLength(theHead->otherFields);
	int size = x* sizeof(Field) + (sizeof(char) * strlen(string)) + sizeof(char) * 1500;
	string = realloc(string, size);
	Node * curr = theHead->otherFields.head;
	Field * theField = NULL;
	while(curr != NULL){
		theField = curr->data;
		char * toPrint = printField(theField);
		strcat(string, toPrint);
		free(toPrint);
		curr = curr->next;
		strcat(string, "\n");
	}
	curr = NULL;
	strcat(string, "\n***Submitter***\nName: ");
	strcat(string, theHead->submitter->submitterName);
	strcat(string, "\nAddress: ");
	strcat(string, theHead->submitter->address);	
	free(temp);
	temp = NULL;
	size = size + 100;
	string = realloc(string, size);
	strcat(string,"\n--Other Fields--\n");
	curr = theHead->submitter->otherFields.head;
	theField = NULL;
	while(curr != NULL){
		theField = curr->data;
		char * toPrint = printField(theField);
		size = size + strlen(toPrint) + 50;
		string = realloc(string, size);
		strcat(string, toPrint);
		free(toPrint);
		curr = curr->next;
		strcat(string, "\n");
	}
	
	strcat(string, "\n***Individual Records***\n\n");
	curr = obj->individuals.head;
	Individual * aRecord = NULL;
	while(curr != NULL){
		aRecord = curr->data;
		strcat(string, "--Individual Record Information--\n");
		char * tempIndi = printIndividual(aRecord);
		size = size + strlen(tempIndi) + 50;
		string = realloc(string, size);
		strcat(string, tempIndi);
		free(tempIndi);
		tempIndi = NULL;	
		curr = curr->next;
		strcat(string,"\n");
	}
	
	strcat(string, "\n***Family Records***\n\n");
	curr = obj->families.head;
	Family * aFamRec = NULL;
	while (curr != NULL){
		aFamRec = curr->data;
		strcat(string, "--Family Record Information--\n");
		char * tempFam = printFamily(aFamRec);
		size = size + strlen(tempFam) + 50;
		string = realloc(string, size);
		strcat(string, tempFam);
		free(tempFam);
		tempFam = NULL;
		curr = curr->next;
		strcat(string, "\n");
	}
}
	
    return string;
	
}

char* printError(GEDCOMerror err){
	char * lineNum = malloc(sizeof(int));
	sprintf(lineNum,"%d",err.line);
	char * string = malloc(sizeof(char) * 50);
	if (err.type == OK){
		strncpy(string,"GEDCOM is okay",50);
	} else if (err.type == INV_FILE){
		strncpy(string,"Invalid File",50);
	} else if (err.type == INV_GEDCOM){
		strncpy(string,"Invalid Gedcom Object",50);
	} else if (err.type == INV_HEADER){
		strncpy(string, "Invalid Header on line ",50);
		strcat(string,lineNum);
	} else if (err.type == INV_RECORD){
		strncpy(string,"Invalid record on line ", 50);
		strcat(string,lineNum);
	} else if (err.type == OTHER_ERROR){
		strncpy(string,"There was some other error",50);
	}
	free(lineNum);
	lineNum = NULL;
    return string;
	
}

void deleteGEDCOM(GEDCOMobject* obj){
	if (obj != NULL){
	//header
	Header * theHeader = obj->header;
	List other = theHeader->otherFields;
	clearList(&other);
	free(obj->header);
	obj->header = NULL;
	
	//submitter
	Submitter * theSubmitter = obj->submitter;
	List otherSubm = theSubmitter->otherFields;
	clearList(&otherSubm);
	free(obj->submitter);
	obj->submitter = NULL;
	
	//individuals list
	List individualsList = obj->individuals;
	Node * curr = individualsList.head;
	Individual * temp = NULL;
	while (curr != NULL){
		temp = curr->data;
		clearList(&(temp->events));
		clearList(&(temp->otherFields));
		curr = curr->next;
	}
	clearList(&individualsList);
	
	
	//families list
	List famList = obj->families;
	Node * current = famList.head;
	Node * currentTwo = NULL;
	Family * tempTwo = NULL;
	Event * tempThree = NULL;
	while (current != NULL){
		tempTwo = current->data;
		currentTwo = tempTwo->events.head;
		while (currentTwo != NULL){
			tempThree = currentTwo->data;
			clearList(&(tempThree->otherFields));
			currentTwo = currentTwo->next;
		}
		clearList(&(tempTwo->events));
		clearList(&(tempTwo->otherFields));
		current = current->next;
	}
	clearList(&famList);
	free(obj);
	obj = NULL;
}
	
}

void deleteField(void* toBeDeleted){
	if (toBeDeleted == NULL){
		return;
	}
	Field * theField = (Field *)(toBeDeleted);
	if (theField->tag != NULL){
		free(theField->tag);
		theField->tag = NULL;
    } if (theField->value != NULL){
		free(theField->value);
		theField->value = NULL;
	} 
	free(theField);
	theField = NULL;
}

int compareFields(const void* first,const void* second){

	Field * a = (Field*)first;
    Field * b = (Field*)second;
    
    if (a == NULL || b==NULL){
		return INT_MIN;
	}
	
    int value = 0;

	char * stringA = malloc(sizeof(char) * 256);
	char * stringB = malloc(sizeof(char) * 256);
	
	if ((strcmp(a->tag, "") != 0) && (strcmp(a->value, "") != 0)){
		strcpy(stringA, a->tag);
		strcat(stringA, " ");
		strcat(stringA, a->value);
	}
	
	if ((strcmp(b->tag, "") != 0) && (strcmp(b->value, "") != 0)){
		strcpy(stringB, b->tag);
		strcat(stringB, " ");
		strcat(stringB, b->value);
	}
	
    value = strcmp(stringA,stringB);
    free(stringA);
    stringA = NULL;
    free(stringB);
    stringB = NULL;
  
    return value;
}

char* printField(void* toBePrinted){
	Field * theField = (Field*)(toBePrinted);
	int x = 0;
	int y = 0;
	if (strcmp(theField->tag, "") != 0){
		x = strlen(theField->tag);
	}
	if (strcmp(theField->value, "") != 0){
		y = strlen(theField->value);
	}
	int z = x+y+50;
	char * string = malloc(sizeof(char) * z);
	strcpy(string,"Tag: ");
	strcat(string,theField->tag);
	strcat(string, "    ");
	strcat(string, "Value: ");
	strcat(string, theField->value);
	return string;
}

void deleteIndividual(void* toBeDeleted){
	if (toBeDeleted == NULL){
		return;
	}
	Individual * theRecord = (Individual *)(toBeDeleted);
	free(theRecord->givenName);
	theRecord->givenName = NULL;
    free(theRecord->surname);
    theRecord->surname = NULL;
    List indiEvents = theRecord->events;
    clearList(&indiEvents);
    List indiFields = theRecord->otherFields;
    clearList(&indiFields);
    if (theRecord->families.head != NULL){
		List relFams = theRecord->families;
		clearList(&relFams);
	}

	free(theRecord);
	theRecord = NULL;
}

int compareIndividuals(const void* first,const void* second){
	char * stringA = malloc(sizeof(char) * 256);
	char * stringB = malloc(sizeof(char) * 256);
	strcpy(stringA, "");
	strcpy(stringB,"");
	int value = INT_MIN;
	Individual * a = (Individual*)first;
    Individual * b = (Individual*)second;
    if (a != NULL && b != NULL){
    
		strcpy(stringA, a->surname);
		strcat(stringA, ",");
		strcat(stringA, a->givenName);
		
		strcpy(stringB, b->surname);
		strcat(stringB, ",");
		strcat(stringB, b->givenName);
		
		value = strcmp(stringA,stringB);		
	}
		free(stringA);
		stringA = NULL;
		free(stringB);
		stringB = NULL;

    return value;
    
	
}
char* printIndividual(void* toBePrinted){
	char * string = malloc(sizeof(char) * 600);
	strcpy(string, "");
	
	if (toBePrinted == NULL){
		strcpy(string, "Error Printing Individual");
		return string;
	}
	
	int size = 0;
	
	Individual * aRecord = (Individual *)(toBePrinted);
		strcat(string, "Given Name: ");
		strcat(string, aRecord->givenName);
		strcat(string, "\nSurname: ");
		strcat(string, aRecord->surname);
		strcat(string, "\n--Events--\n");
		
		Node * current = aRecord->events.head;
		while (current != NULL){
			Event * anEvent = current->data;
			char * printAnEvent = printEvent(anEvent);
			size = size + strlen(printAnEvent) + 100;
			string = realloc(string, size);
			strcat(string, printAnEvent);
			free(printAnEvent);
			current = current->next;
			strcat(string, "\n");
		}
		strcat(string, "--Related Families--\n");
		Node * currFams = aRecord->families.head;
		while (currFams != NULL){
			Family * currFamData = currFams->data;
			if (currFamData->husband != NULL){
					size = size + strlen(currFamData->husband->givenName) + 100;
					size = size + strlen(currFamData->husband->surname) + 100;
			}
			if (currFamData->wife != NULL){
				size = size + strlen(currFamData->wife->givenName) + 100;
				size = size + strlen(currFamData->wife->surname) + 100;
			}
			size = size + 200;
			string = realloc(string, size);
			strcat(string, "Parent(s) of Related Family: ");
			if (currFamData->husband != NULL){
				size = size + strlen(currFamData->husband->givenName) + 100;
				size = size + strlen(currFamData->husband->surname) + 100;
				string = realloc(string, size);
				strcat(string, currFamData->husband->givenName);
				strcat(string, currFamData->husband->surname);
				
			}
			if (currFamData->husband != NULL && currFamData->wife != NULL){
				strcat(string, ", ");
			}			
			if (currFamData->wife != NULL){
				size = size + strlen(currFamData->wife->givenName) + 100;
				size = size + strlen(currFamData->wife->surname) + 100;
				string = realloc(string, size);
				strcat(string, currFamData->wife->givenName);
				strcat(string, currFamData->wife->surname);
			}
			strcat(string, "\n");
			currFams = currFams->next;
		}

		strcat(string, "--Other Fields--\n");
		Node * current2 = aRecord->otherFields.head;
		while (current2 != NULL){
			Event * anEvent2 = current2->data;
			char * printAnEvent2 = printField(anEvent2);
			size = size + strlen(printAnEvent2) + 100;
			string = realloc(string, size);
			strcat(string, printAnEvent2);
			free(printAnEvent2);
			current2 = current2->next;
			strcat(string, "\n");
		}
	
	return string;
	
}

void deleteEvent(void* toBeDeleted){
	Event * anEvent = (Event *)(toBeDeleted);
	List EventOtherFields = anEvent->otherFields;
	clearList(&EventOtherFields);
	free(anEvent->date);
	anEvent->date = NULL;
	free(anEvent->place);
	anEvent->place = NULL;
	free(anEvent);
	anEvent = NULL;
}

int compareEvents(const void* first,const void* second){
	char * stringA = malloc(sizeof(char) * 256);
	char * stringB = malloc(sizeof(char) * 256);
	Event * a = (Event*)first;
    Event * b = (Event*)second;
    if (a == NULL && b == NULL){
		return -2;
	}
		
    int value = -2;
    int x = 0;
    
    if (strcmp(a->type, "") != 0 && strcmp(b->type,"") != 0){
		strcpy(stringA, a->type);
		strcpy(stringB, b->type);
		value = strcmp(stringA,stringB);
	} else if (strcmp(a->date, "") != 0 && strcmp(b->date, "") != 0){
		strcpy(stringA, a->date);
		strcpy(stringB, b->date);
		x = getDateVal(stringA,stringB);
		if (x == 1){
			value = 1;
		} else if (x == 2){
			value = -1;
		} else {
			value = 0;
		}
	}
	free(stringA);
	stringA = NULL;
	free(stringB);
	stringB = NULL;
	return value;
}

char* printEvent(void* toBePrinted){
	char * string = malloc(sizeof(char) * 1500);
	strcpy(string,"");
	Event * anEvent = (Event *)(toBePrinted);
	strcat(string,"New: Type: ");
	strcat(string, anEvent->type);
	strcat(string,"\n     Date: ");
	strcat(string, anEvent->date);
	strcat(string, "\n     Place: ");
	strcat(string,anEvent->place);
	Node * curr = anEvent->otherFields.head;
	int x = getLength(anEvent->otherFields);
	int size = x * sizeof(Field) + strlen(string) + 50;
	string = realloc(string, size);
	while (curr != NULL){
		strcat(string, "\n");
		Field * temp = curr->data;
		char * toPrint = printField(temp);
		size = size + strlen(toPrint);
		string = realloc(string, size);
		strcat(string, toPrint);
		free(toPrint);
		curr = curr->next;
	}
	return string;
}

void deleteFamily(void* toBeDeleted){
	Family * aFamily = (Family *)(toBeDeleted);
	List famEvents = aFamily->events;
	Node * curr = famEvents.head;
	while (curr != NULL){
		Event * eventInfo = curr->data;
		List other = eventInfo->otherFields;
		clearList(&other);
		curr = curr->next;
	}
	if (aFamily->children.head != NULL){
		List childList = aFamily->children;
		clearList(&childList);
	}
    clearList(&famEvents);
    free(aFamily);
    aFamily = NULL;
	}

int compareFamilies(const void* first,const void* second){
	int famA = 0;
	int famB = 0;
	Family * a = (Family*)first;
    Family * b = (Family*)second;
    
    if (a->husband != NULL){
		famA++;
	}
	if (a->wife != NULL){
		famA++;
	}
	Node * curr = a->children.head;
	while (curr != NULL){
		famA++;
		curr = curr->next;
	}
	if (b->husband != NULL){
		famB++;
	}
	if (b->wife != NULL){
		famB++;
	}
	curr = b->children.head;
	while (curr != NULL){
		famB++;
		curr = curr->next;
	}
	
	
	if (famA > famB){
		return 1;
	} else if (famA == famB){
		return 0;
	} else {
		return -1;
	}
}

char* printFamily(void* toBePrinted){
	char * string = malloc(sizeof(char) * 256);
	strcpy(string, "");
	Family * aFamRec = (Family *)(toBePrinted);
	strcat(string, "Husband: ");
	if (aFamRec->husband != NULL){
		strcat(string, aFamRec->husband->givenName);
		strcat(string, aFamRec->husband->surname);
	}
	strcat(string, "\nWife: ");
	if (aFamRec->wife != NULL){
		strcat(string, aFamRec->wife->givenName);
		strcat(string, aFamRec->wife->surname);
	}
	strcat(string, "\n--Events--\n");
	Node * current = aFamRec->events.head;
	int x = getLength(aFamRec->events);
	int size = x*sizeof(Event) + strlen(string) + 50;
	string = realloc(string, size);
	while (current != NULL){
		Event * anEvent = current->data;
		char * printAnEvent = printEvent(anEvent);
		size = size + strlen(printAnEvent) + 50;
		string = realloc(string, size);
		strcat(string, printAnEvent);
		free(printAnEvent);
		current = current->next;
		strcat(string,"\n");
	}
	
	strcat(string, "--Children--\n");
	Node * current2 = aFamRec->children.head;
	while (current2 != NULL){
		Individual * kid = current2->data;
		size = size + strlen(kid->givenName) + strlen(kid->surname) + 100;
		string = realloc(string, size);
		strcat(string, "Name: ");
		if (kid != NULL){
			strcat(string, kid->givenName);
			strcat(string, kid->surname);
		}
		current2 = current2->next;
		strcat(string,"\n");
	}

	strcat(string, "--Other Fields--\n");
	Node * currentTwo= aFamRec->otherFields.head;
	int y = getLength(aFamRec->otherFields);
	int z = y * sizeof(Field) + 50;
	size = size + x + z;
	string = realloc(string, size);
	while (currentTwo != NULL){
		Field * anOtherField = currentTwo->data;
		char * toPrint = printField(anOtherField);
		size = size + strlen(toPrint);
		string = realloc(string, size);
		strcat(string, toPrint);
		free(toPrint);
		currentTwo = currentTwo->next;
		strcat(string,"\n");
	}
	return string;
}

Individual* findPerson(const GEDCOMobject* familyRecord, bool (*compare)(const void* first, const void* second), const void* person){
	
	if (person == NULL || familyRecord == NULL){
		return NULL;
	}
	Individual *tmp = findElement(familyRecord->individuals, compare, person);
	return tmp;
}

List getDescendants(const GEDCOMobject* familyRecord, const Individual* person){ 
	List DescendantsList = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
	if (person == NULL){
		return DescendantsList;
	}
	if (familyRecord == NULL){
		return DescendantsList;
	}
	getChildrenDes(&DescendantsList, person);
	return DescendantsList;
}

List getDescendantListN(const GEDCOMobject* familyRecord, const Individual* person, unsigned int maxGen){
	List DescendantsList = initializeList(&dummyPrint, &dummyDelete, &dummyCompare);
	if (familyRecord == NULL || person == NULL){
		return DescendantsList;
	}
	
	if (maxGen == 0){
		maxGen = 200;
	}
	
	if (maxGen < 0){
		return DescendantsList;
	}
	
	if (maxGen > 5000){
		return DescendantsList;
	}
	
	if (person == NULL){
		return DescendantsList;
	}
	
	int num = -1;
	
	int maxCount = 1;
	while (maxCount <= maxGen){
		List * newList = malloc(sizeof(List));
		*newList = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
		insertBack(&DescendantsList, newList);
		maxCount++;
	}
	
	getChildrenDesN(&DescendantsList, person, &num, maxGen);
	
	putLNAtBack(&DescendantsList);
	Node * temp = NULL;
	Node * curr = DescendantsList.head;
	while (curr != NULL){
		List * aList = curr->data;
		temp = curr->next;
		if (getLength(*aList) == 0){
			deleteDataFromList2(&DescendantsList, curr);
			deleteGeneration(aList);
		}
		curr = temp;
	}
	
	return DescendantsList;
}

GEDCOMerror writeGEDCOM(char* fileName, const GEDCOMobject* obj){
	GEDCOMerror theError;
	theError.type = OK;
	if (fileName == NULL){
		theError.type = WRITE_ERROR;
		return theError;
	}
	if (obj == NULL){
		theError.type = WRITE_ERROR;
		return theError;
	}
	int last = strlen(fileName);
	if (fileName[last-4] != '.' || fileName[last-3] != 'g' || fileName[last-2] !='e' || fileName[last-1] !='d'){
		theError.type = WRITE_ERROR;
		return theError;
	}
	FILE * toWrite = fopen(fileName, "w");
	if (toWrite == NULL){
		theError.type = WRITE_ERROR;
		return theError;
	}
	
	//write header
	fwrite("0 HEAD", sizeof(char), 6, toWrite);
	fwrite("\n",sizeof(char), 1, toWrite);
	fwrite("1 SOUR ", sizeof(char), 7, toWrite);
	fwrite((*obj).header->source, sizeof(char), strlen((*obj).header->source), toWrite);
	fwrite("\n",sizeof(char), 1, toWrite);
	fwrite("1 GEDC", sizeof(char), 6, toWrite);
	fwrite("\n", sizeof(char), 1, toWrite);
	fwrite("2 VERS ",sizeof(char), 7, toWrite);
	float version = (*obj).header->gedcVersion;
	char * versionString = malloc(sizeof(char) * 256);
	sprintf(versionString, "%g", version);
	fwrite(versionString, sizeof(char), strlen(versionString), toWrite);
	free(versionString);
	CharSet encodingVal = (*obj).header->encoding;
	char * encoding = getEncodingString(encodingVal);
	fwrite("\n",sizeof(char), 1, toWrite);
	fwrite("2 FORM LINEAGE-LINKED", sizeof(char), 21, toWrite);
	fwrite("\n",sizeof(char), 1, toWrite);
	fwrite("1 CHAR ", sizeof(char), 7, toWrite);
	fwrite(encoding, sizeof(char), strlen(encoding), toWrite);
	fwrite("\n",sizeof(char), 1, toWrite);
	
	//write submitter
	fwrite("1 SUBM @SREF@", sizeof(char), 13, toWrite);
	fwrite("\n",sizeof(char), 1, toWrite);
	fwrite("0 @SREF@ SUBM", sizeof(char), 13, toWrite);
	fwrite("\n",sizeof(char), 1, toWrite);
	fwrite("1 NAME ", sizeof(char), 7, toWrite);
	fwrite((*obj).submitter->submitterName, sizeof(char), strlen((*obj).submitter->submitterName), toWrite);
	fwrite("\n",sizeof(char), 1, toWrite);
	if (strcmp((*obj).submitter->address, "") != 0){
		fwrite("1 ADDR ", sizeof(char), 7, toWrite);
		char * address = malloc(sizeof(char) * 256);
		strcpy(address, (*obj).submitter->address);
		int checkNewline = checkForNewline(address);
		if (checkNewline == 0){
			fwrite(address, sizeof(char), strlen(address), toWrite);
			free(address);
			fwrite("\n",sizeof(char), 1, toWrite);
		} else if (checkNewline == 1){
			char * token;
			token = strtok(address, "\n");
			if (token == NULL){
				fclose(toWrite);
				theError.type = WRITE_ERROR;
				return theError;
			}
			token = strtok(NULL, "");
			if (token == NULL){
				fclose(toWrite);
				theError.type = WRITE_ERROR;
				return theError;
			}
			fwrite(address, sizeof(char), strlen(address), toWrite);
			fwrite("\n",sizeof(char), 1, toWrite);
			fwrite("2 CONT ", sizeof(char), 7, toWrite);
			char * cont = malloc(sizeof(char) * 256);
			strcpy(cont, "");
			strcpy(cont, token);
			fwrite(cont, sizeof(char), strlen(cont), toWrite);
			fwrite("\n",sizeof(char), 1, toWrite);
			free(cont);
			free(address);
		}
			
			
	}
	
	//write individuals
	Node * currIndi = (*obj).individuals.head;
	int num = 1;
	while (currIndi != NULL){
		Individual * indi = currIndi->data;
		fwrite("0 @IND", sizeof(char), 6, toWrite);
		char * number = malloc(sizeof(char) * 50);
		sprintf(number, "%d", num);
		if (num < 10){
			fwrite(number, sizeof(char), 1, toWrite);
		} else if (num > 9){
			fwrite(number, sizeof(char), 2, toWrite);
		} else {
			fwrite(number, sizeof(char), 3, toWrite);
		}
		fwrite("@ INDI", sizeof(char), 6, toWrite);
		fwrite("\n", sizeof(char), 1, toWrite);
		if (strcmp(indi->givenName, "") != 0 && strcmp(indi->surname, "") != 0){
			//first name and last name exist
			fwrite("1 NAME ", sizeof(char), 7, toWrite);
			fwrite(indi->givenName, sizeof(char), strlen(indi->givenName), toWrite);
			fwrite(" /", sizeof(char), 2, toWrite);
			fwrite(indi->surname, sizeof(char), strlen(indi->surname), toWrite);
			fwrite("/", sizeof(char), 1, toWrite);
			fwrite("\n", sizeof(char), 1, toWrite);
		} else if ((strcmp(indi->givenName, "") == 0) && (strcmp(indi->surname, "") != 0)){
			//no first name but last name
			fwrite("1 NAME ", sizeof(char), 7, toWrite);
			fwrite(" /", sizeof(char), 2, toWrite);
			fwrite(indi->surname, sizeof(char), strlen(indi->surname), toWrite);
			fwrite("/", sizeof(char), 1, toWrite);
			fwrite("\n", sizeof(char), 1, toWrite);
		} else if (strcmp(indi->givenName, "") != 0 && strcmp(indi->surname, "") == 0){
			//no last name but first name
			fwrite("1 NAME ", sizeof(char), 7, toWrite);
			fwrite(indi->givenName, sizeof(char), strlen(indi->givenName), toWrite);
			fwrite("//", sizeof(char), 2, toWrite);
			fwrite("\n", sizeof(char), 1, toWrite);
		} 
		
		
		//get givn and surn
		Node * currName = indi->otherFields.head;
		while (currName != NULL){
			Field * indiEvent2 = currName->data;
			if (strcmp(indiEvent2->tag, "GIVN") == 0){
				fwrite("2 GIVN ", sizeof(char), 7, toWrite);
				fwrite(indiEvent2->value, sizeof(char), strlen(indiEvent2->value), toWrite);
				fwrite("\n", sizeof(char), 1, toWrite);
			} if (strcmp(indiEvent2->tag, "SURN") == 0){
				fwrite("2 SURN ", sizeof(char), 7, toWrite);
				fwrite(indiEvent2->value, sizeof(char), strlen(indiEvent2->value), toWrite);
				fwrite("\n", sizeof(char), 1, toWrite);
			}
			currName = currName->next;
		}
		
		//get other fields level 1
		Node * currField = indi->otherFields.head;
		while (currField != NULL){
			Field * indiField = currField->data;
			bool isLevelOne = isLevelOneIndi(indiField->tag);
			if (isLevelOne == true){
				fwrite("1 ", sizeof(char), 2, toWrite);
				fwrite(indiField->tag, sizeof(char), strlen(indiField->tag), toWrite);
				fwrite(" ", sizeof(char), 1, toWrite);
				fwrite(indiField->value, sizeof(char), strlen(indiField->value), toWrite);
				fwrite("\n", sizeof(char), 1, toWrite);
			}
			currField = currField->next;
		}
		
		//get events
		Node * currEvent = indi->events.head;
		while (currEvent != NULL){
			Event * indiEvent = currEvent->data;
			fwrite("1 ", sizeof(char), 2, toWrite);
			fwrite(indiEvent->type, sizeof(char), strlen(indiEvent->type), toWrite);
			fwrite("\n", sizeof(char), 1, toWrite);
			if (strcmp(indiEvent->date, "") != 0){
				fwrite("2 DATE ", sizeof(char), 7, toWrite);
				fwrite(indiEvent->date, sizeof(char), strlen(indiEvent->date), toWrite);
				fwrite("\n", sizeof(char), 1, toWrite);
			}
			if (strcmp(indiEvent->place, "") != 0){
				fwrite("2 PLAC ", sizeof(char), 7, toWrite);
				fwrite(indiEvent->place, sizeof(char), strlen(indiEvent->place), toWrite);
				fwrite("\n", sizeof(char), 1, toWrite);
			}
			currEvent = currEvent->next;
		}
		
		
		// link FAMC and FAMS values
		Node * relFam = indi->families.head;
		int  relFamNum = 0;
		while (relFam != NULL){
			Family * relFamData = relFam->data;
			if ((compareEntireIndi(relFamData->husband, indi) == 0) || (compareEntireIndi(relFamData->wife, indi) == 0)){
				Node * theFams = (*obj).families.head;
				relFamNum = 1;
				while(theFams != NULL){
					Family * theFamsData = theFams->data;
					if (compareEntireFams(relFamData, theFamsData) == 0){
						fwrite("1 FAMS @FAM", sizeof(char), 11, toWrite);
						char * relFamNumber = malloc(sizeof(char) * 50);
						sprintf(relFamNumber, "%d", relFamNum);
						if (relFamNum < 10){
							fwrite(relFamNumber, sizeof(char), 1, toWrite);
						} else if (relFamNum >= 10 && relFamNum < 100){
							fwrite(relFamNumber, sizeof(char), 2, toWrite);
						} else {
							fwrite(relFamNumber, sizeof(char), 3, toWrite);
						}
						free(relFamNumber);
						fwrite("@", sizeof(char), 1, toWrite);
						fwrite("\n", sizeof(char), 1, toWrite);
					}
					relFamNum++;
					theFams = theFams->next;
				}			
			} else {
				Node * theFams2 = (*obj).families.head;
				relFamNum = 1;
				while(theFams2 != NULL){
					Family * theFamsData2 = theFams2->data;
					if (compareEntireFams(relFamData, theFamsData2) == 0){
						fwrite("1 FAMC @FAM", sizeof(char), 11, toWrite);
						char * relFamNumber2 = malloc(sizeof(char) * 50);
						sprintf(relFamNumber2, "%d", relFamNum);
						if (relFamNum < 10){
							fwrite(relFamNumber2, sizeof(char), 1, toWrite);
						} else if (relFamNum >= 10 && relFamNum < 100){
							fwrite(relFamNumber2, sizeof(char), 2, toWrite);
						} else {
							fwrite(relFamNumber2, sizeof(char), 3, toWrite);
						}
						fwrite("@", sizeof(char), 1, toWrite);
						fwrite("\n", sizeof(char), 1, toWrite);
						free(relFamNumber2);
					}
					relFamNum++;
					theFams2 = theFams2->next;
				}							
			}				
			relFam = relFam->next;
		}
		
		num++;
		free(number);
		currIndi = currIndi->next;
	}
	
	
	//write families
	Node * currFam = (*obj).families.head;
	int numFam = 1;
	while (currFam != NULL){
		Family * fam = currFam->data;
		fwrite("0 @FAM", sizeof(char), 6, toWrite);
		char * numberFam = malloc(sizeof(char) * 50);
		sprintf(numberFam, "%d", numFam);
		if (numFam < 10){
			fwrite(numberFam, sizeof(char), 1, toWrite);
		} else if (numFam > 9){
			fwrite(numberFam, sizeof(char), 2, toWrite);
		} else {
			fwrite(numberFam, sizeof(char), 3, toWrite);
		}
		fwrite("@ FAM", sizeof(char), 5, toWrite);
		fwrite("\n", sizeof(char), 1, toWrite);
		
		Node * currEventFam = fam->events.head;
		while (currEventFam != NULL){
			Event * famEvent = currEventFam->data;
			fwrite("1 ", sizeof(char), 2, toWrite);
			fwrite(famEvent->type, sizeof(char), strlen(famEvent->type), toWrite);
			fwrite("\n", sizeof(char), 1, toWrite);
			if (strcmp(famEvent->date, "") != 0){
				fwrite("2 DATE ", sizeof(char), 7, toWrite);
				fwrite(famEvent->date, sizeof(char), strlen(famEvent->date), toWrite);
				fwrite("\n", sizeof(char), 1, toWrite);
			}
			if (strcmp(famEvent->place, "") != 0){
				fwrite("2 PLAC ", sizeof(char), 7, toWrite);
				fwrite(famEvent->place, sizeof(char), strlen(famEvent->place), toWrite);
				fwrite("\n", sizeof(char), 1, toWrite);
			}
			currEventFam = currEventFam->next;
		}
		
		
		//link husb, wife and chil values under Family
		Individual * theHusb = NULL;
		Individual * theWife = NULL;
		if (fam->husband != NULL){
			theHusb = fam->husband;
		}
		if (fam->wife != NULL){
			theWife = fam->wife;
		}
		
		Node * linkIndi = (*obj).individuals.head;
		int linkFamNum = 1;
		while (linkIndi != NULL){
			Individual * linkIndiData = linkIndi->data;
			//write husband
			if (theHusb != NULL){
				if (compareEntireIndi(theHusb, linkIndiData) == 0){
					fwrite("1 HUSB @IND", sizeof(char), 11, toWrite);
					char * linkFamNumber = malloc(sizeof(char) * 50);
					sprintf(linkFamNumber, "%d", linkFamNum);
					if (linkFamNum < 10){
						fwrite(linkFamNumber, sizeof(char), 1, toWrite);
					} else if (linkFamNum >= 10 && linkFamNum < 100){
						fwrite(linkFamNumber, sizeof(char), 2, toWrite);
					} else {
						fwrite(linkFamNumber, sizeof(char), 3, toWrite);
					}
					fwrite("@", sizeof(char), 1, toWrite);
					fwrite("\n", sizeof(char), 1, toWrite);
					free(linkFamNumber);
				}
			}
			//write wife
			if (theWife != NULL){
				if (compareEntireIndi(theWife, linkIndiData) == 0){
					fwrite("1 WIFE @IND", sizeof(char), 11, toWrite);
					char * linkFamNumber = malloc(sizeof(char) * 50);
					sprintf(linkFamNumber, "%d", linkFamNum);
					if (linkFamNum < 10){
						fwrite(linkFamNumber, sizeof(char), 1, toWrite);
					} else if (linkFamNum >= 10 && linkFamNum < 100){
						fwrite(linkFamNumber, sizeof(char), 2, toWrite);
					} else {
						fwrite(linkFamNumber, sizeof(char), 3, toWrite);
					}
					fwrite("@", sizeof(char), 1, toWrite);
					fwrite("\n", sizeof(char), 1, toWrite);
					free(linkFamNumber);
				}
			}
			linkFamNum++;
			linkIndi = linkIndi->next;
		}
		
		
		int chilIndiNum = 1;
		Node * chil = fam->children.head;
		while (chil != NULL){
			Individual * chilData = chil->data;
			Node * chilIndi = (*obj).individuals.head;
			chilIndiNum = 1;
			while (chilIndi != NULL){
				Individual * chilIndiData = chilIndi->data;
				if (compareEntireIndi(chilIndiData, chilData) == 0){
					fwrite("1 CHIL @IND", sizeof(char), 11, toWrite);
					char * chilIndiNumber = malloc(sizeof(char) * 50);
					sprintf(chilIndiNumber, "%d", chilIndiNum);
					if (chilIndiNum < 10){
						fwrite(chilIndiNumber, sizeof(char), 1, toWrite);
					} else if (chilIndiNum >= 10 && chilIndiNum < 100){
						fwrite(chilIndiNumber, sizeof(char), 2, toWrite);
					} else {
						fwrite(chilIndiNumber, sizeof(char), 3, toWrite);
					}
					fwrite("@", sizeof(char), 1, toWrite);
					fwrite("\n", sizeof(char), 1, toWrite);
					free(chilIndiNumber);
				}
				chilIndiNum++;
				chilIndi = chilIndi->next;
			}			
			chil = chil->next;
		}		

		numFam++;
		free(numberFam);
		currFam = currFam->next;
	}

	fwrite("0 TRLR\n", sizeof(char), 7, toWrite);
	fclose(toWrite);
	return theError;
}

ErrorCode validateGEDCOM(const GEDCOMobject* obj){	
	if (obj == NULL){
		return INV_GEDCOM;
	}
	if (obj->header == NULL){
		return INV_GEDCOM;
	}
	if (obj->submitter == NULL){
		return INV_GEDCOM;
	}
	if (strcmp(obj->header->source, "") == 0){
		return INV_HEADER;
	} 
	if (obj->header->submitter == NULL){
		return INV_HEADER;
	}
	if (obj->header->gedcVersion < 0){
		return INV_HEADER;
	}
	if (obj->header->encoding < 0 || obj->header->encoding > 3){
		return INV_HEADER;
	}
	if (strcmp(obj->submitter->submitterName, "") == 0){
		return INV_RECORD;
	}
	if (strlen(obj->header->source) > 200){
		return INV_RECORD;
	}
	if (strlen(obj->submitter->submitterName) > 60){
		return INV_RECORD;
	}
	if (strlen(obj->submitter->address) > 200){
		return INV_RECORD;
	}
	Node * curr = NULL;
	Node * curr2 = NULL;
	Node * curr3 = NULL;
	Node * curr4 = NULL;
	
	
	
	//check individuals first and last name
	curr = obj->individuals.head;
	while (curr != NULL){
		Individual * temp = curr->data;
		if (strlen(temp->givenName) > 200){
			return INV_RECORD;
		} 
		if (strlen(temp->surname) > 200){
			return INV_RECORD;
		}
		//other fields
		Node * curr2 = temp->otherFields.head;
		while (curr2 != NULL){
			Field * temp2 = curr2->data;
			if (strlen(temp2->tag) > 200){
				return INV_RECORD;
			} 
			if (strlen(temp2->value) > 200){
				return INV_RECORD;
			}
			curr2 = curr2->next;
		}
		//events
		curr3 = temp->events.head;
		while (curr3 != NULL){
			Event * temp3 = curr3->data;
			if (strlen(temp3->type) > 200){
				return INV_RECORD;
			}
			if (strlen(temp3->date) > 200){
				return INV_RECORD;
			}
			if (strlen(temp3->place) > 200){
				return INV_RECORD;
			}
			curr4 = temp3->otherFields.head;
			//other fields of events
			while (curr4 != NULL){
				Field * temp4 = curr4->data;
				if (strlen(temp4->tag) > 200){
					return INV_RECORD;
				} 
				if (strlen(temp4->value) > 200){
					return INV_RECORD;
				}
				curr4 = curr4->next;
			}
			curr3 = curr3->next;
		}
		curr = curr->next;
	}
	
	curr = NULL;
	curr2 = NULL;
	curr3 = NULL;
	curr4 = NULL;

	//other fields for families
	curr = obj->families.head;
	while (curr != NULL){
		Family * temp = curr->data;
		curr2 = temp->otherFields.head;
		while (curr2 != NULL){
			Field * temp2 = curr2->data;
			if (strlen(temp2->tag) > 200){
				return INV_RECORD;
			} 
			if (strlen(temp2->value) > 200){
				return INV_RECORD;
			}
			curr2 = curr2->next;
		}		
		//events for families
		curr3 = temp->events.head;
		while (curr3 != NULL){
			Event * temp3 = curr3->data;
			if (strlen(temp3->type) > 200){
				return INV_RECORD;
			}
			if (strlen(temp3->date) > 200){
				return INV_RECORD;
			}
			if (strlen(temp3->place) > 200){
				return INV_RECORD;
			}
			Node * curr4 = temp3->otherFields.head;
			//other fields of events for families
			while (curr4 != NULL){
				Field * temp4 = curr4->data;
				if (strlen(temp4->tag) > 200){
					return INV_RECORD;
				} 
				if (strlen(temp4->value) > 200){
					return INV_RECORD;
				}
				curr4 = curr4->next;
			}
			curr3 = curr3->next;
		}
		curr = curr->next;
	}	
	
	
	curr = NULL;
	curr2 = NULL;
	curr3 = NULL;
	curr4 = NULL;
	
	//check dereferncing of families
	curr = obj->families.head;
	while (curr != NULL){
		if (curr->data == NULL){
			return INV_RECORD;
		} else {
			Family * temp = curr->data;
			curr2 = temp->otherFields.head;
			while (curr2 != NULL){
				if (curr2->data == NULL){
					return INV_RECORD;
				}
				curr2 = curr2->next;
			}
			curr3 = temp->children.head;
			while (curr3 != NULL){
				if (curr3->data == NULL){
					return INV_RECORD;
				}
				curr3 = curr3->next;
			}
			curr4 = temp->events.head;
			while (curr4 != NULL){
				if (curr4->data == NULL){
					return INV_RECORD;
				}
				curr4 = curr4->next;
			}
		}
		curr = curr->next;
	}
	
	
	curr = NULL;
	curr2 = NULL;
	curr3 = NULL;
	curr4 = NULL;
	
	//check dereferencing of individuals
	curr = obj->individuals.head;
	while (curr != NULL){
		if (curr->data == NULL){
			return INV_RECORD;
		} else {
			Individual * temp = curr->data;
			curr2 = temp->otherFields.head;
			while (curr2 != NULL){
				if (curr2->data == NULL){
					return INV_RECORD;
				}
				curr2 = curr2->next;
			}
			curr3 = temp->families.head;
			while (curr3 != NULL){
				if (curr3->data == NULL){
					return INV_RECORD;
				}
				curr3 = curr3->next;
			}
			curr4 = temp->events.head;
			while (curr4 != NULL){
				if (curr4->data == NULL){
					return INV_RECORD;
				}
				curr4 = curr4->next;
			}
		}
		curr = curr->next;
	}
	
	curr = NULL;
	curr2 = NULL;
	curr3 = NULL;
	curr4 = NULL;	
	
	return OK;
}

void deleteGeneration(void* toBeDeleted){
	if (toBeDeleted == NULL){
		return;
	}
	List * toDelete = (List *)(toBeDeleted);
	clearList(toDelete);
	free(toDelete);
}

char* printGeneration(void* toBePrinted){
	char * toPrint = malloc(sizeof(char) * 100);
	strcpy(toPrint,"");
	if (toBePrinted == NULL){
		return toPrint;
	}
	
	int size = 0;
	strcat(toPrint,"\n***Generation***\n");
	List * test = (List*)(toBePrinted);
	Node * curr = test->head;
	size = strlen(toPrint) + 100;
	while (curr != NULL){
		Individual * personTest = curr->data;
		size = size + strlen(personTest->givenName) + 20;
		toPrint = realloc(toPrint, size);
		strcat(toPrint, personTest->givenName);
		size = size + strlen(personTest->surname) + 20;
		toPrint = realloc(toPrint, size);
		strcat(toPrint, personTest->surname);
		strcat(toPrint, "\n");
		curr = curr->next;
	}
	return toPrint;
}


List getAncestorListN(const GEDCOMobject* familyRecord, const Individual* person, int maxGen){
	
	List AncestorsList = initializeList(&printGeneration, &deleteGeneration, &compareGenerations);
	
	if (maxGen == 0){
		maxGen = 200;
	}
	
	if (maxGen < 0){
		return AncestorsList;
	}
	
	if (person == NULL){
		AncestorsList.head = NULL;
		return AncestorsList;
	}
	
	int num = -1;
	
	int maxCount = 1;
	while (maxCount <= maxGen){
		List * newList = malloc(sizeof(List));
		*newList = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
		insertBack(&AncestorsList, newList);
		maxCount++;
	}
	
	getChildrenAncestors(&AncestorsList, person, &num, maxGen);
	
	putLNAtBack(&AncestorsList);
	
	Node * temp = NULL;
	Node * curr = AncestorsList.head;
	while (curr != NULL){
		List * aList = curr->data;
		temp = curr->next;
		if (getLength(*aList) == 0){
			deleteDataFromList2(&AncestorsList, curr);
			deleteGeneration(aList);
		}
		curr = temp;
	}
	
	return AncestorsList;
	
}

int compareGenerations(const void* first,const void* second){
	List * one = (List *)first;
	List * two = (List *)second;
	Node * curr3 = one->head;
	Node * curr4 = two->head;
	
	
	int m = 0;
	
	while (curr3 != NULL && curr4 != NULL){
		m = compareEntireIndi(curr3->data, curr4->data);
		if (m != 0){
			return 1;
		}
		curr4 = curr4->next;
		curr3 = curr3->next;
	}
	return 0;		
}

char* indToJSON(const Individual* ind){
	char * string = malloc(sizeof(char) * 100);
	strcpy(string, "");
	if (ind == NULL){
		return string;
	}
	char * sex = malloc(sizeof(char) * 100);
	
	strcpy(sex, "");
	Node * curr = ind->otherFields.head;
	
	while (curr != NULL){
		Field * theField = curr->data;
		if (strcmp(theField->tag, "SEX") == 0){
			strcpy(sex, theField->value);
		}
		curr = curr->next;
	}
	
	int size = 1;
	Node * hi;
	Node * heyo = ind->families.head;
	while (heyo != NULL){
		Family * mirTemp = heyo->data;
		if (compareEntireIndi(ind, mirTemp->husband) == 0){
			if (mirTemp->wife != NULL){
				size++;
			}
			hi = mirTemp->children.head;
			while (hi != NULL){
				size++;
				hi = hi->next;
			}
		} else if (compareEntireIndi(ind, mirTemp->wife) == 0){
			if (mirTemp->husband != NULL){
				size++;
			}
			hi = mirTemp->children.head;
			while (hi != NULL){
				size++;
				hi = hi->next;
			}
		}
		heyo = heyo->next;
	}

	char * famSize = malloc(sizeof(char) * 100);
	sprintf(famSize, "%d", size);	
	strcpy(string, "{\"givenName\":\"");
	strcat(string, ind->givenName);
	strcat(string, "\",\"surname\":\"");
	strcat(string, ind->surname);
	strcat(string, "\",\"sex\":\"");
	strcat(string, sex);
	strcat(string, "\",\"familySize\":\"");
	strcat(string, famSize);
	strcat(string, "\"}");
	return string;
}

Individual* JSONtoInd(const char* str){
	if (str == NULL){
		return NULL;
	}

	Individual * person = malloc(sizeof(Individual));
	person->givenName = malloc(sizeof(char) * 256);
	person->surname = malloc(sizeof(char) * 256);
	strcpy(person->givenName, "");
	strcpy(person->surname, "");
	person->otherFields = initializeList(&printField, &deleteField, &compareFields);
	person->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
	person->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
	
	char * string = malloc(sizeof(char) * strlen(str) +1);
	strcpy(string, str);
	char * token;
	char * first = malloc(sizeof(char) * 256);
	char * last = malloc(sizeof(char) * 256);
	strcpy(first, "");
	strcpy(last, "");
	token = strtok(string, ":");
	if (token == NULL){
		deleteIndividual(person);
		free(last);
		free(string);
		free(first);
		return NULL;
	}
	token = strtok(NULL, ",");
	if (token == NULL){
		deleteIndividual(person);
		free(last);
		free(string);
		free(first);
		return NULL;
	}
	if (strcmp(token, "\"\"") != 0){
		strcpy(first, token);
		removeQuotes(first);
	}
	strcpy(person->givenName, first);
	
	token = strtok(NULL, ":");
	if (token == NULL){
		deleteIndividual(person);
		free(last);
		free(string);
		free(first);
		return NULL;
	}
	token = strtok(NULL, "}");
	if (token == NULL){
		deleteIndividual(person);
		free(last);
		free(string);
		free(first);
		return NULL;
	}
	if (strcmp(token, "\"\"") != 0){
		strcpy(last, token);
		removeQuotes(last);
	}
	
	strcpy(person->surname, last);
	
	free(last);
	free(string);
	free(first);
	
	return person;
}

GEDCOMobject* JSONtoGEDCOM(const char* str){
	GEDCOMobject * obj = NULL;
	if (str == NULL){
		return NULL;
	}
	if (strcmp(str, "") == 0){
		return NULL;
	}
	
	char * string = malloc(sizeof(char) * strlen(str) +1);
	strcpy(string, str);
	char * token;
	
	//source
	token = strtok(string, ":");
	if (token == NULL){
		free(string);
		return NULL;
	}
	token = strtok(NULL, ",");
	if (token == NULL){
		free(string);
		return NULL;
	}
	char * theSource = malloc(sizeof(char) * 256);
	strcpy(theSource, "");
	strcpy(theSource, token);
	removeQuotes(theSource);
	
	//gedc version
	token = strtok(NULL, ":");
	if (token == NULL){
		free(string);
		free(theSource);
		return NULL;
	}
	token = strtok(NULL, ",");
	if (token == NULL){
		free(string);
		free(theSource);
		return NULL;
	}
	char * theVersion = malloc(sizeof(char) * 256);
	strcpy(theVersion, "");
	strcpy(theVersion, token);
	removeQuotes(theVersion);
	
	//encoding
	token = strtok(NULL, ":");
	if (token == NULL){
		free(theVersion);
		free(string);
		free(theSource);
		return NULL;
	}
	token = strtok(NULL, ",");
	if (token == NULL){
		free(string);
		free(theVersion);
		free(theSource);
		return NULL;
	}
	char * theEncoding = malloc(sizeof(char) * 256);
	strcpy(theEncoding, "");
	strcpy(theEncoding, token);
	removeQuotes(theEncoding);
	
	//submitter name
	token = strtok(NULL, ":");
	if (token == NULL){
		free(theVersion);
		free(string);
		free(theEncoding);
		free(theSource);
		return NULL;
	}
	token = strtok(NULL, ",");
	if (token == NULL){
		free(string);
		free(theVersion);
		free(theEncoding);
		free(theSource);
		return NULL;
	}
	char * theSubName = malloc(sizeof(char) * 256);
	strcpy(theSubName, "");
	strcpy(theSubName, token);
	removeQuotes(theSubName);
	
	//submitter address
	token = strtok(NULL, ":");
	if (token == NULL){
		free(theVersion);
		free(string);
		free(theSubName);
		free(theEncoding);
		free(theSource);
		return NULL;
	}
	token = strtok(NULL, "}");
	if (token == NULL){
		free(string);
		free(theVersion);
		free(theSubName);
		free(theEncoding);
		free(theSource);
		return NULL;
	}
	char * theSubAddress = malloc(sizeof(char) * 256);
	strcpy(theSubAddress, "");
	strcpy(theSubAddress, token);
	removeQuotes(theSubAddress);
	
	obj = malloc(sizeof(GEDCOMobject));
	obj->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
	obj->individuals = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
	obj->submitter = malloc(sizeof(Submitter) + 256 * sizeof(char));
	obj->header = malloc(sizeof(Header));
	
	obj->submitter->otherFields = initializeList(&printField, &deleteField, &compareFields);
	strcpy(obj->submitter->address, "");
	strcpy(obj->submitter->submitterName, "");
		
	obj->header->otherFields = initializeList(&printField, &deleteField, &compareFields);
	obj->header->submitter = obj->submitter;
	strcpy(obj->header->source, "");
	obj->header->encoding = -1;
	obj->header->gedcVersion = 0;
	
	strcpy(obj->header->source, theSource);
	float version = atof(theVersion);
	obj->header->gedcVersion = version;
	CharSet encoding = getEncoding(theEncoding);
	obj->header->encoding = encoding;
	strcpy(obj->submitter->address, theSubAddress);
	strcpy(obj->submitter->submitterName, theSubName);
	
	free(string);
	free(theSource);
	free(theVersion);
	free(theEncoding);
	free(theSubName);
	free(theSubAddress);
	
	return obj;
}

void addIndividual(GEDCOMobject* obj, const Individual* toBeAdded){
	if (obj == NULL){
		return;
	}
	if (toBeAdded == NULL){
		return;
	}
	insertBack(&obj->individuals, (void*)toBeAdded);
}

char* iListToJSON(List iList){
	char * string = malloc(sizeof(char) * 256);
	strcpy(string, "[]");
	if (iList.head == NULL){
		return string;
	}
	strcpy(string, "[");
	int size = 100;
	Node * curr = iList.head;
	while (curr != NULL){
		Individual * person = curr->data;
		char * indJson = indToJSON(person);
		size = size + strlen(indJson) + 10;
		string = realloc(string, size);
		strcat(string, indJson);
		if (curr->next != NULL){
			strcat(string, ",");
		}
		curr = curr->next;
		free(indJson);
	}
	strcat(string, "]");
	return string;
}

char* gListToJSON(List gList){
	char * string = malloc(sizeof(char) * 256);
	strcpy(string, "[]");
	if (gList.head == NULL){
		return string;
	}
	strcpy(string, "[");
	int size = 100;
	Node * curr = gList.head;
	while (curr != NULL){
		List * theList = curr->data;
		if (theList->head != NULL){
			char * iListJson = iListToJSON(*theList);
			size = size + strlen(iListJson) + 10;
			string = realloc(string, size);
			strcat(string, iListJson);
			free(iListJson);
			if (curr->next != NULL){
				strcat(string, ",");
			}
		}
		curr = curr->next;
	}
	strcat(string, "]");
	return string;
}
