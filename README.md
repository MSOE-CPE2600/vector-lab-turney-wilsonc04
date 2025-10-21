## File Description
This program is an updated version of the vector calculator from originally developed in Week 5.
It now includes dynamic memory allocation and file input/output (I/O) features.
Users can perform vector operations, store an unlimited number of vectors in memory, and
save or load vector data using CSV files.

Vectors are stored dynamically in memory meaning the program no longer has a fixed storage limit.
When the user adds new vecots, memory is allocated as needed, and all allocated memory is released
upon program exit or when the clear command is used. The program has been tested with valgrind to verify 
that no memory leaks occur.

## How to Build
Use the provided makefile to compile the program:

  - make clean\
  - make

The program must compile with no warnings.

## How to Run
Run the executable from the termial:

  - ./vectorcalc

You can also test memory leaks using:

  - valgrind --leak-check=full ./vectorcalc

## Commands Supported
This program supports the following user commands: 

  - load <filename> - Loads vectors from a CSV file.\
  - save <filename> - Saves all vectors currently in memory to a CSV file.\
  - add <v1> <v2> - Adds two vectors and stores the result as a new vector.\
  - sub <v1> <v2> - Subtracts one vector from another.\
  - dot <v1> <v2> - Calculates the dot product.\
  - cross <v1> <v2> - Calculates the cross product.\
  - mag <v> - Calculates the magnitude of a vector.\
  - clear - Deletes all stored vectors and frees memory.\
  - list - Displays all currently stored vectors.\
  - exit - Exits the program cleanly, releasing all dynamic memory.\

## How this program uses dynamic memory
This program dynamically allocates memory to store vecotrs as they are created or loaded.
- Memory automatically expands as more vectors are added.
- All dynamically allocated memory is freed when the user clears the list or exits.
- Verified with Valgrind to ensure zero memory leaks. 
  






  
