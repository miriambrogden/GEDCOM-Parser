#include "GEDCOMparser.h"
#include "GEDCOMutilities.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

ErrorCode checkFile(char* fileName, int * objectLine){
	char * buffer = malloc(sizeof(char) * 1000);
	char * temporary = malloc(sizeof(char) * 1000);
	strcpy(temporary, "");
	strcpy(buffer, "");
	int flag = 0;
	int last = strlen(fileName);
	FILE * fptr = fopen(fileName, "r");
	char theFile[last+1];
	for (int i = 0; i < strlen(fileName) +1; i++){
		theFile[i] = tolower(fileName[i]);
	}
	//check for null file pointer (file does not exist)
    if (fptr == NULL){	
		//fclose(fptr);
		free(temporary);
		free(buffer);
        return INV_FILE;
    } else {
		//check if file is empty
        int size = 0;
        fseek(fptr, 0, SEEK_END);
        size = ftell(fptr);
        fseek(fptr,0,SEEK_SET);
        if (size == 0){
			fclose(fptr);
			free(temporary);
			free(buffer);
            return INV_FILE;
        } else {
			//check if file extension is ged
            if (theFile[last-4] == '.' && theFile[last-3] == 'g' && theFile[last-2]=='e' && theFile[last-1]=='d'){
				ErrorCode value;
				value = newFgets(buffer, fptr);
				int count = 0;
				while (strcmp(buffer, "") != 0){
					if (value == INV_HEADER){
						(*objectLine) = count + 1;
						flag = 1;
					}
					free(buffer);
					buffer = malloc(sizeof(char) * 1000);
					strcpy(buffer,"");
					value = newFgets(buffer,fptr);
					if (strcmp(temporary, buffer) == 0){
						fclose(fptr);
						free(buffer);
						free(temporary);
						return INV_GEDCOM;
					}
					strcpy(temporary, buffer);
					count++;
				}
				if (flag == 0){
					fclose(fptr);
					free(temporary);
					free(buffer);
					return OK;
				} else {
					fclose(fptr);
					free(buffer);
					free(temporary);
					return INV_RECORD;
				}
			} else {
				fclose(fptr);
				free(buffer);
				free(temporary);
				return INV_FILE;
			}
		} 
	}
	
}

void removeNewline(char * line){
    if(line[strlen(line)-1] == '\n' || line[strlen(line)-1] == '\r'){
        line[strlen(line)-1] = '\0';
    }
    if(strlen(line) > 2){
		if (line[strlen(line)-2] == '\n' || line[strlen(line)-2] == '\r'){
			line[strlen(line)-2] = '\0';
		}
	}
}

void deleteFile(void* toBeDeleted){
	FileLineData * fileData = (FileLineData *)(toBeDeleted);
	free(fileData->lineInfo);
	fileData->lineInfo = NULL;
    free(fileData);
    fileData = NULL;
}

int compareFile(const void* first,const void* second){
	FileLineData * a = (FileLineData*)first;
	FileLineData * b = (FileLineData*)second;
	
	if (a->lineNum > b->lineNum){
		return 1;
	} else if (a->lineNum < b->lineNum){
		return -1;
	} 
	return 0;
}

char* printFile(void* toBePrinted){
	char * string = malloc(sizeof(char) * 1000);
	FileLineData * F1 = (FileLineData*)(toBePrinted);
	char * levelVal = malloc(sizeof(int));
	sprintf(levelVal,"%d",F1->level);
	strcpy(string,levelVal);
	strcat(string, "-");
	strcat(string,F1->lineInfo);
	free(levelVal);
	levelVal = NULL;
    return string;
}

enum eCode createHeader(List * fileContents, int * lineNumber, char ** source, float * gedcVersion, CharSet * encoding){
		Node * curr = fileContents->head;
		FileLineData * fileInfo;
		int found = 0;
		char * token;
		char * tempString = malloc(sizeof(char) * 256);
	
		//find the source
		while (curr != NULL){
			fileInfo = curr->data;
			strcpy(tempString,fileInfo->lineInfo);
			token = strtok(tempString, " ");
			if (token != NULL){
			// look for 1 SOUR
            if (strcmp(token,"SOUR") == 0 && fileInfo->level == 1){ 
				if (found == 0){
					token = strtok(NULL, " ");
					if (token != NULL){
						strncpy(*source,token,strlen(token)+1);
						checkContinuation(fileInfo->lineNum, fileContents, *source);
						fileInfo->used = true;
						found++;
					}
				} else {
					// make sure there is only one source
					*lineNumber = fileInfo->lineNum;  
					return INV_HEADER;
				}
			}
			}
		    curr = curr->next;
		}
		free(tempString);
		tempString = NULL;
		
		
		//make sure there actually is a source, and it's not still an empty string
		if (strcmp(*source,"") == 0){  
			*lineNumber = getLastLineHeader(fileContents);
			return INV_HEADER;
		}
		
		//find the gedcversion
		char * tempString2= malloc(sizeof(char) * 256);
		found = 0;
		curr = fileContents->head;
		Node * GEDC = NULL;
		while (curr != NULL){
			fileInfo = curr->data;
			strcpy(tempString2,fileInfo->lineInfo);
			token = strtok(tempString2, " ");
            if (token != NULL && strcmp(token,"GEDC") == 0 && fileInfo->level == 1){  //find a line that says 1 GEDC
				GEDC = curr;
				curr = NULL;
				fileInfo->used = true;
			} else {
				curr = curr->next;
			}
		}
		if (GEDC == NULL){
			free(tempString2);
			tempString2 = NULL;
			*lineNumber = getLastLineHeader(fileContents);
			return INV_HEADER;
		} 
		fileInfo = GEDC->data;
		char * temp = malloc(sizeof(char) * 100);
		strcpy(temp, "");
		while (fileInfo->level > 0){
			fileInfo = GEDC->data;
			strcpy(tempString2,"");
			strcpy(tempString2,fileInfo->lineInfo);
			token = strtok(tempString2, " ");
			if (token != NULL && strcmp(token,"VERS") == 0 && fileInfo->level == 2){ //find a line that says 2 VERS underneath it
				if (found == 0){
					token = strtok(NULL, " ");
					if (token != NULL){
						strncpy(temp,token,strlen(token)+1);
					}
					int versOk = checkVersion(temp);
					if (versOk == 0){
						*lineNumber = fileInfo->lineNum;  // make sure there is only one VERS for GEDC
						fileInfo->used = true;
						free(temp);
						temp = NULL;
						free(tempString2);
						tempString2 = NULL;
						return INV_HEADER;
					}
					*gedcVersion = atof(temp); //convert string to float
					fileInfo->used = true;
					found++;
				} else {
					*lineNumber = fileInfo->lineNum;  // make sure there is only one VERS for GEDC
					free(temp);
					temp = NULL;
					free(tempString2);
					tempString2 = NULL;
					return INV_HEADER;
				}
			}
			GEDC = GEDC->next;
		}
		free(temp);
		temp = NULL;
		free(tempString2);
		tempString2 = NULL;
		
		
		//get encoding
		found = 0;
		char * tempString3 = malloc(sizeof(char) * 256);
		curr = fileContents->head;
		while (curr != NULL){
			fileInfo = curr->data;
			strcpy(tempString3,fileInfo->lineInfo);
			token = strtok(tempString3, " ");
			// look for 1 CHAR
            if (token != NULL && strcmp(token,"CHAR") == 0 && fileInfo->level == 1){ 
				if (found == 0){
					token = strtok(NULL, " ");
					CharSet encode = -1;
					if (token != NULL){
						encode = getEncoding(token);
						
					} 
					if (encode != -1){
						*encoding = encode;
						fileInfo->used = true;
					} 
					found++;
				} else {
					// make sure there is only one source
					*lineNumber = fileInfo->lineNum;  
					return INV_HEADER;
				}
			}
		    curr = curr->next;
		}
		free(tempString3);
		tempString3 = NULL;

		//make sure there actually is an encoding, and it's not still an empty string
		if (*encoding == -1){  
			*lineNumber = getLastLineHeader(fileContents);
			return INV_HEADER;
		}
	
		return OK;
}

int checkVersion(char * string){
	
	if (strcmp(string, "") == 0){
		return 0;
	}

//make sure it's all dots and numbers
	for (int k = 0; k<strlen(string); k++){
		if (string[k] != '.'){
			if (!isdigit(string[k])){
				return 0;
			}
		}
	}
	return 1;
}

CharSet getEncoding(char * string){
	if (strcmp(string,"ANSEL") == 0){
		return ANSEL;
	} else if (strcmp(string,"UTF-8") == 0){
		return UTF8;
	} else if (strcmp(string,"UNICODE") == 0){
		return UNICODE;
	} else if (strcmp(string,"ASCII") == 0){
		return ASCII;
	} else {
		return -1;
	}
}

char * getEncodingString(CharSet encoding){
	if (encoding == ANSEL){ 
		return "ANSEL";
	} else if (encoding == UTF8){
		return "UTF-8";
	} else if (encoding == UNICODE){
		return "UNICODE";
	} else if (encoding == ASCII){
		return "ASCII";
	} else {
		return "No encoding found";
	}
}


ErrorCode getOtherFields(List fileContents, List * otherFields, int * lineNum){
	Node * curr = fileContents.head->next;
	char * token;
	Field * theField = NULL;
	char * temp = malloc(sizeof(char) * 256);
	char * tag = malloc(sizeof(char) * 256);
	FileLineData * fileInfo;
	int levelNum = 0;
	fileInfo = curr->data;
		while (fileInfo->level != 0){
			fileInfo = curr->data;
			strcpy(temp,fileInfo->lineInfo);
			levelNum = fileInfo->level;
			token = strtok(temp, " ");
			if (token != NULL){
				strcpy(tag, token);
			}
			
			if ((fileInfo->used == false) && levelNum > 0){
				theField = malloc(sizeof(Field));
				theField->tag = malloc(sizeof(char) * 256);
				theField->value = malloc(sizeof(char) * 256);
				if (token != NULL){
					strcpy(theField->tag, token);
					token = strtok(NULL,"");
				}
				if (token != NULL){
					strcpy(theField->value, token);
					insertBack(otherFields, theField);
				} else {
					*lineNum = fileInfo->lineNum;
					free(theField->tag);
					free(theField->value);
					free(theField);
					free(temp);
					free(tag);
					return INV_RECORD;
				}
			}
			
			curr = curr->next;
		}
	free(temp);
	temp = NULL;
	free(tag);
	tag = NULL;

	return OK;
}

int hasAt(char * string){
	for (int i = 0; i <strlen(string); i++){
		if (string[i] == '@'){
			return 1;
		}
	}
	return 0;
}

int getLastLineHeader(List * fileContents){
	FileLineData * fileInfo = NULL;
	Node * curr = fileContents->head;
	fileInfo = curr->data;
	removeNewline(fileInfo->lineInfo);
	if (strcmp(fileInfo->lineInfo, "HEAD") == 0 && fileInfo->level == 0){
		curr = fileContents->head->next;
	} else {
		curr = NULL;
	}
	while (curr != NULL){
		fileInfo = curr->data;
		if (fileInfo->level == 0){
			return (fileInfo->lineNum)-1;
		}
		curr = curr->next;
	}
	return -1;
}


void checkContinuation(int lineNum, List * fileContents, char * string){
	Node * curr = NULL;
	Node * temp = NULL;
	FileLineData * fileInfo = NULL;
	FileLineData * foundInfo = NULL;
	char * token;
	char * tempLine = malloc(sizeof(char) * 256);
	curr = fileContents->head;
	while (curr != NULL){ // get line that you are looking at
		fileInfo = curr->data;
		if (fileInfo->lineNum == lineNum){
			temp = curr;
			curr = NULL;
		} else {
			curr = curr->next;
		}
	}
	
	curr = temp;
	foundInfo = curr->data;
	temp = temp->next;
	fileInfo = temp->data;
	while (fileInfo->level > foundInfo->level){
		fileInfo = temp->data;
		strncpy(tempLine, fileInfo->lineInfo, strlen(fileInfo->lineInfo)+1);
		token = strtok(tempLine, " ");
		if (token != NULL){
			if (strcmp(token, "CONT") == 0){
				token = strtok(NULL, "");
				if (token != NULL){
					strcat(string, "\n");
					strcat(string, token);
					fileInfo->used = true;
				}
			}
			else if (strcmp(token, "CONC") == 0){
				token = strtok(NULL, "");
				if (token != NULL){
					strcat(string, " ");
					strcat(string, token);
					fileInfo->used = true;
				}
			}
		}
		temp = temp->next;
	}
	
	free(tempLine);
	tempLine = NULL;
}


void getHusbAndWife(List * families, List * individuals){
	Node * currFamNode = NULL;
	Node * currFamOtherNode = NULL;
	Node * currIndiNode = NULL;
	Node * currIndiOtherNode = NULL;
	Family * famData = NULL;
	Field * famOtherData = NULL;
	Individual * indiData = NULL;
	Field * indiOtherData = NULL;
	char * husbPtr = malloc(sizeof(char) * 256);
	char * wifePtr = malloc(sizeof(char) * 256);
	
	currFamNode = families->head;
	while (currFamNode != NULL){
		 famData = currFamNode->data;
		 famData->husband = NULL;
		 famData->wife = NULL;
		 strcpy(wifePtr,"");
		 strcpy(husbPtr,"");
		 currFamOtherNode = famData->otherFields.head;
		 while (currFamOtherNode != NULL){
			famOtherData = currFamOtherNode->data;
			if (strcmp(famOtherData->tag, "HUSB") == 0){
				if (isPointer(famOtherData->value) == 1){
					strcpy(husbPtr, famOtherData->value);
				}
			}
			if (strcmp(famOtherData->tag, "WIFE") == 0){
				if (isPointer(famOtherData->value) == 1){
					strcpy(wifePtr, famOtherData->value);
				}
			}
			currFamOtherNode = currFamOtherNode->next;
		}
		currIndiNode = individuals->head;
		while (currIndiNode != NULL){
			indiData = currIndiNode->data;
			currIndiOtherNode = indiData->otherFields.head;
			while (currIndiOtherNode != NULL){
				indiOtherData = currIndiOtherNode->data;
				if (strcmp(indiOtherData->tag, husbPtr) == 0 && strcmp(indiOtherData->value, "INDI") == 0){
					famData->husband = indiData;
				}
				if (strcmp(indiOtherData->tag, wifePtr) == 0 && strcmp(indiOtherData->value, "INDI") == 0){
					famData->wife = indiData;
				}
				currIndiOtherNode = currIndiOtherNode->next;
			}
			currIndiNode = currIndiNode->next;
		}
		currFamNode = currFamNode->next;
	}
	free(husbPtr);
	husbPtr = NULL;	 
	free(wifePtr);
	wifePtr = NULL;	
	
}

void getRelFams(List * families, List * individuals){
	Node * currFam = NULL;
	Node * currIndi = NULL;
	Node * currFamOther = NULL;
	Node * currIndiOther = NULL;
	Individual * indiData = NULL;
	Family * famData = NULL;
	Field * indiOtherData = NULL;
	Field * famOtherData = NULL;
	char * famPtr = malloc(sizeof(char) * 256);
	strcpy(famPtr, "");
	
	currIndi = individuals->head;
	while (currIndi != NULL){
		indiData = currIndi->data;
		List relatedFamilies = initializeList(&printFamily, &dummyDelete, &compareFamilies);
		currIndiOther = indiData->otherFields.head;
		while (currIndiOther != NULL){
			indiOtherData = currIndiOther->data;
			if (strcmp(indiOtherData->tag, "FAMS") == 0 || strcmp(indiOtherData->tag, "FAMC") == 0){
				strncpy(famPtr, indiOtherData->value, strlen(indiOtherData->value) + 1);
				currFam = families->head;
				while (currFam != NULL){
					famData = currFam->data;
					currFamOther = famData->otherFields.head;
					while (currFamOther != NULL){
					famOtherData = currFamOther->data;
						if (strcmp(famPtr, famOtherData->tag) == 0 && strcmp(famOtherData->value, "FAM") == 0){
							insertBack(&relatedFamilies, famData);
						}
						currFamOther = currFamOther->next;
					}
					currFam = currFam->next;
				}				
			}
			currIndiOther = currIndiOther->next;
		}
		indiData->families = relatedFamilies;
		currIndi = currIndi->next;
	}
	free(famPtr);
	famPtr = NULL;
}


void getChildren(List * families, List * individuals){
	Node * currFam = NULL;
	Node * currIndi = NULL;
	Node * currFamOther = NULL;
	Node * currIndiOther = NULL;
	Individual * indiData = NULL;
	Family * famData = NULL;
	Field * indiOtherData = NULL;
	Field * famOtherData = NULL;
	char * childPtr = malloc(sizeof(char) * 256);
	strcpy(childPtr, "");
	
	currFam = families->head;
	while (currFam != NULL){
		List childrenList = initializeList(&printIndividual, &dummyDelete, &compareIndividuals);
		famData = currFam->data;
		currFamOther = famData->otherFields.head;
		while (currFamOther != NULL){
			famOtherData = currFamOther->data;
			if(strcmp(famOtherData->tag, "CHIL") == 0){
				strncpy(childPtr, famOtherData->value, strlen(famOtherData->value) + 1);
				currIndi = individuals->head;
				while (currIndi != NULL){
					indiData = currIndi->data;
					currIndiOther = indiData->otherFields.head;
					while (currIndiOther != NULL){
						indiOtherData = currIndiOther->data;
						if(strcmp(childPtr, indiOtherData->tag) == 0 && strcmp(indiOtherData->value, "INDI") == 0){
							insertBack(&childrenList, indiData);
						}
						currIndiOther = currIndiOther->next;
					}
					currIndi = currIndi->next;
				}
			}
			currFamOther = currFamOther->next;
		}
		currFam = currFam->next;
		famData->children = childrenList;
	}
								
	free(childPtr);
	childPtr = NULL;
}

ErrorCode getFamiliesList(List fileContents, List * families){
	Node * curr = fileContents.head;
	Node * current = NULL;
	char * token = NULL;
	char * tokenTwo = NULL;
	char * tokenThree = NULL;
	char * temp = malloc(sizeof(char) * 256);
	char * tempTwo = malloc(sizeof(char) * 256);
	char * tempThree = malloc(sizeof(char) * 256);
	FileLineData * fileInfo = NULL;
	FileLineData * fileData = NULL;
	int isPointerVal = 0;
	int isLineEvent = 0;
	Event * anEvent = NULL;
	
	while (curr != NULL){
		fileInfo = curr->data;
		strncpy(temp,fileInfo->lineInfo, strlen(fileInfo->lineInfo) +1);
		token = strtok(temp, " ");
		if(token != NULL){
			isPointerVal = isPointer(token);
		}
		if (isPointerVal == 1){
			token = strtok(NULL, "");
			if (token != NULL){
			if ((strcmp(token, "FAM") == 0) && fileInfo->level == 0){
				Family * famRecord = malloc(sizeof(Family));
				List famRecEvents = initializeList(&printEvent,&deleteEvent,&compareEvents);
				List famOtherFields = initializeList (&printField, &deleteField, &compareFields);
				Field * tempField = malloc(sizeof(Field));
					tempField->tag = malloc(sizeof(char) * 256);
					tempField->value = malloc(sizeof(char) * 256);
					char * tempStringField = malloc(sizeof(char) * 256);
					strncpy(tempStringField, fileInfo->lineInfo, strlen(fileInfo->lineInfo) +1);
					char * tokenField;
					tokenField = strtok(tempStringField, " ");
					if (tokenField != NULL){
						strncpy(tempField->tag, tokenField, strlen(tokenField) + 1);
						tokenField = strtok(NULL, "");
						if (tokenField != NULL){
							strncpy(tempField->value, tokenField, strlen(tokenField) + 1);
							insertBack(&famOtherFields, tempField);
						}
					}
					free(tempStringField);
					tempStringField = NULL;				
				current = curr->next;
				fileData = current->data;
				while (fileData->level > 0){
					strcpy(tempTwo, fileData->lineInfo);
					tokenTwo = strtok(tempTwo, " ");
					if (tokenTwo != NULL){
						isLineEvent = isFamEvent(tokenTwo);
					}
					//is this line an event? 1 is yes, 0 is no
					if (isLineEvent == 1){
						anEvent = malloc(sizeof(Event));
						List EventOtherFields = initializeList(&printField, &deleteField, &compareFields);
						strcpy(anEvent->type, tokenTwo);
						anEvent->date = malloc(sizeof(char) * 256);
						anEvent->place = malloc(sizeof(char) * 256);
						strcpy(anEvent->date, "");
						strcpy(anEvent->place, "");
						insertBack(&famRecEvents, anEvent);
						Node * tempCurrent;
						tempCurrent = current->next;
						FileLineData * tempFileData;
						tempFileData = tempCurrent->data;
						if (tempFileData->level == 2){
							current = current->next;
						}
						while (tempFileData->level > 1){
							strcpy(tempThree, tempFileData->lineInfo);
							tokenThree = strtok(tempThree, " ");
							if (tokenThree != NULL && strcmp(tokenThree, "DATE") == 0){
								tokenThree = strtok(NULL, "");
								if (tokenThree != NULL){
									strncpy(anEvent->date, tokenThree, strlen(tokenThree) + 1);
								}
							}
							else if (strcmp(tokenThree, "PLAC") == 0){
								tokenThree = strtok(NULL, "");
								if (tokenThree != NULL){
									strncpy(anEvent->place, tokenThree, strlen(tokenThree) + 1);
									checkContinuation(tempFileData->lineNum, &fileContents, anEvent->place);
								}
							} else {
								if(tempFileData->used != true){
									Field * aField = malloc(sizeof(Field));
									aField->tag = malloc(sizeof(char) * 256);
									aField->value = malloc(sizeof(char) * 256);
									strcpy(aField->tag, tokenThree);
									tokenThree = strtok(NULL, "");
									if(tokenThree != NULL){
										strcpy(aField->value, tokenThree);
										insertBack(&EventOtherFields, aField);
									}
								}
							}
							tempCurrent = tempCurrent->next;
							tempFileData = tempCurrent->data;
						}
						anEvent->otherFields = EventOtherFields;
						// not an event so put it in other fields
					} else {
						if (fileData->used != true){
							Field * anOtherField = malloc(sizeof(Field));
							anOtherField->tag = malloc(sizeof(char) * 256);
							anOtherField->value = malloc(sizeof(char) * 256);
							strncpy(anOtherField->tag, tokenTwo, strlen(tokenTwo) +1);
							tokenTwo = strtok(NULL, "");
							if (tokenTwo != NULL){
								strncpy(anOtherField->value, tokenTwo, strlen(tokenTwo) + 1);
								insertBack(&famOtherFields, anOtherField);
							}
						}
					}
					current = current->next;
					fileData = current->data;
				}
				famRecord->events = famRecEvents;
				famRecord->otherFields = famOtherFields;
				insertBack(families, famRecord);
			}
		}
		} 
		curr = curr->next;
	}
	
	free(temp);
	temp = NULL;
	free(tempTwo);
	tempTwo = NULL;
	free(tempThree);
	tempThree = NULL;
	
	return OK;
}

ErrorCode getIndividualsList(List fileContents, List * individuals, int * lineNumber){
	Node * curr = fileContents.head;
	Node * temp = NULL;
	char * string = malloc(sizeof(char) * 256);
	char * givenName = malloc(sizeof(char) * 256);
	strcpy(givenName, "");
	char * surName = malloc(sizeof(char) * 256);
	strcpy(surName, "");
	
	char * token;
	int isLineEvent = 0;
	char * tokenTwo;
	int isPointerVal = 0;
	FileLineData * fileInfo;
	FileLineData * fileData;
	Event * anEvent;
	while (curr != NULL){
		fileInfo = curr->data;
		if (fileInfo->level == 0){
			strncpy(string, fileInfo->lineInfo, 256);
			token = strtok(string, " ");
			//check if the line is a pointer
			if (token != NULL){
				isPointerVal = isPointer(token);
			}
			if (isPointerVal == 1){
				token = strtok(NULL, "");
				if (token != NULL){
					strcpy(string,token);
				}
				//check if the line is a pointer + INDI
				if (strcmp(string, "INDI") == 0){
					Individual * individualRecord = malloc(sizeof(Individual));
					individualRecord->givenName = malloc(sizeof(char) * 256);
					individualRecord->surname = malloc(sizeof(char) * 256);
					strcpy(individualRecord->givenName, "");
					strcpy(individualRecord->surname, "");
					strcpy(string, "");
					List indiOtherFields = initializeList(&printField,&deleteField,&compareFields);
					Field * tempField = malloc(sizeof(Field));
					tempField->tag = malloc(sizeof(char) * 256);
					tempField->value = malloc(sizeof(char) * 256);
					char * tempStringField = malloc(sizeof(char) * 256);
					strncpy(tempStringField, fileInfo->lineInfo, strlen(fileInfo->lineInfo) +1);
					char * tokenField;
					tokenField = strtok(tempStringField, " ");
					if (tokenField != NULL){
						strncpy(tempField->tag, tokenField, strlen(tokenField) + 1);
						tokenField = strtok(NULL, "");
						if (tokenField != NULL){
							strncpy(tempField->value, tokenField, strlen(tokenField) + 1);
							insertBack(&indiOtherFields, tempField);
						}
					}
					free(tempStringField);
					tempStringField = NULL;
					temp = curr->next;
					fileData = temp->data;
					List indiRecEvents = initializeList(&printEvent,&deleteEvent,&compareEvents);
					
					while(fileData->level > 0){
						//find the line under that which is 1 NAME
						fileData = temp->data;
						strcpy(string, fileData->lineInfo);
						tokenTwo = strtok(string, " ");
						if (tokenTwo != NULL){
							isLineEvent = isEvent(tokenTwo);
						}
						//initialize lists
						
						//is the line 1 NAME
						if (strcmp(tokenTwo, "NAME") == 0 && fileData->level == 1){  
							tokenTwo = strtok(NULL,"");
							//get the given name and surname
							if (tokenTwo != NULL){
								getName(&givenName, &surName, tokenTwo);
								strncpy(individualRecord->givenName, givenName, strlen(givenName) +1);
								strncpy(individualRecord->surname, surName, strlen(surName) +1);
							}
							temp = temp->next;
						}
						//is the line an event
						else if (isLineEvent == 1){
							anEvent = malloc(sizeof(Event));
							List eventsOtherFields = initializeList(&printField, &deleteField, &compareFields);
							anEvent->date = malloc(sizeof(char) * 256);
							strcpy(anEvent->date, "");
							anEvent->place = malloc(sizeof(char) * 256);
							strcpy(anEvent->place, "");
							strcpy(anEvent->type,tokenTwo);
							insertBack(&indiRecEvents, anEvent);
							temp = temp->next;
							FileLineData * tempFileData = temp->data;
							char * tempData = malloc(sizeof(char) * 256);
							char * tokenData;
							while (tempFileData->level > 1){
								strcpy(tempData, tempFileData->lineInfo);
								tokenData = strtok(tempData, " ");
								if (tokenData != NULL && strcmp(tempData, "DATE") == 0 && tempFileData->level == 2){
									tokenData = strtok(NULL, "");
									if (tokenData != NULL){
										strncpy(anEvent->date, tokenData, strlen(tokenData) + 1);
										tempFileData->used = true;
									}
								} else if (strcmp(tempData, "PLAC") == 0 && tempFileData->level == 2){
									tokenData = strtok(NULL, "");
									if (tokenData != NULL){
										strncpy(anEvent->place, tokenData, strlen(tokenData) + 1);
										checkContinuation(tempFileData->lineNum, &fileContents, anEvent->place);
										tempFileData->used = true;
									}
								} else {
									if (tempFileData->used != true){
										Field * aField = malloc(sizeof(Field));
										aField->tag = malloc(sizeof(char) * 256);
										aField->value = malloc(sizeof(char) * 256);
										strncpy(aField->tag, tokenData, strlen(tokenData) + 1);
										tokenData = strtok(NULL, "");
										if (tokenData != NULL){
											strncpy(aField->value, tokenData, strlen(tokenData) + 1);
											insertBack(&eventsOtherFields, aField);
										}
									}
								}
								temp = temp->next;
								tempFileData = temp->data;
							}
							free(tempData);
							tempData = NULL;
							anEvent->otherFields = eventsOtherFields;
						//add to other fields
						} else {
							if (fileData->level > 0){
								if (fileData->used != true){
									Field * aField = malloc(sizeof(Field));
									char * tikString = malloc(sizeof(char) * 256);
									strcpy(tikString, fileData->lineInfo);
									char * tik;
									aField->tag = malloc(sizeof(char) * 256);
									aField->value = malloc(sizeof(char) * 256);
									strcpy(aField->tag, "");
									strcpy(aField->value, "");
									tik = strtok(tikString," ");
									if (tik != NULL){
										strcpy(aField->tag, tik);
									}
									tik = strtok(NULL, "");
									if (tik != NULL){
										strcpy(aField->value, tik);
									}
									insertBack(&indiOtherFields, aField);
									free(tikString);
									tikString = NULL;
								}
							}
							temp = temp->next;
						}
						individualRecord->events = indiRecEvents;
						individualRecord->otherFields = indiOtherFields;
					}
					insertBack(individuals, individualRecord);	
										
				}
			
			}
		}
		curr = curr->next;
	}
	
	free(string);
	free(givenName);
	free(surName);
	surName = NULL;
	givenName = NULL;
	string = NULL;
	return OK;
}

int isPointer(char * string){
	if (string[0] == '@' && string[strlen(string)-1] == '@'){
		return 1;
	} 
	return 0;
}

void getName(char ** givenName, char ** surName, char * string){
	char * token;
	char * temp = malloc(sizeof(char) * strlen(string) + 1);
	strcpy(temp,string);
	token = strtok(temp, "/");
	if (token == NULL){
		strcpy(*givenName,"");
	} else {
		strcpy(*givenName,token);
	}
	token = strtok(NULL, "/");
	if (token == NULL){
		strcpy(*surName,"");
	} else {
		strcpy(*surName, token);
	}
	
	char * first = malloc(sizeof(char) * 256);	
	strcpy(first, *givenName);
	if (first[strlen(first)-1] == ' '){
		first[strlen(first)-1] = '\0';
	}
	strcpy(*givenName, first);
	free(first);
	free(temp);
	temp = NULL;
}

ErrorCode getSubmOtherFields(List fileContents, List * otherFields, int lineNum, int * errorLineNum){
	Node * current = fileContents.head;
	FileLineData * fileInfo;
	FileLineData * someLine;
	Node * curr = NULL;
	char * token;
	Field * theField = NULL;
	char * temp = malloc(sizeof(char) * 256);
	char * tag = malloc(sizeof(char) * 256);
	char * pointerLine = malloc(sizeof(char) * 256);
	char * newTok;
	Field * someField = NULL;
	
	
	while(current != NULL){
		someLine = current->data;
		if (someLine->lineNum == lineNum){
			strcpy(pointerLine, someLine->lineInfo);
			someField = malloc(sizeof(Field));
			someField->tag = malloc(sizeof(char) * 256);
			someField->value = malloc(sizeof(char) * 256);
			newTok = strtok(pointerLine, " ");
			if (newTok != NULL){
				strcpy(someField->tag, newTok);
				newTok = strtok(NULL,"");
				if (newTok != NULL){
					strcpy(someField->value, newTok);
					insertBack(otherFields, someField);
				}	
			}	
		} 
		current = current->next;
	}
	free(pointerLine);
	pointerLine = NULL;
	
	current = fileContents.head;
	while(current != NULL){
		fileInfo = current->data;
		if (fileInfo->lineNum == lineNum){
			curr = current;
			current = NULL;
		} else {
			current = current->next;
		}
	}
	
	
	fileInfo = NULL;
	int levelNum = 0;
	curr = curr->next;
	fileInfo = curr->data;
		while (fileInfo->level > 0){

			strcpy(temp,"");
			strcpy(temp,fileInfo->lineInfo);
			levelNum = fileInfo->level;
			token = strtok(temp, " ");
			strcpy(tag, "");
			if (token != NULL){
				strcpy(tag, token);
			}
			if ((strcmp(tag,"SUBM") != 0) && (strcmp(tag, "NAME")!=0) && (strcmp(tag,"ADDR")!=0) && (levelNum > 0) && (fileInfo->used == false)){
				theField = malloc(sizeof(Field));
				theField->tag = malloc(sizeof(char) * 256);
				theField->value = malloc(sizeof(char) * 256);
				strcpy(theField->tag, token);
				token = strtok(NULL,"");
				if (token != NULL){
					strcpy(theField->value, token);
					insertBack(otherFields, theField);
				} else {
					free(theField->tag);
					free(theField->value);
					free(theField);
					free(temp);
					free(tag);
					*errorLineNum = fileInfo->lineNum;
					return INV_RECORD;
				}
			}
			
			curr = curr->next;
			fileInfo = curr->data;
			
		}
	free(temp);
	temp = NULL;
	free(tag);
	tag = NULL;

	return OK;
}

int isFamEvent(char * string){
	int size = strlen(string);
	size = size+1;

	if (strncmp(string, "ANUL", size) == 0){
		return 1;
	}
	if (strncmp(string, "CENS", size) == 0){
		return 1;
	}
	if (strncmp(string, "DIV", size) == 0){
		return 1;
	}
	if (strncmp(string, "DIVF", size) == 0){
		return 1;
	}
	if (strncmp(string, "ENGA", size) == 0){
		return 1;
	}
	if (strncmp(string, "MARR", size) == 0){
		return 1;
	}
	if (strncmp(string, "MARB", size) == 0){
		return 1;
	}
	if (strncmp(string, "MARC", size) == 0){
		return 1;
	}
	if (strncmp(string, "MARL", size) == 0){
		return 1;
	}
	if (strncmp(string, "MARS", size) == 0){
		return 1;
	}
	if (strncmp(string, "EVEN", size) == 0){
		return 1;
	}
	return 0;
}



int isEvent(char * string){
	int size = strlen(string);
	size = size +1;
	if (strncmp(string,"ADOP",size) == 0){
		return 1;
	} 
	if (strncmp(string,"BIRT",size) == 0){
		return 1;
	}
	if (strncmp(string,"BAPM",size) == 0){
		return 1;
	}
	if (strncmp(string,"BARM",size) == 0){
		return 1;
	}
	if (strncmp(string,"BASM",size) == 0){
		return 1;
	}
	if (strncmp(string,"BLES",size) == 0){
		return 1;
	}
	if (strncmp(string,"BURI",size) == 0){
		return 1;
	}
	if (strncmp(string,"CENS",size) == 0){
		return 1;
	}
	if (strncmp(string,"CHR",size) == 0){
		return 1;
	}
	if (strncmp(string,"CHRA",size) == 0){
		return 1;
	}
	if (strncmp(string,"CONF",size) == 0){
		return 1;
	}
	if (strncmp(string,"CREM",size) == 0){
		return 1;
	}
	if (strncmp(string,"DEAT",size) == 0){
		return 1;
	}
	if (strncmp(string,"EMIG",size) == 0){
		return 1;
	}
	if (strncmp(string,"FCOM",size) == 0){
		return 1;
	}
	if (strncmp(string,"GRAD",size) == 0){
		return 1;
	}
	if (strncmp(string,"IMMI",size) == 0){
		return 1;
	}
	if (strncmp(string,"NATU",size) == 0){
		return 1;
	}
	if (strncmp(string,"ORDN",size) == 0){
		return 1;
	}
	if (strncmp(string,"RETI",size) == 0){
		return 1;
	}
	if (strncmp(string,"PROB",size) == 0){
		return 1;
	}
	if (strncmp(string,"WILL",size) == 0){
		return 1;
	}
	if (strncmp(string,"EVEN",size) == 0){
		return 1;
	}
	return 0;
}

void dummyDelete (void * toBeDeleted){}

int getDateVal(char * string, char * string2){
	char * token;
	char * token2;
	int i = 0;
	char * day = malloc(sizeof(char) * 256);
	char * day2 = malloc(sizeof(char) * 256);
	char * month = malloc(sizeof(char) * 256);
	char * month2 = malloc(sizeof(char) * 256);
	char * year = malloc(sizeof(char) * 256);
	char * year2 = malloc(sizeof(char) * 256);
	strcpy(month2, "");
	strcpy(month, "");
	strcpy(day,"");
	strcpy(day2, "");
	strcpy(year, "");
	strcpy(year2, "");
	
	token = strtok(string, " ");
	if (token != NULL){
		strcpy(day, token);
	}
	token = strtok(NULL, " ");
	if (token != NULL){
		strcpy(month, token);
		for (i=0; i<strlen(month);i++){
			month[i] = tolower(month[i]);
		}		
	}
	token = strtok(NULL, "");
	if (token != NULL){
		strcpy(year, token);
	}

	token2 = strtok(string2, " ");
	if (token2 != NULL){
		strcpy(day2, token2);
	}
	token2 = strtok(NULL, " ");
	if (token2 != NULL){
		strcpy(month2, token2);
		for (i=0; i<strlen(month2);i++){
			month2[i] = tolower(month2[i]);
		}		
	}
	
	token2 = strtok(NULL, "");
	if (token2 != NULL){
		strcpy(year2, token2);
	}
	if (strcmp(year,year2) > 0){
		free(day);
		free(day2);
		free(month);
		free(month2);
		free(year);
		free(year2);
		return 1;
	} else if (strcmp(year, year2) < 0){
		free(day);
		free(day2);
		free(month);
		free(month2);
		free(year);
		free(year2);
		return 2;
	} else {
		int monthNum = getMonth(month);
		int monthNum2 = getMonth(month2);
		if (monthNum > monthNum2){
			free(day);
			free(day2);
			free(month);
			free(month2);
			free(year);
			free(year2);
			return 1;
		} else if (monthNum < monthNum2){
			free(day);
			free(day2);
			free(month);
			free(month2);
			free(year);
			free(year2);
			return 2;
		} else {
			if (strcmp(day, day2) > 0){
				
				free(day);
				free(day2);
				free(month);
				free(month2);
				free(year);
				free(year2);
				return 1;
			} else if (strcmp(day, day2) < 0){
				
				free(day);
				free(day2);
				free(month);
				free(month2);
				free(year);
				free(year2);
				return 2;
			} else {
				
				free(day);
				free(day2);
				free(month);
				free(month2);
				free(year);
				free(year2);
				return 0;
			}
		}
	}	
}

int getMonth(char * string){
	int dateVal2 = 0;
	if (strcmp(string, "jan") == 0){
			dateVal2 = 1;
		} else if (strcmp(string, "feb") == 0){
			dateVal2 = 2;
		} else if (strcmp(string, "mar") == 0){
			dateVal2 = 3;
		} else if (strcmp(string, "apr") == 0){
			dateVal2 = 4;
		} else if (strcmp(string, "may") == 0){
			dateVal2 = 5;
		} else if (strcmp(string, "jun") == 0){
			dateVal2 = 6;
		} else if (strcmp(string, "jul") == 0){
			dateVal2 = 7;
		} else if (strcmp(string, "aug") == 0){
			dateVal2 = 8;
		} else if (strcmp(string, "sep") == 0){
			dateVal2 = 9;
		} else if (strcmp(string, "oct") == 0){
			dateVal2 = 10;
		} else if (strcmp(string, "nov") == 0){
			dateVal2 = 11;
		} else if (strcmp(string, "dec") == 0){
			dateVal2 = 12;
		}
		return dateVal2;
	}
	
bool compareDummy(const void* first, const void* second){
	Individual * theFirst = (Individual*)first;
	Individual * theSecond = (Individual*)second;
	
	if (compareIndividuals(theFirst, theSecond) == 0){
		return true;
	} 
	
	return false;
}

int compareEntireFams(const void * first, const void * second){
	Family * one = (Family*)first;
	Family * two = (Family*)second;
	
	if (one == NULL || two == NULL){
		return 1;
	}
	
	//compare families
	int equal = compareFamilies(one,two);
	if (equal != 0){
		return 1;
	}
	 
	//lengths of events
	int x = getLength(one->events);
	int y = getLength(two->events);
	if (x != y){
		return 1;
	}
	
	//lengths of children
	x = getLength(one->children);
	y = getLength(two->children);
	if (x != y){
		return 1;
	}
	
	//lengths of other fields
	x = getLength(one->otherFields);
	y = getLength(two->otherFields);
	if (x != y){
		return 1;
	}
	
	//compare husbands
	if (one->husband != NULL && two->husband != NULL){
		equal = compareEntireIndi(one->husband, two->husband);
		if (equal != 0){
			return 1;
		}
	}
	
	//compare wifes
	if (one->wife != NULL && two->wife != NULL){
		equal = compareEntireIndi(one->wife, two->wife);
		if (equal != 0){
			return 1;
		}
	}
	
	//compare events	
	Node * curr3 = one->events.head;
	Node * curr4 = two->events.head;
	int m = 0;
	while (curr3 != NULL && curr4 != NULL){
		m = compareEvents(curr3->data, curr4->data);
		if (m != 0){
			return 1;
		}
		curr4 = curr4->next;
		curr3 = curr3->next;
	}
	
	
	//compare other fields
	Node * curr1 = one->otherFields.head;
	Node * curr2 = two->otherFields.head;
	int f = 0;
	while (curr1 != NULL && curr2 != NULL){
		f = compareFields(curr1->data, curr2->data);
		if (f != 0){
			return 1;
		}
		curr1 = curr1->next;
		curr2 = curr2->next;
	}

	return 0;
	
}




int compareEntireIndi(const void * first, const void * second){
	Individual * one = (Individual*)first;
	Individual * two = (Individual*)second;
	if (compareIndividuals(first, second) != 0){
		return 1;
	}

	int m = 0;
	
	int x = getLength(one->families);
	int y = getLength(two->families);
	if (x != y){
		return 1;
	}
	x = getLength(one->events);
	y = getLength(two->events);
	if (x != y){
		return 1;
	}
	x = getLength(one->otherFields);
	y = getLength(two->otherFields);
	if (x != y){
		return 1;
	}
	
	Node * curr = one->families.head;
	Node * curr2 = two->families.head;
	while (curr != NULL && curr2 != NULL){
		m = compareFamilies(curr->data, curr2->data);
		if (m != 0){
			return 1;
		}
		curr2 = curr2->next;
		curr = curr->next;
	}
	
	Node * curr3 = one->events.head;
	Node * curr4 = two->events.head;
	while (curr3 != NULL && curr4 != NULL){
		m = compareEvents(curr3->data, curr4->data);
		if (m != 0){
			return 1;
		}
		curr4 = curr4->next;
		curr3 = curr3->next;
	}	
	
	Node * curr5 = one->otherFields.head;
	Node * curr6 = two->otherFields.head;
	while (curr5 != NULL && curr6 != NULL){
		m = compareFields(curr5->data, curr6->data);
		if (m != 0){
			return 1;
		}
		curr5 = curr5->next;
		curr6 = curr6->next;
	}
	
	return 0;
}

void getChildrenDes(List * desList, const Individual * person){
	Node * curr = NULL;
	Node * curr2 = NULL;
	curr = person->families.head;
	Family * famData = NULL;
	int wife = -1;
	int husb = -1;
	
	while (curr != NULL){
		famData = curr->data;
		wife = compareEntireIndi(person, famData->wife);
		husb = compareEntireIndi(person, famData->husband);
		if (wife == 0 || husb == 0){
			curr2 = famData->children.head;
			while (curr2 != NULL){
				Individual * toGet = curr2->data;
				Individual * toAdd = createCopy(toGet);
				insertBack(desList, toAdd);
				getChildrenDes(desList, toGet);
				curr2 = curr2->next;
			}
		}
		curr = curr->next;
	}
}

Individual * createCopy(Individual * toCopy){
	Individual * toReturn = malloc(sizeof(Individual));
	toReturn->givenName = malloc(sizeof(char) * 256);
	strcpy(toReturn->givenName, "");
	toReturn->surname = malloc(sizeof(char) * 256);
	strcpy(toReturn->surname, "");
	
	toReturn->events = initializeList(&printEvent, &dummyDelete, &compareEvents);
	toReturn->families = initializeList(&printFamily, &dummyDelete, &compareFamilies);
	toReturn->otherFields = initializeList(&printField, &dummyDelete, &compareFields);
	
	if (strcmp(toCopy->givenName, "") != 0){
		strcpy(toReturn->givenName, toCopy->givenName);
	}
	if (strcmp(toCopy->surname, "") != 0){
		strcpy(toReturn->surname, toCopy->surname);
	}
	
	
	Node * curr = NULL;
	
	curr = toCopy->events.head;
	while (curr != NULL){
		Event * theEvent = curr->data;
		insertBack(&toReturn->events, theEvent);
		curr = curr->next;
	}
	
	curr = NULL;
	curr = toCopy->families.head;
	while (curr != NULL){
		Family * theFam = curr->data;
		insertBack(&toReturn->families, theFam);
		curr = curr->next;
	}
	
	curr = NULL;
	curr = toCopy->otherFields.head;
	while (curr != NULL){
		Field * theField = curr->data;
		insertBack(&toReturn->otherFields, theField);
		curr = curr->next;
	}
	
	return toReturn;
}

ErrorCode newFgets(char *buffer, FILE * fptr){
	char ch;
	int i = 0;
	ch = fgetc(fptr);
	if (ch != '\n' && ch != '\r' && ch != '\0' && !feof(fptr)){
		i = 0;
		while (ch != '\n' && ch != '\r' && ch != '\0' && !feof(fptr)){
			if (i > 254){
				return INV_HEADER;
			}
			buffer[i] = ch;
			ch = fgetc(fptr);
			i++;
		}
	}

		buffer[i] = '\0';	
	
	if (strcmp(buffer, "0 TRLR") == 0){
		return OK;
	} else {
		ch = fgetc(fptr);
		while(ch == '\n' || ch == '\r' || ch == '\0'){
			ch = fgetc(fptr);
		}
		
	} 
	fseek(fptr,-1,SEEK_CUR);
	return OK;
}

bool isLevelOneIndi(char * tag){
	if (strcmp(tag, "RESN") == 0){
		return true;
	} else if (strcmp(tag, "SEX") == 0){
		return true;
	} else if (strcmp(tag, "SUBM") == 0){
		return true;
	} else if (strcmp(tag, "ALIA") == 0){
		return true;
	} else if (strcmp(tag, "ANCI") == 0){
		return true;
	} else if (strcmp(tag, "DESI") == 0){
		return true;
	} else if (strcmp(tag, "RFN") == 0){
		return true;
	} else if (strcmp(tag, "AFN") == 0){
		return true;
	} else if (strcmp(tag, "REFN") == 0){
		return true;
	} else if (strcmp(tag, "RIN") == 0){
		return true;
	} else if (strcmp(tag, "TITL") == 0){
		return true;
	}
	return false;
}


void getChildrenDesN(List * desList, const Individual * person, int * num, int max){	
	(*num) = (*num) +1;
	int counter = *num;
	List * listToAdd = NULL;
	if (counter >= max){
		return;
	}
	listToAdd = getList(desList, counter, max);
	Node * curr = NULL;
	Node * curr2 = NULL;
	curr = person->families.head;
	Family * famData = NULL;
	int wife = -1;
	int husb = -1;
	
	while (curr != NULL){
		famData = curr->data;
		wife = compareEntireIndi(person, famData->wife);
		husb = compareEntireIndi(person, famData->husband);
		if (wife == 0 || husb == 0){
			curr2 = famData->children.head;
			while (curr2 != NULL){
				Individual * toGet = curr2->data;
				int duplicate = isSame(toGet, desList);
				if (listToAdd != NULL && duplicate != 1){
					Individual * toAdd = createCopy(toGet);
						insertSorted(listToAdd, toAdd);
				}
				getChildrenDesN(desList, toGet, num, max);
				(*num) = (*num) -1;
				curr2 = curr2->next;
			}
		}
		curr = curr->next;
	}
	
}

List * getList(List * desList, int num, int max){
	Node * curr = desList->head;
	List * toReturn = NULL;
	if (num > max){
		return NULL;
	}
	int counter = 0;
	while (counter <= num){
		if (counter == num){
			break;
		} else if (counter != num){
			curr = curr->next;
			counter++;
		}
	}
	toReturn = curr->data;
	return toReturn;
}

int isSame(Individual * toCheck, List * theDes){
		Node * curr = theDes->head;
		while (curr != NULL){
			List * test = curr->data;
			Node * curr2 = test->head;
			while (curr2 != NULL){
				Individual * personTest = curr2->data;
				int x = compareEntireIndi(toCheck, personTest);
				if (x == 0){
					return 1;
				} else {
					curr2 = curr2->next;
				}
			}
			curr = curr->next;
		}
	return 0;
}

char* dummyPrint(void* toBePrinted){
	char * hello = malloc(sizeof(char) * 20);
	return hello;
}

int dummyCompare(const void* first,const void* second){
	return 0;	
}

void putLNAtBack(List * desList){
	Node * curr = desList->head;
		while (curr != NULL){
			List * test = curr->data;
			Node * curr2 = test->head;
			while (curr2 != NULL){
				Individual * personTest = curr2->data;
				curr2 = curr2->next;
				if (strcmp(personTest->surname, "") == 0){
					deleteDataFromList(test, personTest);
					insertBack(test,personTest);
				}
			}
			curr = curr->next;
		}
}


void getChildrenAncestors(List * ansList, const Individual * person, int * num, int max){
	
	(*num) = (*num) +1;
	int counter = *num;
	List * listToAdd = NULL;
	if (counter >= max){
		return;
	}
	listToAdd = getList(ansList, counter, max);
	Node * curr = NULL;
	Node * curr2 = NULL;
	Individual * theHusband = NULL;
	Individual * theWife = NULL;
	Family * famData = NULL;
	int duplicate = 0;
	curr = person->families.head;
	
	while (curr != NULL){
		famData = curr->data;
		curr2 = famData->children.head;
		while (curr2 != NULL){
			Individual * currChild = curr2->data;
			if (compareEntireIndi(currChild, person) == 0){
				if (famData->husband != NULL){
					theHusband = famData->husband;
					duplicate = isSame(theHusband, ansList);
					if (listToAdd != NULL && duplicate != 1){
						Individual * toAdd = createCopy(theHusband);
						insertSorted(listToAdd, toAdd);
					}
					getChildrenAncestors(ansList, theHusband, num, max);
					(*num) = (*num) -1;
				} 
				if (famData->wife != NULL){
					theWife = famData->wife;
					duplicate = isSame(theWife, ansList);
					if (listToAdd != NULL && duplicate != 1){
						Individual * toAdd2 = createCopy(theWife);
						insertSorted(listToAdd, toAdd2);
					}
					getChildrenAncestors(ansList, theWife, num, max);
					(*num) = (*num) -1;
				}
			}
			curr2 = curr2->next;
		}
		curr = curr->next;
	}
	
}

void removeQuotes(char * string){
	if (string[strlen(string)-1] == '\"'){
		string[strlen(string)-1] = '\0';
		if (string[0] == '\"'){
			int i = 0;
			for (i = 0; i<strlen(string); i++){
				string[i] = string[i+1];
			}
		}
	}
}

int checkForNewline(char * string){
	int i = 0;
	for (i=0;i<strlen(string); i++){
		if (string[i] == '\n'){
			return 1;
		}
	}
	return 0;
}

char * tableInfo(char * fileName){
	GEDCOMobject * myObject = NULL;
	GEDCOMerror objError = createGEDCOM(fileName, &myObject);
	char * string = malloc(sizeof(char) * 1000);
	strcpy(string, "[]");
	if (objError.type == OK){
		ErrorCode objError2 = validateGEDCOM(myObject);
		if (objError2 == OK){
			int indis = getLength(myObject->individuals);
			int fams = getLength(myObject->families);
			char * numInd = malloc(sizeof(char) * 100);
			char * numFam = malloc(sizeof(char) * 100);
			sprintf(numInd, "%d", indis);
			sprintf(numFam, "%d", fams);
			Header * theHead = myObject->header;
			Submitter * theSub = myObject->submitter;
			char * version = malloc(sizeof(char) * 100);
			sprintf(version,"%.2f",theHead->gedcVersion);
			char * encoding = getEncodingString(theHead->encoding);
			strcpy(string, "{\"source\":\"");
			strcat(string, theHead->source);
			strcat(string, "\", \"version\":\"");
			strcat(string, version);
			strcat(string, "\", \"encoding\":\"");
			strcat(string, encoding);
			strcat(string, "\", \"address\":\"");
			strcat(string, theSub->address);
			strcat(string, "\", \"name\":\"");
			strcat(string, theSub->submitterName);
			strcat(string, "\", \"indis\":\"");
			strcat(string, numInd);
			strcat(string, "\", \"fams\":\"");
			strcat(string, numFam);
			strcat(string, "\"}");
		}
	} 
	deleteGEDCOM(myObject);
	return string;
}

char * indInfo(char * fileName){
	GEDCOMobject * myObject = NULL;
	GEDCOMerror objError = createGEDCOM(fileName, &myObject);
	if (objError.type == OK){
		ErrorCode objError2 = validateGEDCOM(myObject);
		if (objError2 == OK){
			char * string = iListToJSON(myObject->individuals);
			return string;
		}
	} 
	deleteGEDCOM(myObject);
	
	return "[]";
}

char * createSimple(char * fileName, char * subName, char * subAddress){
	char * string = malloc(sizeof(char) * 100);
	strcpy(string, "");
	GEDCOMobject * myObject = malloc(sizeof(GEDCOMobject));
	
	Header * theHead = malloc(sizeof(Header));
	strcpy(theHead->source,"genealogy app");
	theHead->gedcVersion = 1;
	theHead->encoding = ASCII;
	theHead->otherFields = initializeList(&printField, &deleteField, &compareFields); 
	
	Submitter * theSub = malloc(sizeof(Submitter) + 256 * sizeof(char));
	
	myObject->header = theHead;
	strcpy(theSub->submitterName, subName);
	theSub->otherFields = initializeList(&printField, &deleteField, &compareFields); 
	strcpy(theSub->address, subAddress);
	
	myObject->submitter = theSub;
	theHead->submitter = theSub;
	myObject->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
	myObject->individuals = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
	
	ErrorCode objErr = validateGEDCOM(myObject);
	
	if (objErr == OK){
		GEDCOMerror writeError = writeGEDCOM(fileName, myObject);
		if (writeError.type != OK){
			strcpy(string, "WERROR");
		} else {
			strcpy(string, "OK");
		}
	} else {
		strcpy(string, "OBJERROR");
	}
	deleteGEDCOM(myObject);
	return string;
}

char * addTheIndi(char * fileName, char * JSONstring){
	Individual * toAdd = JSONtoInd(JSONstring);
	GEDCOMobject * myObject = NULL;
	GEDCOMerror objError = createGEDCOM(fileName, &myObject);
	if (objError.type != OK){
		deleteGEDCOM(myObject);
		return "OBJERROR";
	}
	addIndividual(myObject, toAdd);
	GEDCOMerror writeError = writeGEDCOM(fileName, myObject);
	if (writeError.type != OK){
		deleteGEDCOM(myObject);
		return "WERROR";
	}
	deleteGEDCOM(myObject);
	return "OK";
}

char * validateFile(char * fileName){
	GEDCOMobject * myObject = NULL;
	GEDCOMerror myErr = createGEDCOM(fileName, &myObject);
	if (myErr.type != OK){
		return "ERR1";
	}
	ErrorCode myErr2 = validateGEDCOM(myObject);
	if (myErr2 != OK){
		return "ERR2";
	}
	return "OK";
}

char * desToJSON(char * fileName, char * indJSON, int max){
	Individual * toFind = JSONtoInd(indJSON);
	GEDCOMobject * myObject = NULL;
	GEDCOMerror objErr = createGEDCOM(fileName, &myObject);
	if (objErr.type != OK || toFind == NULL){
		deleteGEDCOM(myObject);
		return "[]";
	}
	Node * curr = myObject->individuals.head;
	
	while (curr != NULL){
		Individual * toCheck = curr->data;
		if (compareIndividuals(toCheck, toFind) == 0){
			List theDes = getDescendantListN(myObject, toCheck, max);
			char * toReturn = gListToJSON(theDes);
			deleteGEDCOM(myObject);
			return toReturn;
		}
		curr = curr->next;
	}
	deleteGEDCOM(myObject);
	return "[]";
}

char * ansToJSON(char * fileName, char * indJSON, int max){
	Individual * toFind = JSONtoInd(indJSON);
	GEDCOMobject * myObject = NULL;
	GEDCOMerror objErr = createGEDCOM(fileName, &myObject);
	if (objErr.type != OK || toFind == NULL){
		deleteGEDCOM(myObject);
		return "[]";
	}
	Node * curr = myObject->individuals.head;
	
	while (curr != NULL){
		Individual * toCheck = curr->data;
		if (compareIndividuals(toCheck, toFind) == 0){
			List theAns = getAncestorListN(myObject, toCheck, max);
			char * toReturn = gListToJSON(theAns);
			deleteGEDCOM(myObject);
			return toReturn;
		}
		curr = curr->next;
	}
	deleteGEDCOM(myObject);
	return "[]";
}
