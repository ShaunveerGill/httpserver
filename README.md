## Description
```
In this project, we were tasked with creating an HTTP server that can do three basic methods. These methods
include GET, PUT, and HEAD. The ultimate goal of this project was to build a modular system, in order to manage complexity.
```

# Build
```
In order to build this program the user will enter "make" into the terminal. this will build all necessary files, and 
clean any files created by the program.
```

# Structure
```
Due to the scale of this project modularity was essential to manage the complexity of the program. The program was split
into five functions.  First is the main, which includes the infinite loop the keeps the server up. Then, I had two functions
to validate the request. Lastly I split HEAD and GET into one function, and PUT into its own.  Since HTTP servers require a
strict format for a valid response, it was important to validate the request line before I proceeded with any commands. The
first validation function ensures that the spacing is accurate for the request, and that the header is in a valid format as 
outlined by the spec. the second, valiadtion function, is called to verify the URI, METHOD, and verison for appropriate 
character, and size. I placed GET and HEAD into the same function, as they perform very similar tasks, with the only difference 
being that GET writes the contents of the file. I simply have a toggle for HEAD, if the method is HEAD is will not write out the
content, while if its GET it will. This allowed me to condense my code, and reduce complexity since I only have to worry about
this singular function. PUT on the other is in its own function, since both the validation for the URI and error handling is
unique to PUT, it allows me to isolate it into its own function.
```

## Design Decisions
```
This project contains many complex parts, and therefore design decisions had to be made to ensure that we properly manage complexity.
Starting with parsing the request line, I had the choice of going byte by byte or reading in chunks. Originally, reading byte by byte would 
give me more control, as I could set delimiters and use them to assign chunks of the request. However, I found that it would be extremely
expensive as it would require using systems calls of at least 2048 for the request and header. In order to reduce this strain on the resources
I decided to read in block sizes of 4096 at a time. Once I filled the buffer, I iterate through the buffer to find invalid formatting and ensure
that the header fields are valid. Then I chunk the data by calling sscanf, this allows me to put the variables. Since the data is already validated
I don’t need to worry about breaking on the sscanf. Then verify the contents of the request line, and not just its format. Originally I had
placed the URI, and method validation within each call. For example, if I had PUT, it would validate the request line within the PUT_RESPONSE
functions, and the same for HEAD and GET. However, I realized that I could generalize the URI verification, for both functions before because
they had the same set of parameters. Another important design decision I made was using memset to reset the buffers to null. had I not implemented
memset, the data of the buffer would have carried over from the previous request. This lead to incorrect outputs, and improper parsing.
```
