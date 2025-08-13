@echo off
gcc csv_insights.c -o csv_insights.exe
.\csv_insights.exe output_7-23.csv 10 970000 1000 7-23_insights.txt
pause