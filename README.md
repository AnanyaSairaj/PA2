# PA2
## INTRODUCTION
GOAL :TO CREATE A HTTP BASED WEB SERVER THAT HANDLES MULTIPLE SIMULTANEOUS REQUESTS FROM THE CLIENT. 

This server is used to server multiple client requests parallely.The client requests have been implemented through the web browsers. This web server implementation is tested with Google Chrome and Firefox web browser. Pipeling has also been implemented to ensure the persistent connections as per the requirement.

### FILE STRUCTURE IMPLEMENTED
```
./README.txt
./server.c
./ws.conf
./www/

```

## IMPLEMENTATION

The server implements 2 HTTP Methods requested by the client ,the GET and the POST

* GET METHOD REQUESTED:In this method if the request contains a specific file to be returned the the server returns it .Otherwise it returns a default file .The server receives the request in the format of a http request and hence sends its response also in a http format followed by the file requested.
* POST METHOD REQUESTED: In this method the server requests for some data to be posted along with the request. The server parses the clients request and gets the data to be posted .It then sends the response with the http headers, the file and the data requested appended in the front of this in HTML format.

## ERROR CASES HANDLED

This server implementation handles the 500 Internal server errors .
* INVALID METHOD: This server accepts only GET ,POST and HEAD in the request. For any other method requests it displays BAD REQUEST:INVALID METHOD.
* INVALID HTTP VERSION: This server accepts only HTTP versions 1.1 and 1.0. If the client uses any pther version for the request it throws an error saying BAD REQUEST: INVALID VERSION implying the HTTP version.
* INVALID URL: The server checks the url path sent in the client request and with the acess function and if it does not return F_OK then it throws a BAD REQUEST:INVALID URL error to let the client know of the error in accessing the file from the specified path.


