# Grade - 100%

#!/usr/bin/env python3

import sys
import argparse
import datetime

# Name: Puneet Grewal
# Class: SENG 265 A01
# Prof: Mike Zastre
# Assignment: A2 - second try at iscout


# Unlike A1, icsout now uses 4 main functions as I have implemented read, store, sort and print.


# This function called  "read" takes in 1 parameter which is the file to be read. Using built in commands such as 
# strip, remove and split, the input file is processed into a nested list that contains all the events.

def read(ics_file):
   
    ics = open(ics_file, "r")
    lines = [i.strip() for i in ics]

# These 2 lines are used to remove the first and last lines of the file which are not of use.

    lines.remove(lines[0])
    lines.remove(lines[-1])

# Using split, the different "fields" such as summary, location etc. are separated by the "END:VEVENT" line.
# Irrelevant lines are removed using for, in and if statements. Data is "split" into new lines based on each "field".

    fields = ("\n".join(lines)).split("END:VEVENT")                     
    fields.remove("")                                                   
    fields = [field.split('\n') for field in fields]
    fields = [[line for line in field if (line != "")] for field in fields]
    fields = [[line for line in field if (line != "BEGIN:VEVENT")] for field in fields]                    
    fields = [[line for line in field if (line != "END:VEVENT")] for field in fields]
    fields = [[line for line in field if (line != "VERSION:A")] for field in fields]
    
    ics.close()

    return fields

    
# The second major function is used to store "all" the events in a dictionary.
# 1 parameter is passed which is the nested list from the previous function. 

def store (data):
  
    all_events = {}

# Using simple string splicing methods, all the "fields" are filled in

    for j in data:
        start_ical = j[0][8:]
        end_ical = j[1][6:]

# Over here I have used negative indexes as there is an anamlous "EXDATE" field in line 62 of
# diana-devops.ics. This allows me to bypass that field

        summary = j[-1][8:]
        location = j[-2][9:]

# Here, the location field is formatted to better suit the output

        location = "{{" + location + "}}"

# A major section of the "store" function, repeating events are accounted for in this code block.
# Due to the formatting differences of diana-devops.ics and many.ics, both are handled separately.

        if (j[2][0] == "R" and j[2][18] == "W"):
            rrule_ical = j[2][32:47]
        elif (j[2][0] == "R" and j[2][18] != "W"):
            rrule_ical = j[2][24:39]

# If a event does not repeat, the end date is set as the rrule date

        else:
            rrule_ical = j[1][6:]

        start = ical_to_normal(start_ical)
        end = ical_to_normal(end_ical)
        rrule = ical_to_normal(rrule_ical)

# A while loop similar to that used in icsout.c which adds events until the end date is equal to or 
# less than the rrule date

        while (start < end and rrule >= end):

            date = start.date()

# Checks if a date is absent in the dictionary and if true, initializes a new list for that event
    
            if (date not in all_events and start < end):
                all_events[date] = []

            all_events[date].append((start.time(), end.time(), summary, location))
           
            start = increment(start)
            end = increment(end)
     
    return all_events
    

# A function that sorts the dictionary using the lambda function based on the start date/time.
# Adapted from: https://docs.python.org/3/howto/sorting.html

def sort (events):
    
    for i in events:
        events[i].sort(key = lambda j: [j[0]])


# The last major function that prints all the events that fall within the command line arguments.
# It takes in 3 parameters which include the dictionary and the start and end dates.

def print_event (all_events, start, end):
    
# This code block is used to find all the "events" that occur within the 2 dates that are given
# These are then sorted using keys.
 
    keys = list(all_events.keys())
    keys.sort()
    keys = [event for event in keys if (start <= event and end >= event)]

# This line is written in case an empty file is inputted as I have not implemented error/exception
# handling. (Specifically for test case 4)
    
    if (keys == []):
        exit()

# The first line of the output is handled differently as a blank line is not to be entered
# before it. 

    event = keys[0]

# The "0 > 2" is used because the date/day can be a single or double digit value.

    date = "{0} {1:0>2}, {2} ({3})".format(month(event), event.day, event.year, name_of_day(event))

    printdate(date)

# The main print call that prints the second half of the event Format feature is used here
# as well to convert the time fields into the required formats. Summary and location do not need to be changed.
    
    for i in all_events[event]:
        print("{0} to {1}: {2} {3}".format(time(i[0]),time(i[1]), i[2], i[3]))

    for event in keys[1:]:
        print()
        date = "{0} {1:0>2}, {2} ({3})".format(month(event),event.day, event.year, name_of_day(event))
        printdate(date)
        

        for i in all_events[event]:
            print("{0} to {1}: {2} {3}".format(time(i[0]),time(i[1]), i[2], i[3]))


# Function that prints the date in the correct format and corresponding number of dashes.
    
def printdate (date):
    
    print(date)
    print("-" * len(date))


# Uses the timedelta function to increment parameter entered by 1 week or 7 days for repeating events.

def increment (date):

    increase = datetime.timedelta(days=7)
    return date + increase


# Slicing is used to convert the ical/ics date format into a datetime object that can be processed.

def ical_to_normal(date):
    
    year = int(date[0:4])
    month = int(date[4:6])
    day = int(date[6:8])
    hour = int(date[9:11])
    minute = int(date[11:13])
    second = int(date[13:])
   
    return datetime.datetime(year, month, day, hour, minute, second)


# This function is used to convert the command line/input date into a datetime object using splicing
# 2 different cases are covered, one where the month is 1 digit and the other where it is 2 (Oct - Dec)

def input_to_normal(input_date):

    if (input_date[6] == "/"):
        year = int(input_date[0:4])
        month = int(input_date[5]) 
        day = int(input_date[7:])

    if (input_date[7] == "/"):
        year = int(input_date[0:4])
        month = int(input_date[5:7])
        day = int(input_date[8:])
               
    return datetime.date(year, month, day)


# The following 3 functions use built in datetime features from the datetime module to convert parameters
# passed into the required formats.

def month(month_date):
   
    return month_date.strftime("%B")

def name_of_day(day_date):
    
    return day_date.strftime("%a")

def time(time_date):
    
    return time_date.strftime("%_I:%M %p")


# The code below configures the argparse module for use with
# assignment #2.
# 

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--file', type=str, help='file to be processed')
    parser.add_argument('--start', type=str, help='start of date range')
    parser.add_argument('--end', type=str, help='end of data range')

    args = parser.parse_args()

   # print ('file: {}; start: {}; end: {}'.format( args.file, args.start,
    #    args.end))

    if not args.file:
        print("Need --file=<ics filename>")

    if not args.start:
        print("Need --start=dd/mm/yyyy")

    if not args.end:
        print("Need --end=dd/mm/yyyy")


    events = read(args.file)
    all_events = store(events)
    sort(all_events)
    start = input_to_normal(args.start)
    end = input_to_normal(args.end)
    print_event(all_events, start, end)    


if __name__ == "__main__":
    main()
