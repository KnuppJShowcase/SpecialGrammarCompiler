Jackson Knupp.20

I used the professors perfect project 4! I edited the memory.c, memory.h, and executor.c. 
In memory.c I created a method that is then called in procedure within executor.c to free all 
of the garbage collected things within the last frame. I also added a global variable called canReach which 
counts references. This variable is increments and decrements whenever a new record is allocated or needs to be 
deallocated. I also edited executor.c within the statements where whiles are executed to handle that scope. 

To test I ran the code and then would look at the test cases that failed. From there I would add print
statements or comment out code that I thought might be causing issues. Unfortunately I could not get test case 5,6 working 
completly. My gc: count was complelty correct but for some reason some of the changes I made were messing with how
the executor worked and displayed values. I tried very hard to fix this problem and I belive it has somehting to 
do with how I am saving values and indexing my arrays. I think I may be saving over the executors rValues on occasion or
accidently decrementing them. 