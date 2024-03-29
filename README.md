<img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black" /> <img src="https://img.shields.io/badge/mac%20os-000000?style=for-the-badge&logo=apple&logoColor=white" />

# SRAT
SRAT is a very simple remote acces tool capable of remote command execution aswell as file upload and download.

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
./out/remote [PORT]
```
this will start `remote` and by defult it will listen on port `8080` for new connections. The next step is to run `out/local` and give it the remote IP-address and port to connect to.
```console
./out/local <IP> <PORT>
```

Now everything sent will be executed as a command on the remote machine. Additional functionality is documented under [Commands](#Commands).


### Commands
| Command              | Description                           |
| -----------          | -----------                           |
| get &lt;filename&gt; | Download a file from remote           |
| put &lt;filename&gt; | Upload a file to remote               |


### Limitations
- Directory of remote is not updated. To list file in subdrectory you need to run something like:
```console
cd ./dir_name && ls
```
- Until serialization is fixed you may experiance bugs.

### TODO
- Refactor!
- Serialize data packets
- More error handling
- Add encryption
