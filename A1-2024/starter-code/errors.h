#ifndef ERRORS_H
#define ERRORS_H

int badcommand();
int badcommandFileDoesNotExist();
int badcommandTooManyTokens();
int badcommandTooFewTokens();
int badcommandDirectoryAlreadyExist();
int exceptionCannotLoadInteractiveScript();
int badcommandInvalidPolicy();
int badcommandOutOfPIDs();
int badcommandThreadError();
int badcommandDuplicateProgramsInExec();

#endif
