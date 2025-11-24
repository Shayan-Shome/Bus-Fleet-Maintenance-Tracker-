FleetGuardian – Intelligence Bus Fleet Maintenance Tracker



Author: Shayan Shome  

Language: C  

Project: PPS Project – Project 69  



Overview:

FleetGuardian is a console-based bus fleet maintenance tracker written in C.  

It stores bus details, predicts upcoming maintenance using mileage and optional service intervals, and highlights buses that are due soon or overdue.  



Features:

\- Dynamic fleet storage using `malloc` / `realloc` and an array of `struct Bus`.  

\- Full driver names with spaces and strict input validation for all fields.  

\- Reference date input for maintenance prediction (mileage + days).  

\- Status bands: \*\*OK\*\*, \*\*DUE SOON\*\*, \*\*OVERDUE\*\*, with a health score (0–100).  

\- Add, edit (by position), delete, search by bus number, and summary views.  

\- Persistent storage in `bus\_data.txt` using a `|`-separated text format.  

\- CSV maintenance report export to `fleet\_report.csv` for Excel / Sheets.  



Files:

\- `FleetGuardian\_Main-Shayan\_Shome.c` – complete C source code (single-file program).  

\- `bus\_data-Shayan\_Shome.txt` – sample fleet data (copied to `bus\_data.txt` at runtime).  

\- `start-Shayan\_Shome.bat` – Windows build \& run script using GCC.  



Build \& Run (Windows):

1\. Install MinGW-w64 or any GCC distribution and add `gcc` to PATH.  

2\. Put these files in one folder:  

&nbsp;  - `FleetGuardian\_Main-Shayan\_Shome.c`  

&nbsp;  - `bus\_data.txt` (rename from `bus\_data-Shayan\_Shome.txt`)  

&nbsp;  - `start-Shayan\_Shome.bat`  

3\. Double-click `start-Shayan\_Shome.bat` or run:

&nbsp;  > gcc FleetGuardian\_Main-Shayan\_Shome.c -o FleetGuardian

&nbsp;  > FleetGuardian.exe





On Linux/macOS, compile with:

&nbsp;  > gcc FleetGuardian\_Main-Shayan\_Shome.c -o FleetGuardian

&nbsp;  > ./FleetGuardian





Data Files



&nbsp; - Input: `bus\_data.txt`  

&nbsp; - First line: number of records.  

&nbsp; - Each following line:  

&nbsp;   `BusCode|DriverName|BusNo|LastDay|LastMonth|LastYear|NextDay|NextMonth|NextYear|CurrentKm|LastServiceKm|IntervalKm|IntervalDays|HistoryCount|StatusInt|KmLeft|HealthScore|AvgDailyKm|FuelEff`  



&nbsp; - Output CSV: `fleet\_report.csv`  

&nbsp; - Comma-separated with header:  

&nbsp;   `BusNo,BusCode,DriverName,LastServiceDate,NextDueDate,CurrentKm,KmLeft,HealthScore,Status,ServiceHistoryCount`  



How to Use:

1\. Program first asks for a reference date (dd/mm/yyyy).  

2\. It immediately shows which buses are \*\*overdue\*\* or \*\*due soon\*\*.  

3\. Use the menu to add/edit/delete/search buses and to view or export reports.  

4\. Choose option 10 (\*\*Save \& exit\*\*) to write all data back to `bus\_data.txt`.  



