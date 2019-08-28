#include <stdio.h>
#include <stdlib.h>
#include "Party.h"
#include "list.h"
#include "set.h"
#include <assert.h>
#include <string.h>
//#include "libmtm.a"

#define MAX_PARTY_NAME_LENGTH 50
#define MAX_CANDIDATE_NAME_LENGTH 50
#define MAX_CODE_LENGTH 50
#define ID_LENGTH 9
#define END_OF_FILE 0
#define N_END_OF_FILE 1
#define MINIMUM_CANDIDATES 4

//*************************************************************************
//points to ponder
//*************************************************************************
//what is supposed to happen when allocations fail -
// should i use one of the party statuses? is NULL OK?

//*************************************************************************
//structs
//*************************************************************************
typedef struct candidate{
    char* name;
    char* id;
    char* candidate_gender;
}Candidate, *Candidatep;

struct party {
    char* name;
    PartyCode party_code;
    List candidate_list;
};

//*************************************************************************
//static functions
//*************************************************************************
static ListElement CopyCandidate(ListElement candidate){
    Candidatep copy = malloc(sizeof(Candidate));
    if (copy == NULL) {return NULL;}
    copy -> name = malloc(sizeof(char)*(MAX_CANDIDATE_NAME_LENGTH+1));
    if (copy->name == NULL) {free(copy); return NULL;}
    copy -> id = malloc(sizeof(char)*(ID_LENGTH+1));
    if (copy->id == NULL) {free(copy); free(copy->name); return NULL;}
    copy -> candidate_gender = malloc(sizeof(char)*2);
    if (copy->candidate_gender == NULL) {free(copy); free(copy->name); free(copy->id); return NULL;}

    strcpy((copy->name),((Candidatep)(candidate))-> name);
    strcpy((copy->id),((Candidatep)(candidate))-> id);
    strcpy((copy-> candidate_gender),((Candidatep)(candidate))-> candidate_gender);
    return copy;
}

static void FreeCandidate(ListElement candidate){
    if (candidate == NULL) {return;}
    free(((Candidatep)candidate) -> name);
    free(((Candidatep)candidate) -> id);
    free(((Candidatep)candidate) -> candidate_gender);
    free(candidate);
}

static bool find_beginning(FILE* fin, char* first_char_ptr) {
    char *buffer = malloc(sizeof(char) * 2);
    for (int i = 0; i < 1; i++) {
        if(fgets(&buffer[i],2,fin) == NULL) {
            free(buffer);
            return END_OF_FILE;
        }
        if ((buffer[i] == ' ') || (buffer[i] == '\n'))
            i--;
        else
            *first_char_ptr = buffer[i];
    }
    free(buffer);
    return N_END_OF_FILE;
}

static char* fill_fields1(FILE* fin, int max_length, char special_character, char* first_char_ptr){
    while(find_beginning(fin, first_char_ptr)) {
        char *buffer = malloc(sizeof(char) * (max_length + 1));
        if (buffer == NULL)
            printf("error in allocation\n");
        //char temp;
        buffer[0] = *(first_char_ptr);
        for (int i = 1; i < max_length; i++) {
            fgets(&buffer[i], 2, fin);
            //printf("string[%d] = %c\n", i, string[i]);
            if (buffer[i] == special_character) {
                buffer[i] = '\0';
                break;
            }
        }
        return buffer;
    }
    return NULL;
}

static bool candidate_maker(FILE* fin, List Candidate_List, char* first_char_ptr){
    Candidatep my_candidate = malloc(sizeof(Candidatep));
    if(!(my_candidate -> name = fill_fields1(fin, MAX_CANDIDATE_NAME_LENGTH, ' ', first_char_ptr)))
        return END_OF_FILE;
    my_candidate -> id  = fill_fields1(fin, ID_LENGTH, ' ', first_char_ptr);
    my_candidate -> candidate_gender = fill_fields1(fin, 1, '\n', first_char_ptr);
    printf("candidate name = %s, candidate id = %s, candidate gender = %s\n", my_candidate -> name, my_candidate -> id, my_candidate -> candidate_gender);
    int candidate_list_result = listInsertFirst(Candidate_List, my_candidate);
    if(candidate_list_result);
    return N_END_OF_FILE;
}


//*************************************************************************
//interface functions
//*************************************************************************

//seems to work with no errors, memory leaks exist when tested without destroyParty
Party createParty(char *party_data_file){
    FILE* fin = fopen(party_data_file, "r");
    if (fin == NULL) {printf("Error in opening party_data_file");}
    Party party1 = malloc(sizeof(struct party));
    //maybe initialize as NULL?
    char first_char = '0', *first_char_ptr = &first_char;
    party1->name = fill_fields1(fin, MAX_PARTY_NAME_LENGTH, '\n', first_char_ptr);
    party1->party_code = fill_fields1(fin, MAX_CODE_LENGTH, '\n', first_char_ptr);


    List Candidate_List = listCreate(CopyCandidate, FreeCandidate);
    while(candidate_maker(fin, Candidate_List, first_char_ptr))
        continue;
    party1 -> candidate_list = Candidate_List;

    fclose(fin);
    return party1;
}

//working
void destroyParty(Party party){
    free(party -> name);
    free(party -> party_code);
    listDestroy(party -> candidate_list);
    free(party);
}

//this function runs with no errors, but I didn't check that it does what we want
PartyResult addPerson(Party party, char *name, char *id, Gender gender, int position){
    if (position < 1)
        return PARTY_FAIL;

    //creating new candidate and filling its fields based on given parameters
    Candidatep new_candidate = malloc(sizeof(struct candidate));
    if(new_candidate==NULL){return PARTY_FAIL;}
    assert(name);
    new_candidate -> name = name;
    if (strlen(new_candidate -> name)==0)
        return PARTY_FAIL;
    assert(id);
    new_candidate -> id = id;
    if(strlen(new_candidate -> id)!= ID_LENGTH)
        return PARTY_FAIL;

    //checking whether a different candidate has the same id:
    ListElement current_candidate = listGetFirst(party -> candidate_list);
    while (current_candidate) {
        if (!(strcmp(((Candidatep) current_candidate)->id, new_candidate->id)))
            return PARTY_FAIL;
        else
            current_candidate = listGetNext(party -> candidate_list);
    }

    ((gender == MASCULINE) ? (new_candidate -> candidate_gender = "M") : (new_candidate -> candidate_gender = "F"));

    //inserting the new candidate into the list in given position
    int i = position, list_size = listGetSize(party -> candidate_list);
    if (i > list_size) {
        listInsertLast(party->candidate_list, new_candidate);
        return PARTY_SUCCESS;
    }
    ListElement in_position = listGetFirst(party -> candidate_list);
    ListElement* in_position_p = &in_position;
    while (i > 0){
        *in_position_p = listGetNext(party -> candidate_list);
        i--;
    }
    listInsertBeforeCurrent(party -> candidate_list, new_candidate);
    return PARTY_SUCCESS;
}

//*************************************************************************
//main
//*************************************************************************

int main(){
    char* f_p = "C:\\Users\\User\\Desktop\\attempt.txt";
    //char* f_p = "/home/elisheva.r/attempt.txt";
    Party ninteys_party = createParty(f_p);
    printf("%s\n", ninteys_party -> name);
    printf("%s\n", ninteys_party -> party_code);
    destroyParty(ninteys_party);
    return 0;
}
