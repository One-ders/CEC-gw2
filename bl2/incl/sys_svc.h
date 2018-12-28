
#define SVC_CREATE_TASK 1
#define SVC_MSLEEP      SVC_CREATE_TASK+1
#define SVC_SLEEP_ON    SVC_MSLEEP+1
#define SVC_WAKEUP      SVC_SLEEP_ON+1
#define SVC_IO_OPEN     SVC_WAKEUP+1
#define SVC_IO_READ     SVC_IO_OPEN+1
#define SVC_IO_WRITE    SVC_IO_READ+1
#define SVC_IO_CONTROL  SVC_IO_WRITE+1
#define SVC_IO_LSEEK    SVC_IO_CONTROL+1
#define SVC_IO_CLOSE    SVC_IO_LSEEK+1
#define SVC_IO_SELECT   SVC_IO_CLOSE+1
#define SVC_IO_MMAP     SVC_IO_SELECT+1
#define SVC_IO_MUNMAP   SVC_IO_MMAP+1
#define SVC_KILL_SELF   SVC_IO_MUNMAP+1
#define SVC_BLOCK_TASK  SVC_KILL_SELF+1
#define SVC_UNBLOCK_TASK SVC_BLOCK_TASK+1
#define SVC_KILL_PROC	SVC_UNBLOCK_TASK+1
#define SVC_SETPRIO_TASK SVC_KILL_PROC+1
#define SVC_SETDEBUG_LEVEL SVC_SETPRIO_TASK+1
#define SVC_REBOOT      SVC_SETDEBUG_LEVEL+1
#define SVC_GETTIC      SVC_REBOOT+1
#define SVC_SBRK        SVC_GETTIC+1
#define SVC_BRK         SVC_SBRK+1
#define SVC_FORK        SVC_BRK+1
#define SVC_EXIT	SVC_FORK+1

