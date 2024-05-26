# MemoryOccupierKiller
A tool to kill memory occupier *(or other processes)* based on **taskkill** and **tasklist** for Windows NT

## More technical information in *Technical Documentation.md*(After it finished).

Config file format example(INI file):

[FiltersInformation]
NOF=1 <------------------------------------------------------Number Of Filters
FilterNo1=Blacklist

[Blacklist]
NOV=1 <------------------------------------------------------Number Of Variables
VarNo1=IMAGENAME eq winmine.exe <----------------------------As same as variables for tasklist.exe(In fact MOK just give this variable to tasklist as its arguments)
VarNo2=MEMUSAGE gt 524288 <----------------------------------Also
Expression=(1)(2)& <-----------------------------------------Postfix expression, T = true, F = false, (X) = variable No.X, If it's true, create threads and do the jobs below
NOT=2 <------------------------------------------------------Number Of Threads, Incomplete, The default job is just kill the processes who meet the requirements using taskkill with /f
ThrdNo1=msg;NVIDIA Web Helper.exe has been killed.; <--------Incomplete,
ThrdNo2=killimg;NVIDIA Web Helper.exe <----------------------Also
