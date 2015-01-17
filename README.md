# keystore
### Development of a key-value database (includes client). Created for the Subject of Operating Systems.
##### Authors: Marcos Bjørkelund (code and idea) and Adrián Marcelo Anillo (presentation)

### DESCRIPTION

We have used a server-client model in which the server is constantly running, so we can use dynamic memory allocation (e.g. via `malloc` or `calloc`).
The client does not write anything to memory, but reads the server's settings (this was done by demonstration, however we could have used macros (which we use in some forced cases for the client), which uses semaphores to read (the same as the server, to avoid any modification issue).

We have two different modes of usage:
- `set`: Creates the specified databases if non-existant, and stores the specified value inside them
- `get`: Searches the key in the specified databases, and returns the values found in each

### TECHNIQUES USED

- Linked lists: To store data dynamically and with the fewer limits possible.

- Signals: To stop the server when a `Ctrl-C` or `SIGINT` is sent to the process.

- Threads: for searching through multiple databases

- Shared memory: to store the server's settings

- Semaphores: for altering self-reserved memory for threading and shared memory (which stores the settings) operations

- Clocks: To measure the time a request took and the time the server has been running right before a shutdown.

- UNIX Sockets: To send a request-data between the server and the client. We are using bi-directional UNIX sockets (with `SOCK_STREAM`).
We could have used FIFO pipes but we opted by this, since it is something more modern and client-server specific.
We could also have used message queues instead, which can be seen as ideal but we want our client and server to be connected, and to know when one is not alive. Using sockets also allows us to send non-static data, e.g. strings with a dynamic length.

### INSTALLATION STEPS

First, clone this GIT repository in your local computer:
```
    $ cd ~
    $ git clone https://github.com/marcosbc/keystore.git
```

Next, compile! It is as easy as:
```
    $ make
```

If you want to install it into your system, just run the following command:
```
    $ sudo make install
```

If you decide to install it in your system, you can remove the repository.

### SERVER USAGE

To start your server, go into the folder in which the binaries are located and run:
```
    $ ./keystored
```

Your logs will be located at the `logs` directory inside `/var/log/keystore/`, where you will have `access.log` and `error.log`.

### CLIENT USAGE

First, get into the folder in which the binary files are installed.

To set a value in a database (no database creation step is needed), just run:
```
    $ ./keystore set KEY VALUE DB1[ DB2[ ...]]
```
E.g.:
```
    $ ./keystore set my_key my_value db1 db4 db5
```

To get a value from a database, run the following command:
```
    $ ./keystore get KEY DB1[ DB2[ ...]]
```
E.g:
```
    $ keystore get my_key db1 db2 db3 db4 db5
    db1: my_key=my_value
    db2: my_key=
    db3: my_key=
    db4: my_key=my_value
    db5: my_key=my_value
```

Please notice that even if the value doesn't exist, you will get a return value. However, no databases will get created.

To stop your server, just run:
```
    $ ./keystore stop
```

### TROUBLESHOOTING

If you killed your server with `SIGKILL` or the program encountered any error that did this, you might encounter problems with semaphores and/or shared memory that is already existing.

To check if you have anything left open:
```
    $ ipcs
```

To terminate a semaphore:
```
    $ ipcrm -s SEMAPHORE_ID
```

To terminate shared memory:
```
    $ ipcrm -m SHARED_MEMORY_ID
```
