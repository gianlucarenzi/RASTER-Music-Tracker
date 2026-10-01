#pragma once

#include <string>

// Script mode (rmt /SCRIPT:<file>, see ScriptRunner.h): the message boxes go
// to the console instead - errors and warnings to stderr, information to
// stdout, questions answered No/Cancel with a note - and the errors and
// warnings are collected so the script runner can fail the current command on
// them. With interactive set (Tools > Run Script) the boxes are shown as
// always, the problems are still collected.
extern void SetScriptMessageMode(bool enabled, bool interactive = false);
extern void ClearScriptProblems();
// The errors and warnings since ClearScriptProblems(), one line each; "" for none.
extern std::string GetScriptProblems();

// A message box of the program, with the Win32 style bits (MB_ICON*, MB_*)
// as MessageBox() has them. Returns true when script mode took it over; the
// button it "pressed" is then in answer (IDOK, IDNO or IDCANCEL).
extern bool ScriptMessageBox(const char* text, const char* caption, unsigned type, int& answer);
