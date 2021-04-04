#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LINE_LEN 132
#define MAX_EVENTS 500

/* Name: Puneet Grewal
   V Number : V00951156
   Class: SENG 265 A01
   Prof: Mike Zastre
   Assignment: A1 - icsout.c first version
*/

// Variable for total number of events in a ics file

int num_events;


// Initializing a new data type to store all the individual elements of each event from the files

typedef struct calendar {

    char DTSTART[MAX_LINE_LEN];
    char DTEND[MAX_LINE_LEN];
    char RRULE[MAX_LINE_LEN];
    char SUMMARY[MAX_LINE_LEN];
    char LOCATION[MAX_LINE_LEN];
    
} calendar;


// A new array to store the elements of each individual event. It takes the data type of the above struct.

calendar event[MAX_EVENTS];


// Function "dt_format" is copied directly from timeplay.c file that was included in the a1 subdirectory.
// No changes are made to this file and it is called in function "print".

void dt_format(char *formatted_time, const char *dt_time, const int len) {

      struct tm temp_time;
      time_t    full_time;
      char      temp[5];

      memset(&temp_time, 0, sizeof(struct tm));
      sscanf(dt_time, "%4d%2d%2d",
          &temp_time.tm_year, &temp_time.tm_mon, &temp_time.tm_mday);
      temp_time.tm_year -= 1900;
      temp_time.tm_mon -= 1;
      full_time = mktime(&temp_time);
      strftime(formatted_time, len, "%B %d, %Y (%a)",
          localtime(&full_time));

}

// Function "dt_print" is called from "print" and is used to print the date in the correct format
// with the help of "dt_format".
// This function also prints the required dashes after the date and a new line.

void dt_print (char* formatted_time) {

    printf("%s\n", formatted_time);
    for (int j = 0; j < strlen(formatted_time); j++) {
        printf("-");
    }
    printf("\n");
}

// Function "dt_increment" is copied from timeplay.c similar to "dt_format" however one change is made.
// strftime (after, 8...) is changed to strftime (after, 9...).
// This function is called in "read" to obtain corresponding dates for events that are repeating.

void dt_increment(char *after, const char *before, int const num_days) {
      struct tm temp_time, *p_temp_time;
      time_t    full_time;
      char      temp[5];

      memset(&temp_time, 0, sizeof(struct tm));
      sscanf(before, "%4d%2d%2d", &temp_time.tm_year,
         &temp_time.tm_mon, &temp_time.tm_mday);
      temp_time.tm_year -= 1900;
      temp_time.tm_mon -= 1;
      temp_time.tm_mday += num_days;

      full_time = mktime(&temp_time);
      after[0] = '\0';
      strftime(after, 9, "%Y%m%d", localtime(&full_time));
      strncpy(after + 8, before + 8, MAX_LINE_LEN - 8);
      after[MAX_LINE_LEN - 1] = '\0';

}

// Function "compare_dates" is used in qsort in order to organize the events by their dates so they are printed
// accordingly. Format of the function is adapted from slide 124 of "03-c-programming-DRAFT.pdf".

int compare_dates (const void *a, const void *b) {

     calendar *ia = (calendar *)a;
     calendar *ib = (calendar *)b;

     return strcmp((*ia).DTSTART, (*ib).DTSTART);

}

// One of the 2 major functions in this project. "Read" takes in 7 parameters which include
// 6 integers for the 2 date boundary and the file that is to be read. Using a combination of
// buffer, fgets, sscanf etc., the function iterates through the file whilst adding events to the struct.
 
void read (char *filename, int from_y, int from_m, int from_d, int to_y, int to_m, int to_d) {

// The integers dates need to be converted into a format that matches the dates on the .ics files

    int from_date = from_d + (from_m * 100) + (from_y * 10000);
    int to_date = to_d + (to_m * 100) + (to_y * 10000);

// 2 variables that are used when calculating repeated events

    char new_DTSTART[MAX_LINE_LEN];
    char new_DTEND[MAX_LINE_LEN];

    char buffer[MAX_LINE_LEN];
    FILE *ics;
    ics = fopen(filename, "r");

// "end" and "start" hold the start date and end date of a single event

    int end = 0;
    int start = 0;

// A variable that is used to store all the details about a single event and takes on the data type of the struct

    calendar temp;
    
// The major while loop of this function iterates the whole file until all lines are read.

    while (fgets(buffer, MAX_LINE_LEN, ics)) {
         
        if (strncmp (buffer, "BEGIN:VEVENT", 12) == 0) {
            fgets(buffer, MAX_LINE_LEN, ics);
            sscanf(buffer, "DTSTART:%s", temp.DTSTART);
            fgets(buffer, MAX_LINE_LEN, ics);
            sscanf(buffer, "DTEND:%s", temp.DTEND);

            fgets(buffer, MAX_LINE_LEN, ics);

// Here, the function is broken down into 2 segments, one for repeated events that have a "RRULE" parameter and another for
// those that don't.

            if (strncmp (buffer, "RRULE", 5) != 0) {
                strncpy (temp.RRULE, temp.DTSTART, MAX_LINE_LEN);

// This specific if-statement is to bypass a "EXDATE" parameter that is present on line 62 of Diana Devops
// and not present anywhere else.
 
                if (strncmp (buffer, "EXDATE", 6) ==0 ) {
                     fgets (buffer, MAX_LINE_LEN, ics);
                }

// These if-statements that check if the required field is in the buffer is used in case testing
// involves other parameters such as "EXDATE" above which could cause errors.

                if (strncmp (buffer, "LOCATION", 8) != 0) {
                     fgets (buffer, MAX_LINE_LEN, ics);
                }
                strncpy (temp.LOCATION, &buffer[9], strlen(buffer));

// These specific lines are used for formatting and spacing issues as they reduce the length of the
// string used to hold the variable.

                temp.LOCATION[strlen(temp.LOCATION) - 1] = '\0';
                fgets (buffer, MAX_LINE_LEN, ics);
                if (strncmp (buffer, "SUMMARY", 7) != 0) {
                    fgets (buffer, MAX_LINE_LEN, ics);
                }
                strncpy (temp.SUMMARY, &buffer[8], strlen(buffer));
                temp.SUMMARY[strlen(temp.SUMMARY) - 1] = '\0';
            }

// This part of the function is for events that are repeating and have a "RRULE" field.
            
            else {

// The RRULE formatting of test files many.ics and diana-devops.ics are different hence this 
// if-statement is used to solve that issue.

                if (strncmp (buffer, "RRULE:FREQ=WEEKLY;UNTIL=", 24) == 0) {
                    sscanf (buffer, "RRULE:FREQ=WEEKLY;UNTIL=%15s", temp.RRULE);
                } else {
                        sscanf (buffer, "RRULE:FREQ=WEEKLY;WKST=MO;UNTIL=%15s", temp.RRULE);
                }

                fgets (buffer, MAX_LINE_LEN, ics);

                if (strncmp (buffer, "EXDATE", 6) ==0 ) {
                    fgets (buffer, MAX_LINE_LEN, ics);
                }

                if (strncmp (buffer, "LOCATION", 8) != 0 ) {
                    fgets (buffer, MAX_LINE_LEN, ics);
                }

                strncpy (temp.LOCATION, &buffer[9], strlen(buffer));
                temp.LOCATION[strlen(temp.LOCATION) - 1] = '\0';
                fgets (buffer, MAX_LINE_LEN, ics);

                if (strncmp (buffer, "SUMMARY", 7) != 0 ) {
                    fgets (buffer, MAX_LINE_LEN, ics);
                }

                strncpy (temp.SUMMARY, &buffer[8], strlen(buffer));
                temp.SUMMARY[strlen(temp.SUMMARY) - 1] = '\0';
            }
                          
            sscanf(temp.DTSTART, "%8d", &start);
            sscanf(temp.DTEND, "%8d", &end);

            if ((from_date <= start) && (to_date >= end)) {
                event[num_events] = temp;
                num_events++;
            }

// This nested while-loop is used to find the repeating events as well as count the number of events
// that occur during the 2 boundaries passed from the command line argument. "dt_increment" is used here to 
// find the next day. 2 variables that were declared above are used to store the date of the next event 
// which is then copied into DTSTART. Several if-loops are used to ensure everything is in order such as
// start date is not greater than end etc.

            while (strncmp (temp.DTSTART, temp.RRULE, 15) < 0) {

                dt_increment(new_DTSTART, temp.DTSTART, 7);
                strncpy(temp.DTSTART, new_DTSTART, strlen(temp.DTSTART));
                
                if (strncmp (temp.DTSTART, temp.RRULE, strlen(temp.DTSTART)) <= 0) {
                    
                     dt_increment (new_DTEND, temp.DTEND, 7);
                     strncpy (temp.DTEND, new_DTEND, strlen(temp.DTEND));
 
                     if (strncmp (new_DTSTART, new_DTEND, strlen(temp.DTEND)) < 0) {

                        sscanf(temp.DTSTART, "%8d", &start);
                        sscanf(temp.DTEND, "%8d", &end);
                        
                        if ( end >= start ) {

                            if ((from_date <= start) && (to_date >= end)) {
                                event[num_events] = temp;
                                num_events++;
                            }
                        }
                    }
                }
            }
        }
   }

    fclose(ics);
}

// This functions sorts events by quicksort using compare_dates whenever there is more than 1 event in a file.

void sort () {
    
    if (num_events > 1 ) {
        qsort (event, num_events, sizeof(calendar), compare_dates);
    }
    
}

// The other major function print, is used to print majority of the details with the exception of the date and dashes which are
// done by dt_print. 

void print () {

    for (int i = 0; i < num_events; i++) {

// These variables are used to split the DTSTART and DTEND fields into smaller parts so they can be printed in
// the correct format. 

        int start;
        int end;
        int hour_from;
        int hour_to;
        int minute_from;
        int minute_to;
        
        if ((i == 0 ) || ((i > 0) && (strncmp (event[i - 1].DTSTART, event[i].DTSTART, 8) != 0))) { 
             char formatted_time[MAX_LINE_LEN];

             dt_format(formatted_time, event[i].DTSTART, MAX_LINE_LEN);
        
             if (i > 0) {
                 printf ("\n");
             }
              
             dt_print(formatted_time);
        }

// Splitting the date up into the variables that were declared earlier.

        sscanf (event[i].DTSTART, "%8dT%2d%2d", &start, &hour_from, &minute_from);
        sscanf (event[i].DTEND, "%8dT%2d%2d", &end, &hour_to, &minute_to);

// This part of the function determines whether am or pm is to be used for the starting time and the
// end time. Several nested if-functions are used to determine am or pm as well minusing 12 hours to 
// convert from military time to the correct output.
        
        char *start_am_or_pm;
        char *end_am_or_pm;

        if ((hour_from < 12) && (hour_to < 12)) {
            start_am_or_pm = "AM";
            end_am_or_pm = "AM";
        }

        if ((hour_from < 12) && (hour_to >= 12)) {
            start_am_or_pm = "AM";
            end_am_or_pm = "PM";

            if (hour_to > 12) {
                hour_to = hour_to - 12;
            }
        }
 
        if ((hour_from >= 12) && (hour_to >= 12)) {
            start_am_or_pm = "PM";
            end_am_or_pm = "PM";

            if ((hour_from > 12) && (hour_to > 12)) {
                hour_from = hour_from - 12;
                hour_to = hour_to - 12;
            }

            if ((hour_to > 12) && (hour_from == 12)) {
                hour_to = hour_to - 12;
            }

        }
        
// The main print statement.

        printf ("%2d:%02d %s to %2d:%02d %s: %s {{%s}}\n", hour_from, minute_from, start_am_or_pm,
                           hour_to, minute_to, end_am_or_pm, event[i].SUMMARY, event[i].LOCATION);
        
   }

}


int main(int argc, char *argv[])
{
    int from_y = 0, from_m = 0, from_d = 0;
    int to_y = 0, to_m = 0, to_d = 0;
    char *filename = NULL;
    int i; 

    for (i = 0; i < argc; i++) {
        if (strncmp(argv[i], "--start=", 8) == 0) {
            sscanf(argv[i], "--start=%d/%d/%d", &from_y, &from_m, &from_d);
        } else if (strncmp(argv[i], "--end=", 6) == 0) {
            sscanf(argv[i], "--end=%d/%d/%d", &to_y, &to_m, &to_d);
        } else if (strncmp(argv[i], "--file=", 7) == 0) {
            filename = argv[i]+7;
        }
    }

    if (from_y == 0 || to_y == 0 || filename == NULL) {
        fprintf(stderr, 
            "usage: %s --start=yyyy/mm/dd --end=yyyy/mm/dd --file=icsfile\n",
            argv[0]);
        exit(1);
    }

    /* Starting calling your own code from this point. */
    
// Calling read by passing obtained arguments from command line followed by calling sort to quick sort the events based on date
// and finally calling print to produce the outputs.
    
    read (filename, from_y, from_m, from_d, to_y, to_m, to_d);
    sort ();
    print();
    exit(0);


