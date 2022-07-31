#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SpreaderDetectorParams.h"


//==================   variable declaration   =====================//


#define MAX_LINE_LENGTH 1025

#define INITIAL_SIZE 1000

#define BASE 10

#define ARGC_VALID_NUMBER 3

#define ARGV_FIRST 1

#define ARGV_SECOND 2

#define FOUR_ARGS 4

#define SUPER_SPREADER_PROB 1

#define NAME_PLACE 0

#define ID_PLACE 1

#define AGE_PLACE 2

#define ERROR_PEOPLE_FILE "Error: cannot find people files path.\n"

#define ERROR_MEETING_FILE "Error: cannot find meeting files path.\n"

#define ERROR_OUTPUT_FILE "Error: cannot open output file.\n"

#define ERROR_INPUT_FORMAT "Usage: ./SpreaderDetectorBackend <Path to People.in> <Path to Meetings.in>\n"

//==================         structs         =====================//


/**
 * this is struct that represent person
 */
typedef struct Person
{
	long id;
	float age;
	float probability;
	char name[MAX_LINE_LENGTH];
} Person;


//==================    global variable    =====================//


int gNumOfPeople = 0;


//================== function declaration  =====================//


int initializePeople(FILE *fpP, Person **persons);

Person getPersonFromLine(char *line);

Person *getPersonFromPersons(long id, Person **persons);

int calculateProb(FILE *fpM, Person **persons);

int updateProb(long infectedId, Person **persons, float prob);

void sortByProb(Person *persons, int length);

int compareByProb(const void *p, const void *q);

int fileIsEmpty(FILE *file);

float crna(float dist, float time);

long convertStrToLong(char *str);

float convertStrToFloat(char *str);

int printOutPut(FILE *outFile, Person **persons);

void freePersons(Person **persons);

void closeFiles(FILE *file1, FILE *file2, FILE *file3);


//==================        function       =====================//

/**
 * the main function that running the all program
 * @param argc
 * @param argv
 * @return 0 if success, 1 if failed.
 */
int main(int argc, char *argv[])
{

	if (argc != ARGC_VALID_NUMBER)
	{
		fprintf(stderr, ERROR_INPUT_FORMAT);
		exit(EXIT_FAILURE);
	}

	FILE *fpP;
	char *fNameP = argv[ARGV_FIRST];

	if (!(fpP = fopen(fNameP, "r")))
	{
		fprintf(stderr, ERROR_PEOPLE_FILE);
		exit(EXIT_FAILURE);
	}

	FILE *fpM;
	char *fNameM = argv[ARGV_SECOND];

	if (!(fpM = fopen(fNameM, "r")))
	{
		fprintf(stderr, ERROR_MEETING_FILE);
		closeFiles(fpP, NULL, NULL);
		exit(EXIT_FAILURE);
	}

	FILE *fpO;

	if (!(fpO = fopen(OUTPUT_FILE, "w")))
	{
		fprintf(stderr, ERROR_OUTPUT_FILE);
		closeFiles(fpP, fpM, NULL);
		exit(EXIT_FAILURE);
	}

	/* valid, saving time */
	if (fileIsEmpty(fpP) == 0)
	{
		closeFiles(fpP, fpM, fpO);
		exit(EXIT_SUCCESS);
	}

	/* define array of persons. */
	Person **persons = (Person **) malloc(sizeof(Person *));

	if (persons == NULL)
	{
		fprintf(stderr, STANDARD_LIB_ERR_MSG);
		closeFiles(fpP, fpM, fpO);
		exit(EXIT_FAILURE);
	}

	if (initializePeople(fpP, persons) != 0)
	{
		fprintf(stderr, STANDARD_LIB_ERR_MSG);
		closeFiles(fpP, fpM, fpO);
		freePersons(persons);
		exit(EXIT_FAILURE);
	}

	/* if the meeting file is empty there is nothing to calculate. */
	if (fileIsEmpty(fpM) != 0)
	{

		if (calculateProb(fpM, persons) != 0)
		{
			fprintf(stderr, STANDARD_LIB_ERR_MSG);
			closeFiles(fpP, fpM, fpO);
			freePersons(persons);
			exit(EXIT_FAILURE);
		}

		sortByProb(*persons, gNumOfPeople);
	}

	if (printOutPut(fpO, persons) != 0)
	{
		fprintf(stderr, STANDARD_LIB_ERR_MSG);
		closeFiles(fpP, fpM, fpO);
		freePersons(persons);
		exit(EXIT_FAILURE);
	}

	closeFiles(fpP, fpM, fpO);
	freePersons(persons);
	exit(EXIT_SUCCESS);
}


/**
 * this function extract the people from the file and initialize the in array that we allocate
 * memory for.
 * @param fpM - the file with the data
 * @param persons - the pointer for the array pointer that we allocate memory for
 * @return 0 if success, 1 otherwise
 */
int initializePeople(FILE *fpP, Person **persons)
{
	int personsCapacity = INITIAL_SIZE;

	/* allocate memory for the array */
	*persons = (Person *) malloc(INITIAL_SIZE * sizeof(Person));
	if (*persons == NULL)
	{
		return 1;
	}

	char line[MAX_LINE_LENGTH] = {'\0'};


	/* extract the person from the file */
	while (fgets(line, MAX_LINE_LENGTH, fpP))
	{
		gNumOfPeople++;
		Person person;

		person = getPersonFromLine(line);


		/* if there is not enough space we allocate more memory space */
		if (gNumOfPeople == personsCapacity)
		{
			personsCapacity *= 2;
			*persons = (Person *) realloc(*persons, personsCapacity * sizeof(Person));
			if (*persons == NULL)
			{
				return 1;
			}
		}

		(*persons)[gNumOfPeople - 1] = person;
	}

	return 0;
}


/**
 * this function get's line from the people file extract the data and create person with the data
 * @param line - the line from the people file
 * @return Person type with all the data from the line
 */
Person getPersonFromLine(char *line)
{
	/* default initialize */
	Person person = {0, 0, 0, ""};

	char *token;
	int lineCounter = 0;

	token = strtok(line, " ");

	while (token != NULL)
	{
		switch (lineCounter)
		{
			case NAME_PLACE:
				strcpy(person.name, token);
				break;

			case ID_PLACE:
				person.id = convertStrToLong(token);
				break;

			case AGE_PLACE:
				person.age = convertStrToFloat(token);
				break;

			default:     /* we assuming that the input is valid. */
				break;
		}

		lineCounter++;
		token = strtok(NULL, " ");
	}

	return person;
}


/**
 * this function get's id and return the person from the persons array
 * @param id - the id that the function get's
 * @param persons - the persons array
 * @return the person pointer if exist, NULL otherwise
 */
Person *getPersonFromPersons(const long id, Person **persons)
{

	for (int i = 0; i < gNumOfPeople; ++i)
	{
		if ((*persons)[i].id == id)
		{
			return &(*persons)[i];
		}
	}
	return NULL;
}


/**
 * this function calculate fore each person is probability, by the data from the meeting file, and
 * update the probability in the person struct
 * @param fpP - the meeting file
 * @param persons - the persons array
 * @return  * @return 0 if success, 1 otherwise
 */
int calculateProb(FILE *fpM, Person **persons)
{

	long infectorId = 0;
	long infectedId;
	float distant;
	float time;

	/* the super spreader id */
	fscanf(fpM, "%lu", &infectorId);

	/* super spreader get probability 1 */
	if (updateProb(infectorId, persons, SUPER_SPREADER_PROB) != 0)
	{
		return 1;
	}

	/* extract the data from the meeting file */
	while (fscanf(fpM, "%lu %lu %f %f", &infectorId, &infectedId, &distant, &time) == FOUR_ARGS)
	{

		float infectorProb = getPersonFromPersons(infectorId, persons)->probability;

		/* calculate the probability by the formula */
		float infectedProb = infectorProb * crna(distant, time);

		/* update the new probability */
		if (updateProb(infectedId, persons, infectedProb) != 0)
		{
			return 1;
		}
	}

	return 0;
}


/**
 * this function update the probability by the formula that we have.
 * @param infectedId - the infected person id
 * @param persons - the persons array
 * @param prob - the new probability
 * @return 0 if success, 1 other wise
 */
int updateProb(long infectedId, Person **persons, float prob)
{

	Person *person = getPersonFromPersons(infectedId, persons);

	if (person)
	{
		person->probability = prob;
		return 0;
	}
	else
	{
		return 1;
	}
}


/**
 * Sort an array of length persons by there Probability
 * @param persons - the array that the function sort
 * @param length - the length of the array
 */
void sortByProb(Person *persons, int length)
{
	qsort(persons, length, sizeof(*persons), compareByProb);
}


/**
 * this function compare between to persons by there probability
 * @param p - first person.
 * @param q - second person.
 * @return -1 if the first.prob > second.prob, 1 if first.prob < second.prob, 0 if equal.
 */
int compareByProb(const void *p, const void *q)
{
	const Person *a = (const Person *) p;
	const Person *b = (const Person *) q;

	float x = a->probability;
	float y = b->probability;

	if (x < y)
	{
		return 1;
	}
	else if (x > y)
	{
		return -1;
	}

	return 0;
}


/**
 * this function calculate the crna by the formula that we have.
 * @param dist - the distant that we get's from the meeting file
 * @param time - the time that we get's from the meeting file
 * @return the crna by the formula
 */
float crna(float dist, float time)
{
	return ((time * MIN_DISTANCE) / (dist * MAX_TIME));
}


/**
 * this function convert string to long.
 * @param string
 * @return lung
 */
long convertStrToLong(char *str)
{
	char *strPart;
	long num;
	num = strtol(str, &strPart, BASE);

	return num;
}


/**
 * this function convert string to Float.
 * @param string
 * @return Float
 */
float convertStrToFloat(char *str)
{
	char *strPart;
	float num;
	num = strtof(str, &strPart);

	return num;
}


/**
 * this function print the result to the new file
 * @param outFile - the new file that the function printing to
 * @param persons - the array of all the persons
 * @return 0 if success 1 otherwise
 */
int printOutPut(FILE *outFile, Person **persons)
{
	for (int i = 0; i < gNumOfPeople; ++i)
	{
		if ((*persons)[i].probability >= MEDICAL_SUPERVISION_THRESHOLD)
		{
			if (fprintf(outFile, MEDICAL_SUPERVISION_THRESHOLD_MSG,
						(*persons)[i].name, (*persons)[i].id) < 0)
			{
				return 1;
			}
		}
		else if ((*persons)[i].probability >= REGULAR_QUARANTINE_THRESHOLD &&
				 (*persons)[i].probability < MEDICAL_SUPERVISION_THRESHOLD)
		{
			if (fprintf(outFile, REGULAR_QUARANTINE_MSG, (*persons)[i].name, (*persons)[i].id) < 0)
			{
				return 1;
			}
		}
		else
		{
			if (fprintf(outFile, CLEAN_MSG, (*persons)[i].name, (*persons)[i].id) < 0)
			{
				return 1;
			}
		}
	}

	return 0;
}


/**
 * this function free all the memory that we allocate in the program
 * @param persons - the array of persons
 */
void freePersons(Person **persons)
{
	if (*persons != NULL)
	{
		free(*persons);
		*persons = NULL;
	}

	free(persons);
	persons = NULL;
}

/**
 * this function check if file is empty or not
 * @param file - the file pointer
 * @return 0 if the file empty, 1 other wise
 */
int fileIsEmpty(FILE *file)
{
	long savedOffset = ftell(file);
	fseek(file, 0, SEEK_END);

	if (ftell(file) == 0)
	{
		return 0;
	}

	fseek(file, savedOffset, SEEK_SET);

	return 1;
}


/**
 * this function close 3 files.
 * @param file1 - first pointer.
 * @param file2 - second pointer.
 * @param file3 - third pointer.
 */
void closeFiles(FILE *file1, FILE *file2, FILE *file3)
{
	if (file1 != NULL)
	{
		fclose(file1);
	}
	if (file2 != NULL)
	{
		fclose(file2);
	}
	if (file3 != NULL)
	{
		fclose(file3);
	}
}

