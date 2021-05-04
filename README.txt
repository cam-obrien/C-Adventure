#C Adventure

This my personal take on the classic game "Colossal Cave Adventure" (1976). I replicated this game using C only.
The project is separated into two c programs. Firstly the "buildrooms.c" program creates a set of 7 "room files" containing the name of the room, a list of connecting rooms, and the type of the room. there are three types of rooms: the start room, middle rooms, and end room. The general point of the game is to navigate from the beginning room to the end room using the connecting rooms. During the game the user may type the command "time" to receive the current time of their system including the day, month, and year.

# To Run:

$ gcc -o obriecam.buildrooms obriecam.buildrooms.c
$ obriecam.buildrooms'

* This program might take a while to complete running *

Once the buildrooms program has finished running the following commands will begin the game.

$ gcc -o obriecam.adventure obriecam.adventure.c -lpthread
$ obriecam.adventure
