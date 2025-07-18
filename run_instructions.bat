@echo off
echo Hi there welcome to my program. Let's get started!
pause
gcc csv_insights.c -o csv_insights.exe
.\csv_insights.exe trial1_parsed.csv 550 8027 200 trial1_insights.txt
.\csv_insights.exe trial2_parsed.csv 322 7389 150 trial2_insights.txt
pause