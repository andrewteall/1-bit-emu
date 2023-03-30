# __uLog__ - A C Micro Logging Framework
uLog enables simple application logging filtered by levels in a small package. Following the same format as printf() with formatted strings, uLog, allows you to add logs to your application that can be filtered via 6 Log Levels.

## Quick start
Start with the include:

```c
#include "ulog.h"
```
By default uLog only logs WARNING messages and above. Usually a program recieves a command line argument to set the logging level:

```c
if (!strcmp(argv[i],"-v") || !strcmp(argv[i],"--v") || !strcmp(argv[i],"--vv") \
			|| !strcmp(argv[i],"--vvv") || !strcmp(argv[i],"--vvvv")|| !strcmp(argv[i],"--vvvvv")){
			int logLevel = 0;
			if (!strcmp(argv[i],"-v")){
				logLevel = str2num(argv[i+1]);
			} else {
				logLevel = strlen(argv[i])-2;
			}
			if (logLevel >= 0 && logLevel <= 5){
				setLoggingLevel(logLevel);
			} else {
				ulog(ERROR,"Unsupported Log Level value");
				return 1;
			}
		}
```

or you can set your desired Logging Level via:

```
setLoggingLevel(loglevel);
```

From here you simply write:

```
ulog(DEBUG,"Debug message to be written. Val: %i",value);
```

Valid Log Levels are(These are enums and have correlating stings at the same index):

```OFF,FATAL,ERROR,WARNING,INFO,DEBUG```


If for any reason you need to determine what the current logging level is you can use:

```
getLoggingLevel();
```

getLoggingLevel() returns an int that correlates to the Log Levels enums.

## Documentation

Further documentation can be found in the ```ulog.h``` file.

## Building

To build the uLog lib to be used in your program:

```
make ulog
```

If you wish to use a version stripped of debug information:

```
make ulog-release
```