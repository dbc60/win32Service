@echo off

REM Run emacs with a locally defined .emacs configuration file
%HOME%\Apps\emacs\bin\runemacs.exe --no-init-file --load="%~dp0.emacs"
