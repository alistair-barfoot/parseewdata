@echo off
gcc csv_insights.c -o csv_insights.exe
.\csv_insights.exe Speed1-0-test.csv 1 668 1 Speed1-0-test_insights.txt
.\csv_insights.exe Speed1-5-test.csv 1 150 1 Speed1-5-test_insights.txt
.\csv_insights.exe Speed2-0-test.csv 1 296 1 Speed2-0-test_insights.txt
pause