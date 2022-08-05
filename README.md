# SRAT
SRAT is a simple remote acces tool capable of remote command execution aswell as file upload and download.

## Build Instructions
To build simply execute `make`. This will build both `out/local` and `out/remote`.

If you only wish to build one of these you can execute
```console
make local
```
or
```console
make remote
```
To clean `out` and `obj` execute
```console
make clean
```

## Run SRAT
To run SRAT start by executing
```console
./out/remote
```
this will start `remote` and by defult it will listen on port `8080` for new connections. The next step is to run `out/local` and give it the remote IP-address and port to connect to.
```console
./out/local [IP] [PORT]
```

Now everything sent will be executed as a command on the remote machine. Additional functionality is documented under [Commands](#Commands).


### Commands
| Command              | Description                           |
| -----------          | -----------                           |
| get [filename]       | Download a file from remote           |
| put [filename]       | Upload a file to remote               |


### TODO
- Error handling
- Refactor and clean
- Documentation