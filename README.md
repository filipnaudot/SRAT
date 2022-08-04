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

### Commands
| command              | Description                           |
| -----------          | -----------                           |
| get [filename]       | Download a file from remote           |
| put [filename]       | Upload a file to remote               |


### TODO
- Error handling
- Refactor and clean
- Documentation